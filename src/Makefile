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

CXX      := g++
PYTHON   := python3
EXT_PATH := build/lib.linux-x86_64-3.4
EXT_LIB  := egret_ext.cpython-34m.so

CXXFLAGS := -Wall -I. -g -O0 -fPIC

SRC := egret.cpp Scanner.cpp ParseTree.cpp CharSet.cpp NFA.cpp Transition.cpp State.cpp Stats.cpp error.cpp
HDR := Scanner.h ParseTree.h CharSet.h NFA.h Transition.h State.h Stats.h error.h
OBJ := $(patsubst %.cpp, %.o, $(SRC))

all: libegret.a egret_ext

libegret.a: $(OBJ)
	ar -r $@ $(OBJ)

%.o:	%.cpp $(HDR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

egret_ext: libegret.a
	rm -rf $(EXT_PATH)/$(EXT_LIB)
	$(PYTHON) create_ext.py build
	cp -f $(EXT_PATH)/$(EXT_LIB) ..

clean:
	rm -f libegret.a *.o
	rm -rf build
	rm -rf egret
	rm -rf ../$(EXT_LIB)
