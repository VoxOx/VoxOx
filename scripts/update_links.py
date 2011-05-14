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
# This script updates library links of Mach-O files in a directory
# (1st parameter) and its sub-directories. If a prefix is given, it is added 
# in front of links.
# It can also updates links of a binary file given a path to a directory
# that contains several libraries.
import binary_utilities, os, re, sys

USAGE='''
update_links.py <path> [<executable>]
\tThis syntax will update all install names in path.
\tIf executable is given, install names will be relative to this executable,
\totherwise there will be absolute.

update_links.py <binary_file> <path_to_binary_files>
\tThis syntax updates install names of one binary file.
'''

def update_links_of_file_with_libraries(binary, availableLibs, executable = None):
	'''
	Update all install_name of binary file path.
	Libraries are looked for in availableLibs.
	'''
	replacePath = None
	if (executable != None):
		replacePath = os.path.dirname(os.path.dirname(executable))

	libraries = binary_utilities.get_linked_libraries_of_file(binary)

	for lib in libraries:
                libName = binary_utilities.get_library_basename(lib)
                if availableLibs.has_key(libName):
                        newLib = availableLibs[libName]
                        if replacePath != None:
                                newLib = newLib.replace(replacePath, "@executable_path/..")
                        command = "install_name_tool -change %(lib)s %(newlib)s %(bin)s" \
                                % { "lib" : lib, "newlib" : newLib, "bin" : binary }
                        print command
                        os.popen(command)

def update_links_of_path(path, executable = None):
	'''
	Update all install_name of binary files in path.
	Libraries are looked for in path too.
	'''
	availableLibraries = binary_utilities.get_binary_files(path)
	for lib in availableLibraries:
		update_links_of_file_with_libraries(availableLibraries[lib], availableLibraries, executable)

def update_links_of_file_with_libpath(binary, path_to_libraries):
	'''
	Update all install_name of file 'binary' with files from 'path_to_libraries'.
	'''
	availableLibraries = binary_utilities.get_binary_files(path_to_libraries)
	update_links_of_file_with_libraries(binary, availableLibraries)

if __name__ == "__main__":
	if len(sys.argv) < 2:
		print USAGE
		sys.exit(1)

	if os.path.isdir(sys.argv[1]):
		print "updating links in %(path)s ..." % { "path" : sys.argv[1] }
		executable = None
		if len(sys.argv) == 3:
			executable = sys.argv[2]
		update_links_of_path(sys.argv[1], executable)
		print "done"
	elif os.path.isfile(sys.argv[1]):
		if len(sys.argv) < 3:
			print USAGE
			sys.exit(1)
		print "updating %(file)s with libraries from %(libpath)s ..." % { "file" : sys.argv[1], "libpath" : sys.argv[2] }
		update_links_of_file_with_libpath(sys.argv[1], sys.argv[2])
		print "done"
