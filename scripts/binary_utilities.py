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
# Functions used by several scripts for SDK installation.
# These functions works on Mac OS X and Linux only.
import commands, os, platform, re, sys

# This is a list of libs considered to be already installed on all machines,
# and thus should not be included in the package.
LINUX_LIB_LIST = [
	"ICE",
	"SM",
	"X11",
	"Xau",
	"Xdmcp",
	"Xext",
	"Xfixes",
	"Xi",
	"Xinerama",
	"Xrandr",
	"Xrender",
	"Xt",
	"Xcursor",
	"c",
	"dl",
	"gcc_s",
	"m",
	"nsl",
	"pthread",
	"resolv",
	"rt",
	"stdc++",
	"z",
	"glib-2.0",
	"gmodule-2.0",
	"gobject-2.0",
	"gthread-2.0",
	"crypto",
	"ssl",
]

def is_system_library(path):
	'''
	Return True if the given library is available by default on all system.
	It also return True on Mac OS X if the library is a Framework.
	'''
	result = False
	if platform.system() == "Linux":
		basename = get_library_basename(path)
		basename = re.sub(r"lib([^ ]+)\.so.*", r"\1", basename)
		result = basename in LINUX_LIB_LIST
	elif platform.system() == "Darwin":
		# VOXOX CHANGE -ASV- 09-01-2009: we should not exclude "/usr/local"
		result = ((path.startswith('/usr') and not path.startswith("/usr/local")) 
			or path.startswith('/System')
			or '.framework' in path)
	else:
		print "!! Platform not supported"
		sys.exit(1)
	return result

def __get_linked_libraries_of_file_for_macosx(path):
        '''
	Get the list of linked libraries (result of otool -L 'path') of 'path'
	'''
	# Get the amount of line to pass. If calling otool -L on a .dylib file,
	# two lines must be passed (1st displays the file name, 2nd displays the lib. id)
	# If on a .so, only one line must be passed (no lib. id displayed).
	passCount = 1
	if path.endswith(".dylib"):
		passCount = 2

	pipe = os.popen("otool -L %s" % path)
	passedCounter = 0
	result = []
	for line in pipe:
		if passedCounter < passCount:
			passedCounter += 1
			continue
		result += [re.sub(r"^[ \t]*(.*) \(.*\)$", r"\1", line).strip()]
	pipe.close()
	return result

def __get_linked_libraries_of_file_for_linux(path):
	'''
	Gets the list of linked libraries of a binary file (result of ldd 'path').
	Code taken from one written by Aurélien Gâteau.
	'''
	result = []

	status, output = commands.getstatusoutput("ldd " + path)

	if status != 0:
		raise Exception("No such file: " + path)

	rx = re.compile("lib[-+_a-zA-Z0-9.]+\.so.* => ([^ ]+)")

	for line in output.split("\n"):
		match = rx.search(line)
		if not match:
			continue
		libPath = match.group(1)

		result += [libPath]

	return result

def get_linked_libraries_of_file(path):
	'''
	Get the list of linked libraries of 'path'. Works on Mac OS X and Linux
	'''
	result = ""
	if platform.system() == "Linux":
		result = __get_linked_libraries_of_file_for_linux(path)
	elif platform.system() == "Darwin":
		result = __get_linked_libraries_of_file_for_macosx(path)
	else:
		print "!! Platform not supported"
		sys.exit(1)
	return result

def get_library_basename(libname):
	'''
	Compute the basename of 'libname'.
	e.g: if libname = /usr/lib/libc.so, get_library_basename returns libc.so
	e.g: if libname = /Library/Frameworks/QtCore.framework/Versions/4/QtCore,
		get_library_basename returns QtCore.framework/Versions/4/QtCore
	'''
	# Classic libname (first example)
	result = os.path.basename(libname)

	# Looking for a framework lib (second example)
	rexp = re.compile(r'(?:[^/]*/)*([^/]*)\.framework/Versions/([^/]*)/([^/]*)')
	mObj = rexp.match(libname)
	if mObj != None and mObj.lastindex != None:
		if mObj.group(1) == mObj.group(3):
			result = mObj.group(1) + ".framework/Versions/" + mObj.group(2) + "/" + mObj.group(3)

	return result

def is_binary_file(path):
	'''
	Tests if path is a binary (Mach-O) file.
	@return True if so
	'''
	pipe  = os.popen("file '" + path + "'", "r")
	result = False
	for line in pipe:
		# Mach-O: Mac OS X shared library and executable
		# ELF: Linux shared library and executable
		if line.find("Mach-O")  != -1 \
			or line.find("ELF") != -1:
			result = True
	pipe.close()
	return result

def is_universal_binary_file(path):
	'''
	Tests if path is a Univeral Binary (valid only on Mac OS X).
	@return True if UB
	'''
	pipe  = os.popen("file '" + path + "'", "r")
	result = False
	for line in pipe:
		# Mach-O: Mac OS X shared library and executable
		# ELF: Linux shared library and executable
		if line.find("Mach-O") != -1 and line.find("universal") != -1:
			result = True
	pipe.close()
	return result

def get_binary_files(path):
	'''
	@return a dictionary of binary files available in path with the
	following scheme: "<basename>:<fullpath>".
	'''
	result = {}
	for root, dirs, files in os.walk(path):
		for f in files:
			myFile = os.path.join(root, f)
			if is_binary_file(myFile):
				result[get_library_basename(myFile)] = myFile
	return result

if __name__ == "__main__":
	get_binary_files(sys.argv[1])

