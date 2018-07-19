# create_ext.py: Creates egret_ext Python library
#
# Copyright (C) 2016-2018  Eric Larson and Anna Kirk
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

from distutils.core import setup, Extension

module1 = Extension('egret_ext',
                    sources = ['egret_ext.cpp'],
                    libraries = ['egret'],
                    library_dirs = ['.'])

setup(name = 'Egret',
      version = '1.0',
      description = 'Egret engine interface',
      ext_modules = [module1])
