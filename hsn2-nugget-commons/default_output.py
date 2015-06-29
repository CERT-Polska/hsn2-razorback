#!/usr/bin/python -tt

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


import logging
import os
import re
import shutil

import hsn2_commons.hsn2objectwrapper as ow
from hsn2_commons.hsn2taskprocessor import ProcessingException, InputException


class UnexpectedOutputException(ProcessingException):
	pass

class DefaultOutputMapper():
	# These should be class variables if I understand correctly.
	reReturn = re.compile(r'Returned\swith:\s(\d+)')
	noticeIgnore = re.compile(r'inspecting',re.IGNORECASE)
	reTmpDir = re.compile(r"Temporary\sfiles\sstored\sto\s'(\S+)'")
	tmpDirs = None # init this
	hsn2TP = None # init this
	writtenMetaData = None
	verdictIsWarning = False
	lastReportedFilename = None
	defaultClassification = None
	classificationSet = None
	warningsSetClassification = False
	returnValue = None

	def __init__(self, hsn2TP):
		self.tmpDirs = list()
		self.writtenMetaData = dict()
		self.hsn2TP = hsn2TP
		self._defineOutputNameMapping()
		self._defineOutputProcessing()
		self._defineLevelProcessing()
		self.verdictIsWarning = False
		self._defineClassificationSet()

	def _defineClassificationSet(self):
		self.classificationSet = {
				"benign" : 0,
				"suspicious" : 1,
				"malicious" : 2
			}

	def _defineOutputNameMapping(self):
		prefix = "rb_"
		# Mapping is in the following notation: <BEFORE> : <AFTER>
		# An <AFTER> of length 0 (ex. "") means ignore.
		self.output_name_mapping = {
				"HTTP_REQUEST" : prefix + "http_request",
				"OSVDB" : prefix + "osvdb",
				"HTTP_RESPONSE" : prefix + "http_response",
				"DEST" : prefix + "dest",
				"SOURCE" : prefix + "source",
				"BID" : prefix + "bid",
				"URI" : prefix + "uri",
				"PATH" : prefix + "path",
				"REPORT" : prefix + "report",
				"CVE" : prefix + "cve",
				"FILENAME" : prefix + "filename",
				"HOSTNAME" : prefix + "hostname",
				"MALWARENAME" : prefix + "malwarename",
				"Message" : prefix + "verdict_message",
				"SourceFire flags" : prefix + "classification",
				"Priority" : prefix + "verdict_priority",
				"RETURN" : prefix + "return_value"
			}

	def _defineOutputProcessing(self):
		#This is responsible for dealing with types/files
		self.output_processing = {
				"Metadata_Add_String" : self._attributeString,
				"Metadata_Add_IPv4" : self._attributeString,
				"Metadata_Add_IPv6" : self._attributeString,
				"Metadata_Add_Port" : self._attributeInt,
				"Razorback_Render_Verdict" : self._attributeVerdict,
				"File_Store" : self._storeFile
			}

	def _defineLevelProcessing(self):
		self.level_processing = {
				"Debug" : None,
				"Info" : None,
				"Notice" : self._processNotice,
				"Warning" : self._processWarning,
				"Error" : self._processError,
				"Critical" : self._processError,
				"Alert" : self._processError,
				"Emergency" : self._processError
			}

	def _attributeVerdict(self,line):
		line = line.strip()
		if line[:7] == "Message":
			(head,tail) = line.split('-', 1)
			head = head.strip()
			tail = tail.strip()
			if tail[:7] == "Warning":
				self.verdictIsWarning = True
			else:
				self.verdictIsWarning = False
		if self.verdictIsWarning:
			if line[:16] == "SourceFire Flags":
				(head,tail) = line.split('-', 1)
				tail = tail.strip()
				if self.warningsSetClassification and self.classificationSet.get(self.defaultClassification,0) < self.classificationSet.get(tail,0):
					self.defaultClassification = tail.strip()
			self.warnings.append("Verdict warning - %s" % line)
		else:
			if line[:8] == "Priority":
				self._attributeInt(line)
			else: self._attributeString(line)

	def _attributeCheck(self,line):
		line = line.strip()
		(head,tail) = line.split('-', 1)
		head = head.strip()
		tail = tail.strip()
		name = self.output_name_mapping.get(head)
		if head == "FILENAME":
			self.lastReportedFilename = tail
		if name is None:
			raise UnexpectedOutputException(line)
		if len(name) > 0:
			if self.writtenMetaData.get(name) is not None:
				val = getattr(self.hsn2TP.objects[0], name)
				self.warnings.append("Attribute '%s' has multiple values. Overwritten. Previous value was %s." % (name, str(val)))
			self.writtenMetaData[name] = tail
		return (name,tail)

	def _attributeString(self, line):
		(name,tail) = self._attributeCheck(line)
		if len(name) > 0:
			self.hsn2TP.objects[0].addString(name,tail)

	def _attributeInt(self, line):
		(name,tail) = self._attributeCheck(line)
		if len(name) > 0:
			self.hsn2TP.objects[0].addInt(name,tail)

	def _storeFile(self, tail):
		tail = tail.strip()
		(pDir,pFile) = os.path.split(tail)
		(pHash,pSize) = os.path.splitext(pFile)
		dsId = self.hsn2TP.dsAdapter.putFile(tail,self.hsn2TP.currentTask.job)
		os.remove(tail)
		fileObj = ow.Object()
		if len(self.lastReportedFilename) > 0:
			fileObj.addString("filename",self.lastReportedFilename)
			self.lastReportedFilename = ""
		fileObj.addString("type","undefined")
		fileObj.addString("sha256",pHash)
		fileObj.addInt("size",pSize[1:])
		fileObj.addBytes("content", dsId, 0)
		fileObj.addObject("parent", self.hsn2TP.objects[0].getObjectId())
		newObjIds = self.hsn2TP.osAdapter.objectsPut(self.hsn2TP.currentTask.job, self.hsn2TP.currentTask.task_id, [fileObj])
		self.hsn2TP.newObjects.extend(newObjIds)

	def _processNotice(self,line):
		line = line.strip()
		found = False
		try:
			(source,tail) = line.split("-",1)
			source = source.strip()
			fun = self.output_processing.get(source)
			if fun is None:
				found = True
			else:
				fun(tail)
				found = True
		except ValueError:
			if self.noticeIgnore.search(line) is not None:
				found = True
			tmpDir = self.reTmpDir.match(line)
			if tmpDir is not None:
				self.tmpDirs.append(tmpDir.group(1))
				found = True
			ret = self.reReturn.match(line)
			if ret is not None:
				ret = ret.group(1)
				if int(ret) != 0:
					self.warnings.append(line)
				self.returnValue = int(ret)
				ret = "RETURN - %s" % ret
				self._attributeInt(ret)
				found = True
		except UnexpectedOutputException as e:
			self.warnings.append(e.message)
			found = True # otherwise we would add this twice.
		if found is False:
			self.warnings.append(line)

	def _processInfo(self,line):
		print line

	def _processWarning(self,line):
		self.warnings.append(line)

	def _processError(self,line):
		line = line.strip()
		if line[:5] == "INPUT":
			raise InputException(line)
		elif line[:6] == "DEFUNCT":
			raise ProcessingException(line)
		else:
			self.warnings.append(line)
		#raise ProcessingException(line)

	def nonLoggerOutput(self,line):
		'''
		Function for handling output that didn't have a log level.
		Output that reaches this probably wasn't printed by rzb_log.
		'''
		return

	def setDefaultClassification(self):
		'''
		Checks whether the object already has a classification set and if not then it sets it to benign.
		This should be called only after processing all the output from the nugget.
		'''
		name_mapping = self.output_name_mapping.get("SourceFire flags")
		try:
			if self.defaultClassification is None: return
			if self.returnValue != 0: return
			if name_mapping is None or len(name_mapping) == 0: return
			if not self.hsn2TP.objects[0].isSet(name_mapping):
				self.hsn2TP.objects[0].addString(name_mapping,self.defaultClassification)
		except UnexpectedOutputException:
			pass

	def process(self, text):
		'''
		This function should define all the processing that is to be done - mapping/filtering etc.
		'''
		self.returnValue = None
		self.defaultClassification = "benign"
		self.verdictIsWarning = False
		self.tmpDirs = list()
		self.writtenMetaData = dict()
		self.warnings = list()
		self.lastReportedFilename = ""
		l = len(text)
		while (l > 0):
			try:
				line = text.pop(0)
				l = l - 1
				(head, tail) = line.split(":",1)
				fun = self.level_processing.get(head)
				if fun is not None:
					fun(tail.strip())
				else:
					if head not in self.level_processing:
						raise ValueError()
			except ValueError as e:
				self.nonLoggerOutput(line)
		for tmpDir in self.tmpDirs:
			try:
				shutil.rmtree(tmpDir,ignore_errors=True)
				#os.removedirs(tmpDir)
			except OSError as e:
				logging.warn(e)
				self.warnings.append(str(e))
		warnings = self.warnings[:]
		self.setDefaultClassification()
		self.warnings = list()
		# Object and text will be modified in place. Return used for unexpected output
		return warnings

