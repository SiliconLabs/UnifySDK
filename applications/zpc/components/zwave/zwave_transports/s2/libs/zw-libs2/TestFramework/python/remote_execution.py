#!/usr/bin/python
# Copyright (c) 2018 Silicon Laboratories Inc.

import sys
import os
import paramiko
import socket
import threading
import time
import re
from . import xunit
from distutils.version import StrictVersion

zw700versions = [(StrictVersion('0.1.9'), './bin/start-700_platform.sh -k ~/{} -r 256', './bin/start-700_platform.sh -s'),
                 (StrictVersion('0.1.8'), './bin/start-700_platform.sh -k ~/{} -r 256 -c', './bin/start-700_platform.sh -s'),
                 (StrictVersion('0.0.1'), './bin/start-700_platform.sh -k ~/{} -r 256',    'killall toplevel')]

DEBUG = 0

class RemoteTestSession():
  def __init__(self, server, username='developer', password='1234', timeout=60, xmlfile=None):
    self.server = server
    self.xmlfile = xmlfile
    self.testResult = []
    self.testTime   = 0.0
    self.commands   = zw700versions[-1]

    self.username = username
    self.password = password
    self.timeout  = timeout

    self.remotesession_stdin = None

    self.sshclient = paramiko.SSHClient()
    self.sshclient.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    if DEBUG: print('Connecting to: %s' % self.server)
    self.sshclient.connect(self.server, username=self.username, password=self.password)
    self.sftpsession = self.sshclient.open_sftp()
    self.getServerVersion()

  def put(self, localFile, remoteFile):
    # Use SFTP module to upload file
    self.sftpsession.put(localFile, remoteFile)

  def executeTest(self, remoteFile):
    self.testResult = []
    consoleThread = RemoteTestConsoleThread(self, self.server, 11236)
    consoleThread.start()
    if DEBUG: print('Running %s remotely' % remoteFile)
    remote_cmd = self.commands[1].format(remoteFile)
    stdin, stdout, stderr = self.sshclient.exec_command(remote_cmd)
    if DEBUG:
      s = stdout.read()
      if s:
        print('commands[1] stdout:')
        print('%s' % s)
      s = stderr.read()
      if s:
        print('commands[1] stderr:')
        print('%s' % s)
    stdin.close()
    stdout.close()
    stderr.close()
    consoleThread.join()

  def getTestResult(self):
    return self.testResult

  def getTestResultXml(self, testname):
    return xunit.TestSuite().fromUnity(self.testResult, name=testname, time=self.testTime, hostname=self.server)

  def setTestResult(self, testResult, time=0.0):
    self.testResult = testResult
    self.testTime   = time

  def endTest(self):
    remote_cmd = self.commands[2]
    stdin, stdout, stderr = self.sshclient.exec_command(remote_cmd)
    if DEBUG:
      s = stdout.read()
      if s:
        print('commands[2] stdout:')
        print('%s' % s)
      s = stderr.read()
      if s:
        print('commands[2] stderr:')
        print('%s' % s)
    stdin.close()
    stdout.close()
    stderr.close()

  def getTimeout(self):
    return self.timeout

  def getServerVersion(self):
    systemtype = 'Unknown'
    version    = 'Unknown'
    remote_cmd = './bin/start-700_platform.sh -v -k none'
    stdin, stdout, stderr = self.sshclient.exec_command(remote_cmd)
    versionstr = stdout.read()
    versionlist = re.findall(r".*;.*;?", versionstr, re.MULTILINE)
    if len(versionlist) > 0:
      versiongrp = re.match(r"([^;]*);([^;]*)(.*)?", versionlist[0])
      if versiongrp.group(1) is not None and versiongrp.group(1) != "": systemtype = versiongrp.group(1)
      if versiongrp.group(2) is not None and versiongrp.group(2) != "": version = versiongrp.group(2)
      for i in zw700versions:
        if StrictVersion(version) >= StrictVersion('%s' % (i[0])):
          if DEBUG: print('System version: %s, Version required: %s or newer' % (version, i[0]))
          self.commands = i
          break

    if DEBUG: print('Remote server: System: %s, Version: %s' % (systemtype, version))

  def close(self):
    self.sftpsession.close()
    self.sshclient.close()

class TimeoutMonitorThread(threading.Thread):

  def __init__(self, sshsession):
      super(TimeoutMonitorThread, self).__init__()
      self.sshsession = sshsession
      self.timeout    = sshsession.getTimeout()
      self.starttime  = 0
      self.stoptime   = 0
      self.status     = 'stopped'

  def run(self):
    if DEBUG: print('TimeoutMonitor started')
    self.starttime = time.time()
    self.timeouttime = self.starttime + self.timeout
    currentTime = time.time()
    while currentTime < self.timeouttime:
      time.sleep(0.2)
      currentTime = time.time()

    self.stoptime = time.time()
    if self.timeouttime and (currentTime >= self.timeouttime ):
      if DEBUG: print('Timeout reached')
      self.status     = 'expired'
    else:
      if DEBUG: print('TimeoutMonitor stopped')
      self.status     = 'stopped'
    self.sshsession.endTest()

  def resettimer(self):
    currentTime = time.time()
    self.timeouttime = currentTime + self.timeout

  def status(self):
      return self.status

  def timeElapsed(self):
      return self.stoptime - self.starttime

  def exit(self):
    self.timeouttime = 0

