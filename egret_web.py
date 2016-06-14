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
data = {}
data['regex'] = ''
data['baseSubstr'] = ''
data['testString'] = ''
data['showGroups'] = False
data['useDiffBase'] = False

# configuration
DEBUG = True

# create our application
app = Flask(__name__)
app.config.from_object(__name__)

@app.route('/', methods=['GET', 'POST'])
def process_submit():
    global session
    
    # get data from text boxes
    if 'regex' in request.form:
        data['regex'] = request.form.get('regex')
    if 'baseSubstr' in request.form:
        data['baseSubstr'] = request.form.get('baseSubstr')
    if 'testString' in request.form:
        data['testString'] = request.form.get('testString')

    # process checkbox options
    if request.method == 'POST':
        data['showGroups'] = 'showGroups' in request.form
        data['useDiffBase'] = 'useDiffBase' in request.form
        
    # process saved string options
    addedStrs = []
    if 'addTestString' in request.form:
      addedStrs = [ testString ]
    elif 'addSelectedAccept' in request.form:
      addedStrs = request.form.getlist('accept')
    elif 'addAccept' in request.form:
      addedStrs = data['passList']
    elif 'addSelectedReject' in request.form:
      addedStrs = request.form.getlist('reject')
    elif 'addReject' in request.form:
      addedStrs = data['failList']
    elif 'deleteSelected' in request.form:
      deletedStrs = request.form.getlist('delete')
      for s in deletedStrs:
        session.remove(s)
    elif 'deleteAll' in request.form:
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
    if data['regex'] == '':
      return render_template('egret.html', data=data, session=session)
    
    # run egret engine
    if data['useDiffBase']:
      baseSubstr = data['baseSubstr']
    else:
      baseSubstr = 'evil'
    (data['passList'], data['failList'], data['errorMsg'], data['warnings']) = \
        egret_api.run_egret(data['regex'], baseSubstr, session)
    
    # get group information
    if data['showGroups'] and data['errorMsg'] == None:
        (data['groupHdr'], data['groupRows'], data['numGroups']) = \
            egret_api.get_group_info(data['regex'], data['passList'])
    else:
        (data['groupHdr'], data['groupRows'], data['numGroups']) = \
            (None, None, None)
    
    
    # determine if test string is accepted or not
    if data['testString'] != '' and data['errorMsg'] == None:
        data['testResult'] = egret_api.run_test_string(data['regex'], data['testString'])
    else:
        data['testResult'] = ''
    
    # render webpage
    return render_template('egret.html', data=data, session=session)
            
@app.route('/regex_gen', methods=['GET', 'POST'])
def generate_regex():
    return render_template('regex_gen.html')

@app.route('/download')
def download_file():
    data = []
    for item in session:
        data.append(item + '\n')
        
    return Response(data,
                    mimetype='text/plain',
                    headers={'Content-Disposition':
                            'attachment;filename=session.txt'})

@app.route('/upload', methods=['GET', 'POST'])
def upload_file():
    return render_template('upload.html')

   
# Some sample test strings
# \b\d{3}[-.]?\d{3}[-.]?\d{4}\b   phone numbers
# (?:#|0x)?(?:[0-9A-F]{2}){3,4}   colors
# (IMG\d+)\.png                   names + .png (useful for testing groups)
            
if __name__ == '__main__':
    #app.run() # for default host/port
    app.run(host='0.0.0.0',port=8080) # for my dev environment
