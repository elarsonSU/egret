# egret_api.py: API for EGRET web interface
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
import egret_ext

def run_egret(regexStr, baseSubstring, testList):
    try:
        regex = re.compile(regexStr)
    except re.error as e:
        status = "ERROR (compiler error): Regular expression did not compile: " + e.msg
        return ([], [], status, [])
        
    inputStrs = egret_ext.run(regexStr, baseSubstring, False, False, False)
    status = inputStrs[0]
    if status[0:5] == "ERROR":
        return ([], [], status, [])
    elif status == "SUCCESS":
        warnings = None
    else:
        warnings = status.rstrip().split("\n")

    inputStrs = inputStrs[1:]
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
