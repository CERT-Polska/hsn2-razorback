#!/usr/bin/python -tt

# Copyright (c) NASK, NCSC
#
# This file is part of HoneySpider Network 2.1.
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

from subprocess import Popen, PIPE, STDOUT
import datetime
import os
import random
import signal
import sys
import time

from hsn2objectwrapper import Object
from hsn2osconnector import HSN2ObjectStoreConnector, QueryStructure
from hsn2osconnector import ObjectStoreException
from hsn2rmq import RabbitMqBus
import argparsealiases as argparse
import loggingSetup


class MyObject():
    obj = None

    def __init__(self, c, s):
        self.obj = Object()
        self.obj.addString("color", c)
        self.obj.addInt("size", s)

    def getObject(self):
        return self.obj


class objectStorePerfTest():
    analyzerProc = None
    reporterProc = None
    frameworkProc = None
    description = 'Object store performance test'
    args = None
    osConnector = None
    fwBud = None
    connector = "127.0.0.1"
    port = 5672
    jobId = 63
    taskId = 123
    objectList = None
    roundsCnt = 100
    insertCnt = updateCnt = getCnt = queryCnt = 0

    def __init__(self):
        #loggingSetup.setupLogging(None, True)
        self.objectList = []
        self.parseOptions()
        if self.args.testType == 'main':
            return self.createProc()

        self.fwBus = RabbitMqBus(host=self.connector, port=self.port, resp_queue="nugget1", app_id="nugget1")
        self.fwBus.openFwChannel()
        self.osConnector = HSN2ObjectStoreConnector(self.fwBus)

        if self.args.testType == "analyzer":
            return self.measureTime(self.analyzerTask, "Analyzer time")

        if self.args.testType == "reporter":
            self.prepareData()
            return self.measureTime(self.reporterTask, "Reporter time")

        if self.args.testType == "framework":
            self.prepareData()
            return self.measureTime(self.frameworkTask, "Framework time")

    def measureTime(self, arg, info):
        sys.stdin.read(1)
        start = time.time()
        arg()
        end = time.time()
        print "INFO: %s - %.6f, Executed %d insertions, %d updates, %d get requests, %d queries" % (info, end - start, self.insertCnt, self.updateCnt, self.getCnt, self.queryCnt)

    def parseOptions(self):
        parser = argparse.ArgumentParser(description=self.description, formatter_class=argparse.ArgumentDefaultsHelpFormatter)
        parser.add_argument('--test', '-t', action='store', choices='analyzer reporter framework main'.split(), help='test type', required=True, dest='testType')
        self.args = parser.parse_args()

    def prepareData(self):
        print "Preparing data..."
        for i in range(0, 10):
            self.objectList.append(self.osConnector.objectsPut(self.jobId, self.taskId, [MyObject("green%d" % i, i).getObject()])[0])
        print "Data prepared."

    def analyzerTask(self):
        random.seed()
        k = 1
        for i in range(0, self.roundsCnt):
            self.objectList.append(self.osConnector.objectsPut(self.jobId, self.taskId, [MyObject("green%d" % i, i).getObject()])[0])
            self.objectList.append(self.osConnector.objectsPut(self.jobId, self.taskId, [MyObject("red%d" % i, i).getObject()])[0])
            self.objectList.append(self.osConnector.objectsPut(self.jobId, self.taskId, [MyObject("blue%d" % i, i).getObject()])[0])
            self.insertCnt += 3
            obj = self.osConnector.objectsGet(self.jobId, [self.objectList[i]])[0]
            self.getCnt += 1
            obj.addString("time%d" % k, str(time.time()))
            k = k + 1
            self.osConnector.objectsUpdate(self.jobId, [obj])
            obj.addString("time%d" % k, str(time.time()))
            k = k + 1
            self.osConnector.objectsUpdate(self.jobId, [obj])
            obj.addString("time%d" % k, str(time.time()))
            k = k + 1
            self.osConnector.objectsUpdate(self.jobId, [obj])
            obj.addString("time%d" % k, str(time.time()))
            k = k + 1
            self.osConnector.objectsUpdate(self.jobId, [obj])
            self.updateCnt += 4

    def reporterTask(self):
        random.seed()
        for i in range(0, self.roundsCnt):
            obj = self.osConnector.objectsGet(self.jobId, [self.objectList[random.randint(0, 9)]])[0]
            self.getCnt += 1
            queryCnt = random.randint(0, 2)
            if queryCnt > 0:
                queryObj = Object()
                queryObj.addString("color", "green1")
                qS = QueryStructure(queryObj, False)
                for x in range(0, queryCnt):
                    qS.setNegate(x == 1)
                    self.osConnector.query(self.jobId, [qS])
            self.queryCnt += queryCnt
            self.osConnector.objectsGet(self.jobId, self.objectList[:5])
            self.getCnt += 5

    def frameworkTask(self):
        random.seed()
        k = 1
        for i in range(0, self.roundsCnt):
            obj = self.osConnector.objectsGet(self.jobId, [self.objectList[random.randint(0, 9)]])[0]
            self.getCnt += 1
            obj.addString("time%d" % k, str(time.time()))
            k = k + 1
            self.osConnector.objectsUpdate(self.jobId, [obj])
            obj.addString("time%d" % k, str(time.time()))
            k = k + 1
            self.osConnector.objectsUpdate(self.jobId, [obj])
            self.updateCnt + 2
            queryObj = Object()
            queryObj.addString("color", "green1")
            qS = QueryStructure(queryObj, False)
            self.osConnector.query(self.jobId, [qS])
            qS.setNegate(True)
            self.osConnector.query(self.jobId, [qS])
            self.queryCnt += 2

    def createProc(self):
        self.analyzerProc = Popen(["./objectStorePerfTest.py", "-t", "analyzer"], stdin=PIPE)
        self.reporterProc = Popen(["./objectStorePerfTest.py", "-t", "reporter"], stdin=PIPE)
        self.frameworkProc = Popen(["./objectStorePerfTest.py", "-t", "framework"], stdin=PIPE)

        time.sleep(8)

        self.analyzerProc.stdin.write("a")
        self.reporterProc.stdin.write("a")
        self.frameworkProc.stdin.write("a")

        self.analyzerProc.wait()
        self.reporterProc.wait()
        self.frameworkProc.wait()


if __name__ == '__main__':
    objectStorePerfTest()
