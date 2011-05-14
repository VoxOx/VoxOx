#!/usr/bin/env python
import fileinput
import re
import sys
from optparse import OptionParser

# For now this is quite rudimentary: we just remove all attributes of the body
# tag. We may need to be less "violent".
bodyRe = re.compile("&lt;body [^>]*>")

def fixFonts(line):
    return bodyRe.sub("&lt;body>", line)

usage = \
"""
%prog [options] file.ui

Removes global font settings from all text of a .ui file. This is necessary
because if you ever set some text formatting in a QLabel for example, Qt
Designer will automatically define the font-family and font-size for the whole
widget. Those hardcoded family and sizes often won't look good on other
platforms.
"""

def main():
    parser = OptionParser(usage=usage.strip())

    parser.add_option("-i", "--inplace",
                      action="store_true", dest="inplace", default=False,
                      help="Modify file passed on argument, don't print to stdout")

    (options, args) = parser.parse_args()
    if len(args) != 1:
        parser.error("incorrect number of arguments")

    if options.inplace:
        inplace = 1
    else:
        inplace = 0

    for line in fileinput.input(args[0], inplace):
        sys.stdout.write( fixFonts(line) )

    return 0


if __name__=="__main__":
    sys.exit(main())
# vi: ts=4 sw=4 et
