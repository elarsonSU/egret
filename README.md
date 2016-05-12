# egret
Evil generation of regular expression test strings

Licensing and copying information:
----------------------------------

This software is distributed under the GNU General Public License version 3.
See the file LICENSE included in this distribution for a copy of this license.
If the file is not present, see <http://www.gnu.org/licenses/>

Dependencies:
-------------
EGRET requires Python 3.4 or later (due to using re.fullmatch).
Python 2.x versions will not work.

Setup directions:
-----------------

1. Execute: cd src

2. You may need to modify these Makefile variables:
CXX      := g++
PYTHON   := python3
EXT_PATH := build/lib.linux-x86_64-3.4
EXT_LIB  := egret_ext.cpython-34m.so

The latter two variables refer to the location and name of a library that is created.
The library contains C++ code but is used as a Python module. The precise name of the
path and library may vary. To find out the name, simply make the project once and then
analyze the build subdirectory that is created.
 
3. Execute: make

4. Execute: cd ..

5. You should see the library (the egret_ext .so file) in this directory.

6. Edit the first line of egret.py to refer to your python3 location.

Running:
--------
Simply run 'egret.py'.  It will prompt for a regular expression and then generate
test strings.

The script also some command line options.  To see a list, execute: egret.py -h

Acknowledgments:
----------------
A portion of EGRET was derived from a RE->NFA converter developed by Eli Bendersky.

Contact Information:
--------------------
Eric Larson
elarson@seattleu.edu