class RemoteTestConsoleThread(threading.Thread):

  def __init__(self, sshsession, host, port):
      super(RemoteTestConsoleThread, self).__init__()
      self.sshsession = sshsession
      self.host = host
      self.port = port

  def run(self):
    self.running = 50
    receive_buffer = ''
    timeoutThread = TimeoutMonitorThread(self.sshsession)
    timeoutThread.start()
    while self.running:
      try:
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect((self.host, self.port))
        if DEBUG: print('Connected %s:%d' % (self.host, self.port))
        while self.running:
          try:
            if not self.running: break
            new_data = self.socket.recv(1024)
            if not new_data: break
            timeoutThread.resettimer()
            receive_buffer += new_data

            retval = re.search(r"\x04", new_data)
            if retval:
              if DEBUG: print('Test completed')
              self.running = 0
              timeoutThread.exit()
              self.sshsession.endTest()

          except IOError as e:
            if DEBUG: print('Connection has been closed / 2')
            self.running = 0

        self.socket.close()
      except IOError as e:
        if DEBUG: print('Connection closed / 1')
      if self.running: self.running -= 1
      if self.running:
        time.sleep(0.2)
    console_output = receive_buffer.splitlines()

    # make sure timeout thread is stopped and joined
    timeoutThread.exit()
    timeoutThread.join()

    if timeoutThread.status == 'expired':
      summary = '%i Tests 1 Failures 0 Ignored' % ((len(console_output)+1), )
      console_output.append(':9999:Unknown:FAIL: Timeout')
      console_output.append('-----------------------')
      console_output.append(summary)
      console_output.append('FAIL')
    totaltime = timeoutThread.timeElapsed()
    self.sshsession.setTestResult(console_output, totaltime)

  def exit(self):
    if self.running == 1:
      self.running = 0
      try:
        eof_message = b''
        self.socket.shutdown(socket.SHUT_RDWR)
        self.socket.sendto(eof_message, (self.host, self.port))
      except IOError as e:
        if DEBUG: print('Connection exit / 4')
      self.socket.close()

def usage():
  print('Usage')
  print('')
  print('  remote_execution.py [options]')
  print('')
  print('Options')
  print('  -h, --help                             = Print this help text.')
  print('  -V, --verbose                          = Enable verbose output')
  print('  -r, --remote-target <target>           = Remote target for test execution (Currently only SSH targets are supported)')
  print('                                           <target> can be dns name or ip of remote system')
  print('  -u, --user <username>                  = Username when accessing remote test system (only for SSH targets), default=developer')
  print('  -p, --password <password>              = Password for user specified by \'-u\' or \'--user\' (only for SSH targets), , default=1234')
  print('  -f, --file <file>                      = Test file (executable) to upload to remote target')
  print('  -t, --timeout <seconds>                = Timeout before terminating a stalled test in seconds, default=10 seconds')
  print('  -o, --out <file>                       = Test report in Xunit xml format')

def main(argv):
  import getopt
  opts, args = getopt.getopt(sys.argv[1:], 'hVr:u:p:f:t:o:', ['help', 'verbose', 'remote-target=', 'user=', 'password=', 'file=', 'timeout=', 'out='])

  remote_target = ""
  username      = "developer"
  password      = "1234"
  file          = ""
  timeout       = 10
  xml           = None
  xmlfile       = None

  global DEBUG

  for opt, arg in opts:
    if opt in ('-h', '--help'):
      usage()
      sys.exit(0)
    elif opt in ('-V', '--verbose'):
      DEBUG = True
    elif opt in ('-r', '--remote-target'):
      remote_target = arg
    elif opt in ('-u', '--user'):
      username = arg
    elif opt in ('-p', '--password'):
      password = arg
    elif opt in ('-f', '--file'):
      file = arg
    elif opt in ('-t', '--timeout'):
      timeout = int(arg)
    elif opt in ('-o', '--out'):
      from . import xunit
      import xml.etree.ElementTree as ElementTree
      xml     = xunit.Xunit()
      xmlfile = arg

  test_session = RemoteTestSession(remote_target, username, password, timeout=timeout)
  testsuitename = os.path.basename(file)
  remote_file = 'projects/700series/images/%s' % testsuitename
  if DEBUG: print('Uploading: %s' % testsuitename)
  test_session.put(file, remote_file)

  test_session.executeTest(remote_file)
  if xmlfile is not None:
      xml.append(test_session.getTestResultXml(testsuitename))
      ElementTree.ElementTree(xml.getTree()).write(xmlfile, encoding="UTF-8", xml_declaration=True)
  if DEBUG:
    result = test_session.getTestResult()
    for line in result:
      print(line)


if __name__ == "__main__":
    main(sys.argv[1:])
