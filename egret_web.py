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
import os
import sqlite3
from flask import Flask, request, url_for, render_template, flash, Response, redirect
from contextlib import closing
import egret_api

# global variables (for sessions)
session = [] # Holds all of the strings currently being tested against the regex
allPass = [] # Currently passing strings from session
allFail = [] # Currently failing strings from session
current = [] # Holds all strings from last tested regex

# configuration
DEBUG = True

# create our application
app = Flask(__name__)
app.config.from_object(__name__)

@app.route('/', methods=['GET', 'POST'])
def process_submit():
    regex = request.args.get('regex')
    testString = request.args.get('testString')
    showGroups = request.args.get('showGroups')
    upload = request.args.get('upload') 
    
    if upload:
        uploadedStrings = upload.splitlines()
        for item in uploadedStrings:
            session.append(item)

    # empty regex --> return empty results
    if regex == None or regex == '':
        return render_template('egret.html',
                testString=testString, showGroups=showGroups, session=session)
    
    # run egret engine
    (passList, failList, errorMsg, warnings) = egret_api.run_egret(regex)
    
    
    # add passing/failing strings to session if they aren't already,
    # and if user wants them to be added
    # if sessionBox != "on":
    #     for item in passList:
    #         if item not in session: # avoids duplicates
    #             session.append(item)
    #     for item in failList:
    #         if item not in session: # avoids duplicates
    #             session.append(item)
    
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
    
    # clear previous results
    allPass[:] = []
    allFail[:] = []
        
    for item in passList:
        if item not in allPass:
            allPass.append(item)
            current.append(item)
    for item in failList:
        if item not in allFail:
            allFail.append(item)    
            current.append(item)

    # retest each string
    for item in session:
        if egret_api.run_test_string(regex, item) == "ACCEPTED":
            allPass.append(item)
        else:
            allFail.append(item)
    
    # clear session
    # session[:] = []
    
    # render webpage with current session
    return render_template('egret.html',
            regex=regex, testString=testString, showGroups=showGroups,
            passList=passList, failList=failList, errorMsg=errorMsg, warnings=warnings,
            groupHdr=groupHdr, groupRows=groupRows, numGroups=numGroups,
            testResult=testResult, session=session, allPass=allPass, allFail=allFail)

@app.route('/download')
def download_file():
    data = []
    for item in session:
        data.append(item + '\n')
        
    return Response(data,
                    mimetype="text/plain",
                    headers={"Content-Disposition":
                            "attachment;filename=session.txt"})

@app.route('/upload', methods=['GET', 'POST'])
def upload_file():
    return render_template('upload.html')
   
@app.route('/clear')
def clear():
    session[:] = []
    return render_template('egret.html')

@app.route('/save', methods=["GET", "POST"])
def save():
    strings = []
    submit = request.args.get('submit')
    if submit == "Save selected strings":
        strings = request.args.getlist("save")
    else:
        strings = current
        
    for item in strings:
        if item not in session:
            session.append(item)
    return render_template('egret.html', session=session, allPass=allPass, allFail=allFail)
    
    
# strings to test with
# \b\d{3}[-.]?\d{3}[-.]?\d{4}\b phone numbers
# (?:#|0x)?(?:[0-9A-F]{2}){3,4} colors
            
if __name__ == '__main__':
    # app.run() # for default host/port
    app.run(host="0.0.0.0",port=8080) # for my dev environment