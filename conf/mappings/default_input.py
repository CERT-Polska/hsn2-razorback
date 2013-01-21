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

'''
Created on 11-04-2012

@author: wojciechm
'''
import sys
sys.path.append("/opt/hsn2/python/commlib")
from hsn2taskprocessor import ParamException

class DefaultInputMapper():


	def __init__(self):
		self._defineInputTypeMapping()

	def _defineInputTypeMapping(self):
		#provided as an example only. Needs to be filled in with hsn2Types and their corresponding razorback types from the wiki
		self.map = {
				"swf":"FLASH_FILE",
				"pdf":"PDF_FILE",
				"tar":"TAR_FILE",
				"gz":"GZIP_FILE"
			}

	def translateType(self,hsn2Type):
		return self.map.get(hsn2Type)

#	def getTypeAsArgument(self,hsn2Type):
#		internalType = self.translateType(hsn2Type)
#		if internalType is not None:
#			internalType = "--type=%s" % internalType
#		else: raise ParamException("No mapping defined for '%s'" % hsn2Type)
#		return internalType

class InputMapper(DefaultInputMapper):
	pass

if __name__ == '__main__':
	im = InputMapper()
	try:
		print im.translateType("TAR")
	#	print im.getTypeAsArgument("TAR")
	except ParamException as e:
		print e
