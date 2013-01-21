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

originalName = "archiveInflate"
nuggetName = "rb-archiveinflate"
cwd = c.getTargetDir()
execPath = "/opt/hsn2/%s/%s" % (nuggetName, nuggetName)

class TestArchiveInflate(unittest.TestCase):
	@classmethod
	def setUpClass(cls):
		# build the nugget - building not needed now assumming that we are using version installed from package.
		#c.runExternal(args=["/usr/bin/make", "%s" % originalName], cwd=cwd)
		pass

	def test_supported(self):
		'''
		Test for properly specified UUID and file.
		'''
		testArg1 = "--type=GZIP_FILE"
		testArg2 = "--file=%s/test/resources/testtar.tar.gz" % cwd
		rexp = re.compile(r"Extracting\sPath")
		rexp2 = re.compile(r"File_Store")
		output = c.runExternal([execPath, testArg1, testArg2], cwd=cwd)
		self.assertRegexpMatches("".join(output), rexp, "Didn't report extracting path.")
		self.assertRegexpMatches("".join(output), rexp2, "Didn't report calling File_Store.")

	def test_notsupported(self):
		'''
		Test for when the passed file isn't of the type specified by the passed uuid.
		'''
		testArg1 = "--type=AR_FILE"
		testArg2 = "--file=%s/test/resources/testtar.tar.gz" % cwd
		rexp = re.compile(r"Unrecognized\sarchive\sformat")
		output = c.runExternal([execPath, testArg1, testArg2], cwd=cwd)
		self.assertRegexpMatches("".join(output), rexp, "Warning should have been displayed about unrecognised archive.")

	def test_nouuid(self):
		'''
		Test for when no uuid was passed or the uuid doesn't correspond to a archive format supported by the nugget.
		'''
		testArg1 = "--type=JAVASCRIPT"
		testArg2 = "--file=%s/test/resources/testtar.tar.gz" % cwd
		rexp = re.compile(r"No\sformats\sregistered")
		output = c.runExternal([execPath, testArg1, testArg2], cwd=cwd)
		self.assertRegexpMatches("".join(output), rexp, "Didn't report no formats registered, but should have.")
		output = c.runExternal([execPath, testArg2], cwd=cwd)
		self.assertRegexpMatches("".join(output), rexp, "Didn't report no formats registered, but should have.")

	def test_error1(self):
		c.commonErrorNoFile(self, execPath)

	def test_error2(self):
		c.commonErrorDirInsteadOfFile(self, execPath)
