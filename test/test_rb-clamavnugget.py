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

'''cd
Created on 30-03-2012

@author: wojciechm
'''
import common4tests as c
import unittest
import re

originalName = "clamavNugget"
nuggetName = "rb-clamavnugget"
cwd = c.getTargetDir()
execPath = "/opt/hsn2/%s/%s" % (nuggetName, nuggetName)

class TestClamAvNugget(unittest.TestCase):
	'''
	This unit test requires clamav-daemon started with the supplied test database.
	An init.d file and the database are supplied with the integration test libraries.
	'''

	@classmethod
	def setUpClass(cls):
		# build the nugget - building not needed now assumming that we are using version installed from package.
		#c.runExternal(args=["/usr/bin/make", "%s" % originalName], cwd=cwd)
		pass

	def test_alarm(self):
		testArg1 = ""
		testArg2 = "--file=%s/test/resources/eicar" % cwd
		rexp = re.compile(r"ClamAV\sFound:\sEicar-Test-Signature")
		output = c.runExternal([execPath, testArg1, testArg2], cwd = cwd)
		self.assertRegexpMatches("".join(output), rexp, "Didn't report finding test signature.")

	def test_clean(self):
		testArg1 = ""
		testArg2 = "--file=%s/test/resources/make" % cwd
		rexp = re.compile(r"ClamAV\sFound:\sEicar-Test-Signature")
		output = c.runExternal([execPath, testArg1, testArg2], cwd = cwd)
		self.assertNotRegexpMatches("".join(output), rexp, "Reported alarm, but should have been clean.")

	# antivirus should be able to scan any input file
	#def test_diff(self):
	#	testResource = "test/resources/standard.sw"
	#	output = c.runWrapper(nuggetName, testResource)
	#	print output

	def test_error1(self):
		c.commonErrorNoFile(self, execPath)

	def test_error2(self):
		c.commonErrorDirInsteadOfFile(self, execPath)
