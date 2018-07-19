# egret_web.py: Web interface for EGRET using Flask
#
# Copyright (C) 2016-2018  Eric Larson and Nicolas Oman
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

import os
from flask import Flask, request, url_for, render_template, Response, Markup
import egret_web_api

# EGRET global variables
test_strings = [] # Holds all of the strings currently being tested against the regex
egret = {}
egret['baseSubstr'] = ''
egret['testString'] = ''
egret['showGroups'] = False
egret['useDiffBase'] = False
UPLOAD_FOLDER = '/tmp' # Uploads module requires this to be set, but nothing is actually saved there
ALLOWED_EXTENSIONS = set(['txt'])

# configuration
DEBUG = True

# create our application
app = Flask(__name__)
app.config.from_object(__name__)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

def run_acre(regex):
  if regex != '':
    (result, errorMsg) = egret_web_api.run_acre(regex)
    if result:
      result = Markup(result)
    return (result, errorMsg)

  return (None, None)

def run_egret(regex):
  global test_strings
  global egret

  # get data from text boxes
  if 'baseSubstr' in request.form:
    egret['baseSubstr'] = request.form.get('baseSubstr')
  if 'testString' in request.form:
    egret['testString'] = request.form.get('testString')

  # process checkbox options
  if request.method == 'POST':
    egret['showGroups'] = 'showGroups' in request.form
    egret['useDiffBase'] = 'useDiffBase' in request.form
        
  # process saved string options
  addedStrs = []
  if 'addTestString' in request.form:
    addedStrs = [ egret['testString'] ]
  elif 'addSelectedAccept' in request.form:
    addedStrs = request.form.getlist('accept')
  elif 'addAccept' in request.form:
    addedStrs = egret['passList']
  elif 'addSelectedReject' in request.form:
    addedStrs = request.form.getlist('reject')
  elif 'addReject' in request.form:
    addedStrs = egret['failList']
  elif 'deleteSelected' in request.form:
    deletedStrs = request.form.getlist('delete')
    for s in deletedStrs:
      test_strings.remove(s)
  elif 'deleteAll' in request.form:
    test_strings = []

  for item in addedStrs:
    if item not in test_strings:
      test_strings.append(item)

  # run egret engine
  if egret['useDiffBase']:
    baseSubstr = egret['baseSubstr']
  else:
    baseSubstr = 'evil'
      
  if regex != '':
    (egret['passList'], egret['failList'], egret['errorMsg'], egret['warnings']) = \
      egret_web_api.run_egret(regex, baseSubstr, test_strings)
  else:
    (egret['passList'], egret['failList'], egret['errorMsg'], egret['warnings']) = \
      ([], [], None, None)

  if egret['warnings']:
    egret['warnings'] = Markup(egret['warnings'])
    
  # get group information
  if regex != '' and egret['showGroups'] and egret['errorMsg'] == None:
    (egret['groupHdr'], egret['groupRows'], egret['numGroups']) = \
      egret_web_api.get_group_info(regex, egret['passList'])
  else:
    (egret['groupHdr'], egret['groupRows'], egret['numGroups']) = \
      (None, None, None)
    
    
  # determine if test string is accepted or not
  if regex != '' and egret['testString'] != '' and egret['errorMsg'] == None:
    egret['testResult'] = egret_web_api.run_test_string(regex, egret['testString'])
  else:
    egret['testResult'] = ''
    
def allowed_file(filename):
  return '.' in filename and filename.rsplit('.', 1)[1] in ALLOWED_EXTENSIONS
           
# ROUTES

@app.route('/', methods=['GET', 'POST'])
def process_submit():

  if 'regex' in request.form:
    regex = request.form.get('regex')
  else:
    regex = ''

  # run tools
  (acre_result, acre_error) = run_acre(regex)
  run_egret(regex)
    
  # render webpage
  return render_template('egret.html', regex=regex, egret=egret, test_strings=test_strings, \
    acre_result=acre_result, acre_error=acre_error)
            
@app.route('/download')
def download_file():
  contents = []
  for item in test_strings:
    contents.append(item + '\n')
        
  return Response(contents,
    mimetype='text/plain',
    headers={'Content-Disposition':'attachment;filename=test_strings.txt'})

@app.route('/upload', methods=['GET', 'POST'])
def upload_file():
  if request.method == 'POST':
    infile = request.files['file']
    # check for no file selected
    if request.files['file'].filename == '':
      return render_template('upload.html',
        uploadError='No file selected')

    # if filename isn't allowed
    if not allowed_file(infile.filename):
      return render_template('upload.html',
        uploadError='Wrong filetype. Please upload a \'.txt\' file')

    # if correctly uploaded and allowed filetype
    if infile and allowed_file(infile.filename):
      lines = infile.readlines()
      new_strings = []
      # decode byte lists into strings
      for item in lines:
        new_string = item.decode("utf-8") 
        new_strings.append(new_string.rstrip())
      # transfer content into test strings
      for item in new_strings:
        if item not in test_strings:
          test_strings.append(item)
      return process_submit()

  return render_template('upload.html')

if __name__ == '__main__':
    #app.run() # for default host/port
    app.run(host='0.0.0.0',port=8080) # for my dev environment

