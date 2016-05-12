# egret_web.py: Web interface for EGRET using Flask
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


# all the imports
import sqlite3
from flask import Flask, request, url_for, render_template, flash
from contextlib import closing
import egret_api

# configuration
DEBUG = True

# create our application
app = Flask(__name__)
app.config.from_object(__name__)

@app.route('/', methods=['GET'])
def process_submit():
    regex = request.args.get('regex')
    testString = request.args.get('testString')
    showGroups = request.args.get('showGroups')

    # empty regex --> return empty results
    if regex == None or regex == '':
        return render_template('egret.html',
                testString=testString, showGroups=showGroups)

    # run egret engine
    (passList, failList, errorMsg, warnings) = egret_api.run_egret(regex)

    # get group information
    if showGroups == "on" and errorMsg == None:
        (groupHdr, groupRows, numGroups) = egret_api.get_group_info(regex, passList)
    else:
        groupHdr = groupRows = numGroups = None
           
    # determine if test string is accepted or not
    if testString != None and testString != '' and errorMsg == None:
        testResult = egret_api.run_test_string(regex, testString)
    else:
        testResult = ''

    # render webpage
    return render_template('egret.html',
            regex=regex, testString=testString, showGroups=showGroups,
            passList=passList, failList=failList, errorMsg=errorMsg, warnings=warnings,
            groupHdr=groupHdr, groupRows=groupRows, numGroups=numGroups,
            testResult=testResult)
            
if __name__ == '__main__':
    app.run()

