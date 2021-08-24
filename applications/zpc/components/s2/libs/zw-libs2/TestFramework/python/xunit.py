# Copyright (c) 2018 Silicon Laboratories Inc.
import os
import xml.etree.ElementTree as ElementTree
import re

class Xunit():
  def __init__(self, testsuite=None):
    self.testsuites = ElementTree.Element("testsuites")
    self.testsuites.text = '\n'
    self.testsuites.tail = '\n'

    if testsuite is not None:
      self.testsuites.append(testsuite)

  def append(self, testsuite):
    testsuitetree = testsuite.getTree()
    self.testsuites.append(testsuitetree)

  def getTree(self):
    return self.testsuites

  def tostring(self):
    xmlout = '<?xml version="1.0" encoding="UTF-8"?>\n'
    xmlout += ElementTree.tostring(self.testsuites)
    return xmlout

class TestSuite():
  def __init__(self, name="", hostname=None):
    self.testsuite = ElementTree.Element("testsuite")
    self.testsuite.set('name', name)
    self.testsuite.text = '\n'
    self.testsuite.tail = '\n'
    if hostname is not None:
      self.testsuite.set('hostname', hostname)

    self.testsuite.set('failures', "0")

    self.tests     = 0
    self.passed    = 0
    self.failures  = 0
    self.skipped   = 0
    self.time      = 0.0

  def getTree(self):
    return self.testsuite

  def append(self, testcase):
    sub = ElementTree.SubElement(self.testsuite, "testcase")
    sub.set('name', testcase.getName())
    sub.set('time', str(testcase.getTime()))
    sub.text = '\n'
    sub.tail = '\n'
    self.tests += 1
    self.time  += testcase.getTime()
    if testcase.failed():
      self.failures += 1
      sub = ElementTree.SubElement(sub, "error")
      sub.set('message', testcase.getMessage())
      sub.text = testcase.getVerboseMessage()
      sub.tail = '\n'

    if testcase.skipped():
      self.skipped += 1
      sub = ElementTree.SubElement(sub, "skipped")
      sub.text = '\n'
      sub.tail = '\n'
    self.testsuite.set('tests',   str(self.tests))
    self.testsuite.set('errors',  str(self.failures))
    self.testsuite.set('skipped', str(self.skipped))
    self.testsuite.set('time',    str(self.time))

  def setTime(self, time):
    self.time = time
    self.testsuite.set('time',    str(self.time))

  def fromUnity(self, report, name="", time=None, hostname=None):
    self.testsuite.set('name', name)
    self.testsuite.text = '\n'
    self.testsuite.tail = '\n'

    for line in report:
      passed = re.match(r":[0-9]*:(.*):PASS.*", line)
      failed = re.match(r":[0-9]*:(.*):FAIL:(.*)", line)
      ignore = re.match(r":[0-9]*:(.*):IGNORE:(.*)", line)
      if passed:
        testcase = TestCase(passed.group(1))
        self.append(testcase)
      elif failed:
        testcase = TestCase(failed.group(1), error=True)
        testcase.setMessage(failed.group(2))
        self.append(testcase)
      elif ignore:
        testcase = TestCase(ignore.group(1), ignore=True)
        testcase.setMessage(ignore.group(2))
        self.append(testcase)

    self.testsuite.set('tests',   str(self.tests))
    self.testsuite.set('errors',  str(self.failures))
    self.testsuite.set('skipped', str(self.skipped))
    self.testsuite.set('time',    str(self.time))
    if time is not None:
      self.testsuite.set('time', str(time))
    if hostname is not None:
      self.testsuite.set('hostname', hostname)
    return self

  def fromCtest(self, report, hostname=None):
    testsuites = []

    current = '1'
    next    = '2'
    testsuiterep = []
    time = 0.0
    for line in report:
      found = re.match(r"[ \t]*[0-9]*/[0-9]* Test[ \t]*#[0-9]*:.*([0-9]*\.[0-9]*)[ \t]*sec.*", line)
      if found:
        time = float(found.group(1))

      found = re.match(r""+current+": (.*)", line)
      if found:
        testsuiterep.append(found.group(1))

      found = re.match(r""+next+": (.*)", line)
      if found:
        testsuitename = os.path.basename(re.sub(r"^Test[ \t]*command:[ \t]*(.*)[\r|\n]+$", "\\1", testsuiterep[0]))
        testsuite = TestSuite().fromUnity(testsuiterep, name=testsuitename, hostname=hostname, time=time)
        testsuites.append(testsuite)

        current = str(int(current) + 1)
        next    = str(int(next) + 1)
        time = 0.0
        testsuiterep = []
        testsuiterep.append(found.group(1))

    testsuitename = os.path.basename(re.sub(r"^Test[ \t]*command:[ \t]*(.*)[\r|\n]+$", "\\1", testsuiterep[0]))
    testsuite = TestSuite().fromUnity(testsuiterep, name=testsuitename, hostname=hostname, time=time)
    testsuites.append(testsuite)
    return testsuites

class TestCase():
  def __init__(self, name="", error=False, ignore=False, time=0.0):
    self.name     = name
    self.time     = time
    self.error    = error
    self.ignore   = ignore
    self.message  = ''
    self.vmessage = ''

  def failed(self):
    return self.error

  def skipped(self):
    return self.ignore

  def getName(self):
    return self.name

  def getTime(self):
    return self.time

  def getMessage(self):
    return self.message

  def getVerboseMessage(self):
    return self.vmessage

  def setMessage(self, message):
    self.message = message

  def setVerboseMessage(self, message):
    self.vmessage = message
