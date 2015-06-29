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
from os import X_OK
from os import access
from os import path

from hsn2_commons.hsn2service import HSN2Service
from hsn2_commons.hsn2service import startService
from hsn2_nugget_commons.nuggetTaskProcessor import NuggetTaskProcessor


class MappingException(Exception):
	pass


class NuggetService(HSN2Service):
	serviceName = "nugget1"
	mappings = "/etc/hsn2/razorback/mappings/" # hard coding this as it's meant to be the default location.
	description = "HSN 2 Razorback Nugget Service"
	inputmapping = None
	outputmapping = None
	'''
	This is the HSN2 service for executing wrapped Razorback nuggets.
	'''

	def extraOptions(self,parser):
		'''Arguments specific to this service. Receives a parser with the standard options. Returns a modified parser.'''
		parser.add_argument('--nugget', '-n', action='store', help='nugget executable path', required=True, dest='nugget')
		parser.add_argument('--input', '-I', action='store', help='configuration to use for input mapping', required=False,
						dest='inputmapping', default="default_input.py")
		parser.add_argument('--output', '-O', action='store', help='configuration to use for output mapping', required=False,
						dest='outputmapping', default="default_output.py")
		return parser

	def sanityChecks(self, cliargs):
		passed = HSN2Service.sanityChecks(self, cliargs)
		if path.isdir(cliargs.nugget):
			logging.error("'%s' is a directory" % cliargs.nugget)
			passed = False
		if not access(cliargs.nugget, X_OK):
			logging.error("'%s' isn't executable or does not exist!" % cliargs.nugget)
			passed = False
		if not path.isabs(cliargs.inputmapping):
			cliargs.inputmapping = self.mappings + cliargs.inputmapping
		if path.isdir(cliargs.inputmapping):
			logging.error("'%s' is a directory" % cliargs.inputmapping)
			passed = False
		elif not path.isfile(cliargs.inputmapping):
			logging.error("'%s' does not exist!" % cliargs.inputmapping)
			passed = False
		if not path.isabs(cliargs.outputmapping):
			cliargs.outputmapping = self.mappings + cliargs.outputmapping
		if path.isdir(cliargs.outputmapping):
			logging.error("'%s' is a directory" % cliargs.outputmapping)
			passed = False
		elif not path.isfile(cliargs.outputmapping):
			logging.error("'%s' does not exist!" % cliargs.outputmapping)
			passed = False
		if passed is True:
			cliargs.inputmapping = self.importMapping(cliargs.inputmapping)
			cliargs.outputmapping = self.importMapping(cliargs.outputmapping)
			if cliargs.inputmapping is None or cliargs.outputmapping is None:
				passed = False
		return passed

if __name__ == '__main__':
	startService(NuggetService,NuggetTaskProcessor)
