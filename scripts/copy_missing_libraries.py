#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# WengoPhone, a voice over Internet phone
# Copyright (C) 2004-2007  Wengo
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# @author Philippe Bernery <philippe.bernery@wengo.com>
#
# This script copy libraries linked libraries found in 1st argument and
# not installed by default on the system and not found in 1st argument.
# These libraries are usually in /opt/local/lib, /sw/lib, /usr/local/lib, etc...
import binary_utilities, os, platform, shutil, sys

USAGE='''
usage:
copy_missing_libraries.py <dir>
copy_missing_libraries.py <file> <output_dir>
'''

def copy_missing_libraries_of_file(root, path):
	'''
	Copy missing libraries of file 'path'.
	A library is missing if it is not in 'root' or if it is not a system lib.
	'''
	# Change install names
	linkedLib = binary_utilities.get_linked_libraries_of_file(path)
	for lib in linkedLib:
		newLibLink = binary_utilities.get_library_basename(lib)
		if not binary_utilities.is_system_library(lib) \
			and not os.path.exists(os.path.join(root, newLibLink)):
			print "Copying", lib, "in", root + "..."
			shutil.copy(lib, root)
			# Call recursively copy_missing_libraries_of_file because adding 
			# newLibLink may have added some new dependencies.
			copy_missing_libraries_of_file(root, os.path.join(root, newLibLink))

def copy_missing_libraries(path):
	'''
	Copy missing libraries in dir 'path'.
	'''
	for root, dirs, files in os.walk(path):
		for f in files:
			absSrcFile = os.path.join(root, f)
			if binary_utilities.is_binary_file(absSrcFile):
				copy_missing_libraries_of_file(path, absSrcFile)

if __name__ == "__main__":
	if len(sys.argv) == 2:
		copy_missing_libraries(sys.argv[1])
	elif len(sys.argv) == 3:
		copy_missing_libraries_of_file(sys.argv[2], sys.argv[1])
	else:
		print USAGE
		sys.exit(1)
