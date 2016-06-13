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
formData = {}

# configuration
DEBUG = True

# create our application
app = Flask(__name__)
app.config.from_object(__name__)

@app.route('/', methods=['GET', 'POST'])
def process_submit():
    global session
    
    # get data from text boxes
    regex = request.form.get('regex')
    baseSubstr = request.form.get('baseSubstr')
    testString = request.form.get('testString')
    formData['regex'] = regex
    formData['baseSubstr'] = baseSubstr
    formData['testString'] = testString

    # process checkbox options
    if "showGroups" in request.form:
      formData['showGroups'] = True
    else:
      formData['showGroups'] = False
    if "useDiffBase" in request.form:
      formData['useDiffBase'] = True
    else:
      formData['useDiffBase'] = False
      baseSubstr = "evil"
        
    # process saved string options
    addedStrs = []
    if "addTestString" in request.form:
        addedStrs = [ testString ]
    if "addSelectedAccept" in request.form:
        addedStrs = request.form.getlist("accept")
    elif "addAccept" in request.form:
        addedStrs = formData['passList']
    elif "addSelectedReject" in request.form:
        addedStrs = request.form.getlist("reject")
    elif "addReject" in request.form:
        addedStrs = formData['failList']
    elif "deleteSelected" in request.form:
        deletedStrs = request.form.getlist("delete")
        for s in deletedStrs:
            session.remove(s)
    elif "deleteAll" in request.form:
        session = []

    for item in addedStrs:
        if item not in session:
            session.append(item)

    # uploads strings to session
    #upload = request.form.get('upload') 
    #if upload:
    #    uploadedStrings = upload.splitlines()
    #    for item in uploadedStrings:
    #        session.append(item)

    # empty regex --> return empty results
    if regex == None or regex == '':
        formData['regex'] = ''
        formData['baseSubstr'] = ''
        formData['testString'] = ''
        return render_template('egret.html', formData=formData, session=session)
    
    # run egret engine
    (passList, failList, errorMsg, warnings) = egret_api.run_egret(regex, baseSubstr, session)
    formData['passList'] = passList
    formData['failList'] = failList
    formData['errorMsg'] = errorMsg
    formData['warnings'] = warnings
    
    # get group information
    if formData['showGroups'] and errorMsg == None:
        (groupHdr, groupRows, numGroups) = egret_api.get_group_info(regex, passList)
        formData['groupHdr'] = groupHdr
        formData['groupRows'] = groupRows
        formData['numGroups'] = groupRows
    else:
        formData['groupHdr'] = None
        formData['groupRows'] = None
        formData['numGroups'] = None
    
    
    # determine if test string is accepted or not
    if testString != None and testString != '' and errorMsg == None:
        formData['testResult'] = egret_api.run_test_string(regex, testString)
    else:
        formData['testResult'] = ''
    
    # clear previous results
    current = sorted(list(set(passList) | set(failList)))
    
    # render webpage with current session
    return render_template('egret.html', formData=formData, session=session)
            
@app.route('/regex_gen', methods=['GET', 'POST'])
def generate_regex():
    return render_template('regex_gen.html')

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

   
# Some sample test strings
# \b\d{3}[-.]?\d{3}[-.]?\d{4}\b   phone numbers
# (?:#|0x)?(?:[0-9A-F]{2}){3,4}   colors
# (IMG\d+)\.png                   names + .png (useful for testing groups)
            
if __name__ == '__main__':
    #app.run() # for default host/port
    app.run(host="0.0.0.0",port=8080) # for my dev environment
