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

import os
import subprocess
import re

razorbackDir = os.chdir(os.getcwd() + "/..")
razorbackDir = os.getcwd()


def changeDir():
    global razorbackDir
    os.chdir(razorbackDir)


def getTargetDir():
    global razorbackDir
    return razorbackDir


class ReturnException(Exception):
    pass


def runWrapper(nugget, resource, bdir=razorbackDir, rdir=razorbackDir):
    '''
    Runs the nugget with the passed
    '''
    testResource = os.path.join(rdir, resource)
    binary = os.path.join(bdir, nugget, nugget)
    return runExternal(args=[binary, testResource], cwd=rdir)


def runExternal(args, cwd):
    try:
        proc = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, cwd=cwd)
        proc.wait()
        output = proc.stdout.readlines()
        if proc.returncode != 0:
            raise ReturnException("Execution returned %d" % proc.returncode)
        else:
            print '\n'.join(output)
            return output
    except (OSError, ValueError) as e:
        print ' '.join(args)
        print e
        raise


def commonErrorNoFile(obj, execPath, cwd=razorbackDir):
    testArg1 = ""
    testArg2 = os.path.join(cwd, "resources")
    rexp = re.compile(r"Trouble\spreparing\sfile\stransfer|Failed\sto\sopen")
    output = runExternal([execPath, testArg1, testArg2], cwd=cwd)
    obj.assertRegexpMatches("".join(output), rexp, "Didn't report error opening/preparing file.")


def commonErrorDirInsteadOfFile(obj, execPath, cwd=razorbackDir):
    testArg1 = ""
    testArg2 = os.path.join(cwd, "resources", "nonexistetnt")
    rexp = re.compile(r"Trouble\spreparing\sfile\stransfer|Failed\sto\sopen")
    output = runExternal([execPath, testArg1, testArg2], cwd=cwd)
    obj.assertRegexpMatches("".join(output), rexp, "Didn't report error opening/preparing file.")