class OutputMapper(DefaultOutputMapper):
	pass

if __name__ == '__main__':
	obj = ow.Object()
	obj.objects = [ow.Object()]
	im = OutputMapper(obj)
	try:
		print im.output_name_mapping
		print im.process(["Notice: Razorback_Render_Verdict - Message - Bla", "Notice: Metadata_Add_String - FILENAME - Bla"])
	except Exception as e:
		print e
	from clamavnugget_output import OutputMapper as clamOM
	obj = ow.Object()
	obj.objects = [ow.Object()]
	im = clamOM(obj)
	try:
		print "ClamAv"
		print im.output_name_mapping
		print im.process(["Notice: Razorback_Render_Verdict - Message - Bla", "Notice: Metadata_Add_String - FILENAME - Bla"])
		print "classification:", im.defaultClassification
	except Exception as e:
		print e
	from archiveinflate_output import OutputMapper as aiOM
	obj = ow.Object()
	obj.objects = [ow.Object()]
	im = aiOM(obj)
	try:
		print "ArchiveInflate"
		print im.output_name_mapping
		print im.process(["Notice: Razorback_Render_Verdict - Message - Warning - Bla", "Notice: Razorback_Render_Verdict - SourceFire Flags - suspicious", "Notice: Razorback_Render_Verdict - SourceFire Flags - malicious", "Notice: Razorback_Render_Verdict - SourceFire Flags - suspicious", "Notice: Metadata_Add_String - FILENAME - Bla"])
	except Exception as e:
		print e
	finally:
		print "classification:", im.defaultClassification
		print "was warning:", im.verdictIsWarning
