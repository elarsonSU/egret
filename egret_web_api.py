# egret_web_api.py: API for EGRET web interface
#
# Copyright (C) 2016-2018  Eric Larson, Anna Kirk, and Nicolas Oman
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
import egret_ext

def run_egret(regexStr, baseSubstring, testList):
    try:
        regex = re.compile(regexStr)
    except re.error as e:
        status = "ERROR (compiler error): Regular expression did not compile: " + str(e)
        return ([], [], status, [])
        
    inputStrs = egret_ext.run(regexStr, baseSubstring, False, True, False, False)

    idx = 0
    line = inputStrs[idx]
    if line[0:5] == "ERROR":
        return ([], [], line, [])

    while line != "BEGIN":
      idx += 1;
      line = inputStrs[idx]

    if idx == 0:
      alerts = []
      inputStrs = inputStrs[1:]
    else:
      alerts = inputStrs[:idx]
      inputStrs = inputStrs[idx+1:]

    warnings = ""
    for a in alerts:
      warnings += a

    matches = []
    nonMatches = []

    inputStrs = sorted(list(set(inputStrs) | set(testList)))
    
    for inputStr in inputStrs:
        search = regex.fullmatch(inputStr)
        if search:
            matches.append(inputStr)
        else:
            nonMatches.append(inputStr)

    return (matches, nonMatches, None, warnings)

# Precondition: regexStr successfully compiles
def run_test_string(regexStr, testStr):
    regex = re.compile(regexStr)
    if regex.fullmatch(testStr):
        return "ACCEPTED"
    else:
        return "REJECTED"

# Precondition: regexStr successfully compiles and all strings in testStrings
# match the regular expression
def get_group_info(regexStr, testStrings):
    # check for empty list
    if len(testStrings) == 0:
        return (None, None, None)

    # compile regex
    regex = re.compile(regexStr)

    # determine if there are named groups, numbered groups, or no groups
    match = regex.fullmatch(testStrings[0])
    if len(match.groupdict()) != 0:
        useNames = True
        names = list(match.groupdict().keys())
        nameList = []
        for name in names:
            r = r"\?P<" + name
            start = re.search(r, regexStr).start()
            nameList.append((start, name))
        nameList = sorted(nameList)
        groupHdr = [ name for (start, name) in nameList ]
    elif len(match.groups()) != 0:
        useNames = False
        groupHdr = [ str(i) for i in range(0, len(match.groups())) ]
    else:
        return (None, None, None)

    # get groups for each string
    groupRows = []
    for testStr in testStrings:
        match = regex.fullmatch(testStr)
        if useNames:
            g = match.groupdict()
            row = []
            for i in groupHdr:
                row.append(g[i])
        else:
            row = list(match.groups())
        row.insert(0, testStr)
        groupRows.append(row)

    groupHdr.insert(0, 'String')
    return (groupHdr, groupRows, len(groupHdr) - 1)

def run_acre(regexStr):
  try:
    regex = re.compile(regexStr)
  except re.error as e:
    errorMsg = "ERROR (compiler error): Regular expression did not compile: " + str(e)
    return (None, errorMsg)
        
  alerts = egret_ext.run(regexStr, "evil", True, True, False, False)

  first_line = alerts[0]
  if first_line[0:5] == "ERROR":
    return (None, first_line)

  status = ""
  for s in alerts:

    lines = s.split("<br>")
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
              status += prevLine
              status += '<br>'
              status += line
              status += '<br>'
            prevLine = ""
          else:  # prevLine == ""
            prevLine = line
            prevSuccess = success
        else:  # example but not anchor example
          prevLine = ""
          if success:
            suppressLine = False
      else: # not example
        prevLine = ""

      if re.match("\.\.\.Suggested fix", line) != None:
        first, second = line.split(':', 1)
        fixStr = second[1:]

        try:
          r = re.compile(fixStr)
        except re.error as e:
          suppressLine = True

      if not suppressLine:
        status += line
        status += '<br>'

  # get rid of line breaks at end (eliminates extra space at the end)
  while (status[-4:] == "<br>"):
    status = status[0:-4]

  return (status, None)
