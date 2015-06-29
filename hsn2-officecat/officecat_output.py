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

from default_output import DefaultOutputMapper


class OutputMapper(DefaultOutputMapper):
	'''
	Output mapper class configured for officeCat
	'''

	def _defineOutputNameMapping(self):
		prefix = "rb_officecat_"
		# Mapping is in the following notation: <BEFORE> : <AFTER>
		# An <AFTER> of length 0 (ex. "") means ignore.
		self.output_name_mapping = {
			"CVE" : prefix + "cve",
			"Message" : prefix + "verdict_message",
			"SourceFire flags" : prefix + "classification",
			"Priority" : prefix + "verdict_priority",
			"RETURN" : prefix + "return_value"
		}
