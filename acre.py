# acre.py: Command line interface for ACRE (Automatic Checking of Regular Expressions)
#
# Copyright (C) 2016-2018  Eric Larson
# elarson@seattleu.edu
# 
# This file is part of EGRET / ACRE.
# 
# This program is free software: you can redistribute it and/or modify
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

import re
import sys	
import egret_ext
from optparse import OptionParser
#import time

# process command line options
parser = OptionParser()
parser.add_option("-f", "--file", dest = "fileName", help = "file containing regex")
parser.add_option("-r", "--regex", dest = "regex", help = "regular expression")
parser.add_option("-o", "--output_file", dest = "outputFile", help = "output file name")
parser.add_option("-w", "--warn", action = "store_true", dest = "warnMode",
    default = False, help = "warn if example or fix are broke")
parser.add_option("-d", "--debug", action = "store_true", dest = "debugMode",
    default = False, help = "display debug info")
parser.add_option("-s", "--stat", action = "store_true", dest = "statMode",
    default = False, help = "display stats")
opts, args = parser.parse_args()

# check for valid command lines
if opts.fileName != None and opts.regex != None:
  print("Cannot specify both a regular expression and input file")
  sys.exit(-1)

# get the regular expression
descStr = ""
if opts.fileName != None:
  inFile = open(opts.fileName)
  regexStr = inFile.readline().rstrip()
  try:
    descStr = inFile.readline().rstrip()
  except:
    descStr = ""
  inFile.close()
elif opts.regex != None:
  regexStr = opts.regex
else:
  regexStr = input("Enter a Regular Expression: ")

# compile the regular expression
try: 
  hasError = False
  regex = re.compile(regexStr)

  # execute regex-test
  # start_time = time.process_time()
  alerts = egret_ext.run(regexStr, "evil", True, False, opts.debugMode, opts.statMode)
  # elapsed_time = time.process_time() - start_time

except re.error as e:
  status = "ERROR (compiler error): Regular expression did not compile: " + str(e) + "\n"
  alerts = [ status ]

#if opts.statMode:
#  fmt = "{0:30}| {1}"
#  print(fmt.format("Time", elapsed_time))

# write the output header
header = "Regex: " + regexStr + "\n\n"
if descStr != "":
  header += ("Description: " + descStr + "\n\n")
if opts.outputFile:
  outFile = open(opts.outputFile, 'w')
  outFile.write(header)
else:
  print(header, end='')

# write the alerts
for s in alerts:

  lines = s.split("\n")
  prevLine = ""
  for line in lines: 

    suppressLine = False
    example = False
    anchorExample = False
    if re.match("\.\.\.String with", line) != None:
      example = True
      anchorExample = True
    if re.match("\.\.\.Example", line) != None:
      example = True

    if example:
      suppressLine = True
      first, second = line.split(':', 1)
      exampleStr = second[1:]
      success = re.fullmatch(regexStr, exampleStr) != None
      if anchorExample:
        if prevLine != "":
          if prevSuccess and success: # both anchor examples successful
            if opts.outputFile:
              outFile.write(prevLine)
              outFile.write('\n')
              outFile.write(line)
              outFile.write('\n')
            else:
              print(prevLine)
              print(line)
          else: # one line failed
            if opts.warnMode:
              status = "ATTENTION: EXAMPLE STRING NOT ACCEPTED"
              if opts.outputFile:
                outFile.write(prevLine)
                outFile.write('\n')
                if not prevSuccess:
                  outFile.write(status)
                  outFile.write('\n')
                outFile.write(line)
                outFile.write('\n')
                if not success:
                  outFile.write(status)
                  outFile.write('\n')
              else:
                print(prevLine)
                if not prevSuccess:
                  print(status)
                print(line)
                if not success:
                  print(status)
          prevLine = ""
        else:  # prevLine == ""
          prevLine = line
          prevSuccess = success
      else:  # example but not anchor example
        prevLine = ""
        if success:
          suppressLine = False
        else:
          if opts.warnMode:
            status = "ATTENTION: EXAMPLE STRING NOT ACCEPTED"
            if opts.outputFile:
              outFile.write(line)
              outFile.write('\n')
              outFile.write(status)
              outFile.write('\n')
            else:
              print(line)
              print(status)
    else: # not example
      prevLine = ""

    if re.match("\.\.\.Suggested fix", line) != None:
      first, second = line.split(':', 1)
      fixStr = second[1:]

      try:
        r = re.compile(fixStr)
      except re.error as e:
        suppressLine = True
        if opts.warnMode:
          status = "ATTENTION: SUGGESTED FIX DID NOT COMPILE: "  + str(e)
          if opts.outputFile:
            outFile.write(line)
            outFile.write('\n')
            outFile.write(status)
            outFile.write('\n')
          else:
            print(line)
            print(status)

    if not suppressLine:
      if opts.outputFile:
        outFile.write(line)
        outFile.write('\n')
      else:
        print(line)

# close the output
if opts.outputFile:
  outFile.close()

sys.exit(0)
