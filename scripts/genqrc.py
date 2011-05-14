#!/usr/bin/env python
# vim: noexpandtab

# This script generates a .qrc file for all images. The .qrc file is not used
# by the build system, but it's useful to be able to view images in Designer.
import os
import sys
import time


EXTENSIONS=[".png", ".mng"]


def writeComment(out):
	dateString = time.asctime()
	out.write("<!--\n")
	out.write("This file has been generated on %s by genqrc.py\n" % dateString)
	out.write("Command line used:\n")
	cmdLine = " ".join(sys.argv)
	out.write(cmdLine + "\n")
	out.write("-->\n")


def writeContent(out, dir):
	out.write("<RCC>\n<qresource prefix='/'>\n")

	lst = []
	for root, dirs, fileNames in os.walk(dir):
		for fileName in fileNames:
			ext = os.path.splitext(fileName)[1]
			if not ext in EXTENSIONS:
				continue
			path = os.path.join(root, fileName)
			lst.append(path)

	# Sort the list so that generating it on different platforms does not
	# result in different orders (minimize diff size)
	lst.sort()

	for path in lst:
		out.write("<file>%s</file>\n" % path)

	out.write("</qresource>\n</RCC>\n")


def usage():
	print """
Usage: genqrc.py path/to/resources/dir

Output to stdout a .qrc file for all files with these extensions: %s
""" % str(EXTENSIONS)


def main(args):
	if len(args) != 2:
		usage()
		return 1

	dir = args[1]

	out = sys.stdout
	writeComment(out)
	writeContent(out, dir)

	return 0


if __name__ == "__main__":
	sys.exit(main(sys.argv))
