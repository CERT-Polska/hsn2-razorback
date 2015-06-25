# Copyright (c) NASK, NCSC
# 
# This file is part of HoneySpider Network 2.0.
# 
# This is a free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


import sys
sys.path.append("/opt/hsn2/python/commlib")
from hsn2taskprocessor import HSN2TaskProcessor
from hsn2taskprocessor import ParamException, ProcessingException
from hsn2osadapter import ObjectStoreException
import logging
import subprocess


class NuggetTaskProcessor(HSN2TaskProcessor):
	nugget = None
	inputMapping = None
	inputMapper = None
	outputMapping = None
	outputMapper = None
	proc = None

	'''
	Task processor for Razorback nuggets.
	What should be done in processing:
	1) launch nugget with required arguments
	2) read output - determine whether successful or failed
	3a) If failed throw TaskFailedException
	3b) If successful return tuple (task, warnings)
	'''
	def __init__(self,connector,datastore,serviceName,serviceQueue,objectStoreQueue,**extra):
		'''
		Runs Process init first and then creates required connections.
		To do: Implement data store connection.
		To do: Implement object store interaction.
		'''
		HSN2TaskProcessor.__init__(self,connector,datastore,serviceName,serviceQueue,objectStoreQueue,**extra)
		self.nugget = extra.get("nugget")
		self.inputMapping = extra.get("inputmapping")
		self.outputMapping = extra.get("outputmapping")
		self.outputMapper = self.outputMapping.OutputMapper(self)
		self.inputMapper = self.inputMapping.InputMapper()

	def taskProcess(self):
		'''	This method should be overridden with what is to be performed.
			Returns a list of warnings (warnings). The current task is available at self.currentTask'''
		logging.debug(self.__class__)
		logging.debug(self.currentTask)
		logging.debug(self.objects)
		if len(self.objects) == 0:
			raise ObjectStoreException("Task processing didn't find task object.")

		if self.objects[0].isSet("content"):
			filepath = self.dsAdapter.saveTmp(self.currentTask.job, self.objects[0].content.getKey())
		else:
			#raise ParamException("Content attribute is not set.")
			filepath = ""

		if self.objects[0].isSet("type"):
			ftype = self.inputMapper.translateType(self.objects[0].type)
		else:
			ftype = "none"

		if self.objects[0].isSet("uuid"):
			uuid = self.objects[0].uuid
		else:
			uuid = "none"

		params = [self.nugget,filepath,uuid,ftype]
		try:
			value = self.runExternal(params)
			# Output mapper is responsible for adding attributes to the object.
			warnings = self.outputMapper.process(value)
			self.dsAdapter.removeTmp(filepath)
		except Exception as e:
			self.dsAdapter.removeTmp(filepath)
			raise e
		return warnings

	def prepareExternal(self, i, args):
		'''this method detects if nugget process is running and if not it starts it'''
		if i >= 5:
			return False

		if self.proc == None:
			self.proc = subprocess.Popen([args[0], "-d"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

		if self.proc.poll() != None:
			self.proc = None
			return self.prepareExternal(i + 1, args)
		return True

	def runExternal(self,args):
		try:
			if not self.prepareExternal(1, args):
				raise ParamException("runExternal: Couldn't run external nugget binary")

			try:
				self.proc.stdin.write("--uuid=%s --type=%s --file=%s\n" % (args[2], args[3], args[1]))
			except IOError:
				self.proc = None
				self.runExternal(args)

			line = ""
			output = []
			while line != "Info: FINISHED\n":
				if self.proc.poll() != None:
					raise ProcessingException("Nugget process failed")
				line = self.proc.stdout.readline()
				output.append(line)
			return output
		except OSError as (e):
			raise ParamException(e.message)
		except ValueError as (e):
			raise ParamException(e.message)

	def cleanup(self):
		if self.proc is not None:
			self.proc.kill()
			self.proc = None
