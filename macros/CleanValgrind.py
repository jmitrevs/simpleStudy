#!/usr/bin/env python

#This removes the "blocks are still reachable" blocks

import getopt, sys, fileinput, re

def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "h", ["help"])
    except getopt.GetoptError, err:
        # print help information and exit:
        print str(err) # will print something like "option -a not recognized"
        usage()
        sys.exit(2)
    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        else:
            assert False, "unhandled option"

    # define regexps
    firstLine = re.compile(r'==\d+== [\d,]+ bytes in \d+ blocks are still reachable')
    secondLine = re.compile(r'==\d+==    at')
    followingLines = re.compile(r'==\d+==    by')

    sawfirst = False
    sawsecond = False

    for line in fileinput.input(args):
        if firstLine.match(line):
            sawfirst = True
        elif sawfirst and secondLine.match(line):
            sawsecond = True
            sawfirst = False
        elif sawsecond and followingLines.match(line):
            pass
        elif sawsecond:
            sawsecond = False
        else:
            print line,
        

def usage():
    print "Usage:", sys.argv[0], '[input file]'
    print "  The program removes the block still reachable statements from the valgrind output."
    print "  Reads from stdin if no input file given."

if __name__ == "__main__":
    main()
