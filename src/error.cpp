/*  error.cpp: Error processing

    Copyright (C) 2016  Eric Larson and Anna Kirk
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

#include <string>
#include "error.h"
using namespace std;

static string alerts = "";

void
clearAlerts()
{
  alerts = "";
}

void
addWarning(string type, string message)
{
  string warning = "WARNING (" + type + "): " + message;
  alerts += warning;
  alerts += "\n";
}

void
addViolation(string type, string message)
{
  string violation = "VIOLATION (" + type + "): " + message;
  alerts += violation;
  alerts += "\n";
}

string
getAlerts()
{
  return alerts;
}
