/*  egret_ext.cpp: Python interface for C++ engine

    Copyright (C) 2016-2018  Eric Larson and Anna Kirk
    elarson@seattleu.edu

    This file is part of EGRET.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Python.h>
#include <string>
#include <vector>
#include "egret.h"
using namespace std;

static PyObject *EgretExtError;

static PyObject *
egret_run(PyObject *self, PyObject *args)
{
  const char *regex;
  const char *base_substring;
  int check_mode;
  int web_mode;
  int debug_mode;
  int stat_mode;

  if (!PyArg_ParseTuple(args, "sspppp", &regex, &base_substring,
        &check_mode, &web_mode, &debug_mode, &stat_mode))
    return NULL;

  vector <string> tests =
    run_engine(regex, base_substring, check_mode, web_mode, debug_mode, stat_mode);

  PyObject *list = PyList_New(0);
  vector <string>::iterator it;
  for (it = tests.begin(); it != tests.end(); it++) {
    PyList_Append(list, PyUnicode_FromString((*it).c_str()));
  }

  return list;
}

static PyMethodDef EgretExtMethods[] = {
  {"run", egret_run, METH_VARARGS, "Run EGRET."},
  {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef egret_extmodule = {
  PyModuleDef_HEAD_INIT,
  "egret_ext", /* name of module */
  NULL,       /* module documentation, may be NULL */
  -1,         /* size of per-interpreter state of the module,
               or -1 if the module keeps state in global variables. */
  EgretExtMethods
};

extern "C" {

  PyMODINIT_FUNC
  PyInit_egret_ext(void)
  {
    PyObject *m;

    m = PyModule_Create(&egret_extmodule);
    if (m == NULL)
      return NULL;

    EgretExtError = PyErr_NewException("egret_ext.error", NULL, NULL);
    Py_INCREF(EgretExtError);
    PyModule_AddObject(m, "error", EgretExtError);
    return m;
  }
}
