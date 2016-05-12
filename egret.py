#!/usr/local/bin/python3

# egret.py: Command line interface for EGRET
#
# Copyright (C) 2016  Eric Larson and Anna Kirk
# elarson@seattleu.edu
# 
# This file is part of EGRET.
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
import egret_api
from optparse import OptionParser
#import time

# Precondition: regexStr successfully compiles and all strings in testStrings
# match the regular expression
def get_group_info(regexStr, testStrings):
   # check for empty list
   if len(testStrings) == 0:
       return {}

   # compile regex
   regex = re.compile(regexStr)

   # determine if there are named groups, numbered groups, or no groups
   match = regex.fullmatch(testStrings[0])
   if len(match.groupdict()) != 0:
       useNames = True
   elif len(match.groups()) != 0:
       useNames = False
   else:
       return None

   # get groups for each string
   groupDict = {}
   for testStr in testStrings:
       match = regex.fullmatch(testStr)
       if useNames:
           groupDict[testStr] = match.groupdict()
       else:
           groupDict[testStr] = match.groups()

   return groupDict

parser = OptionParser()
parser.add_option("-f", "--file", dest = "fileName", help = "file containing regex")
parser.add_option("-r", "--regex", dest = "regex", help = "regular expression")
parser.add_option("-o", "--output_file", dest = "outputFile", help = "output file name")
parser.add_option("-d", "--debug", action = "store_true", dest = "debugMode",
    default = False, help = "display debug info")
parser.add_option("-s", "--stat", action = "store_true", dest = "statMode",
    default = False, help = "display stats")
parser.add_option("-g", "--groups", action = "store_true", dest = "showGroups",
    default = False, help = "show groups")
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

# execute regex-test
#start_time = time.process_time()
inputStrs = egret_ext.run(regexStr, opts.debugMode, opts.statMode)
status = inputStrs[0]
inputStrs = inputStrs[1:]
hasError = (status[0:5] == "ERROR")
hasWarning = (not hasError and status != "SUCCESS")

if not hasError:

  # test each string against the regex
  regex = re.compile(regexStr)
  matches = []
  nonMatches = []
  for inputStr in inputStrs:
    search = regex.fullmatch(inputStr)
    if search:
        matches.append(inputStr)
    else:
        nonMatches.append(inputStr)
  #elapsed_time = time.process_time() - start_time

  # display groups if requested
  if opts.showGroups:
      groupDict = get_group_info(regexStr, matches)
      if groupDict == None:
          showGroups = False
          if hasWarning:
              status = status + "Regex does not have any capturing groups\n"
          else:
              hasWarning = True
              status = "Regex does not have any capturing groups\n"
      else:
          showGroups = True
          maxLength = 7 # smallest size of format
          for inputStr in matches:
              if len(inputStr) > maxLength:
                  maxLength = len(inputStr)
          groupFmt = "{0:" + str(maxLength) + "}  {1}"
  else:
      showGroups = False

  # print the stats
  if opts.statMode:
    fmt = "{0:30}| {1}"
    print("--------------------------------------")
    print(fmt.format("Matches", len(matches)))
    print(fmt.format("Non-matches", len(nonMatches)))
    #print(fmt.format("Time", elapsed_time))


# write the output header
header = "Regex: " + regexStr + "\n\n"
if descStr != "":
    header += ("Description: " + descStr + "\n\n")
if hasError:
    header += (status + "\n")
elif hasWarning:
    header += ("Warnings:\n" + status + "\n")

if opts.outputFile:
    outFile = open(opts.outputFile, 'w')
    outFile.write(header)
    if hasError:
        outFile.close();
        sys.exit(-1);
else:
    print()
    print(header, end='')
    if hasError:
        sys.exit(-1);

# print the match strings
if opts.outputFile:
    outFile.write("Matches:\n")
else:
    print("Matches:")
for inputStr in sorted(matches):
    if inputStr == "":
        dispStr = "<empty>"
    else:
        dispStr = inputStr

    if showGroups:
        dispStr = groupFmt.format(dispStr, str(groupDict[inputStr]))

    if opts.outputFile:
        outFile.write(dispStr + "\n")
    else:
        print(dispStr)

# print the non match strings
if opts.outputFile:
    outFile.write("\nNon-matches:\n")
else:
    print("\nNon-matches:")
for inputStr in sorted(nonMatches):
    if inputStr == "":
        dispStr = "<empty>"
    else:
        dispStr = inputStr

    if opts.outputFile:
        outFile.write(dispStr + "\n")
    else:
        print(dispStr)

# close the output
if opts.outputFile:
    outFile.close()

sys.exit(0)
