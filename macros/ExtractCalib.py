#!/usr/bin/env python

#This script extracts the 7-constant calibration constants from the OPC file

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
    corr = re.compile(r'Envr_CANbus_\d+\.Envr_ELMB_[A-Fa-f\d]+\.ELMB_Channel_\d+_Corr')
    corrNeg = re.compile(
        r'Envr_CANbus_\d+\.Envr_ELMB_[A-Fa-f\d]+\.ELMB_Channel_\d+_Corr = ' +
        r'Envr_CANbus_\d+\.Envr_ELMB_[A-Fa-f\d]+\.ELMB_Channel_\d+' +
        r'\+\((?P<Const5>[.\d]+)\*Envr_CANbus_\d+.Envr_ELMB_[A-Fa-f\d]+\.ELMB_Channel_\d+(?P<Const4>[+-][.\d]+)\)\*\(' +
        r'(?P<Const3>[.\d]+)')

    corrPos = re.compile(
        r'Envr_CANbus_\d+\.Envr_ELMB_[A-Fa-f\d]+\.ELMB_Channel_\d+_Corr = ' +
        r'Envr_CANbus_\d+\.Envr_ELMB_[A-Fa-f\d]+\.ELMB_Channel_\d+' +
        r'\+\((?P<Const4>[.\d]+)(?P<Const5>[+-][.\d]+)\*Envr_CANbus_\d+\.Envr_ELMB_[A-Fa-f\d]+.ELMB_Channel_\d+\)\*\(' +
        r'(?P<Const3>[.\d]+)')

    combPosPos = re.compile(
        r'Envr_CANbus_\d+\.Envr_ELMB_[A-Fa-f\d]+\.ELMB_Channel_\d+ = (?P<Const1>[.\d]+)' +
        r'\*\(Envr_CANbus_\d+\.Envr_ELMB_[A-Fa-f\d]+\.ELMB_Channel_\d+_Corr-BOn\*(?P<Const6>[.\d]+)\)/' +
        r'\(\(Envr_CANbus_\d+\.Envr_ELMB_[A-Fa-f\d]+\.ELMB_Channel_\d+_Corr-BOn\*[.\d]+\)' +
        r'\+\(Envr_CANbus_\d+\.Envr_ELMB_[A-Fa-f\d]+\.ELMB_Channel_\d+_Corr-BOn\*(?P<Const7>[.\d]+)\)\)' +
        r'(?P<Const2>[+-][.\d]+)')

    combNegPos = re.compile(
        r'Envr_CANbus_\d+\.Envr_ELMB_[A-Fa-f\d]+\.ELMB_Channel_\d+ = (?P<Const1>[.\d]+)' +
        r'\*\(Envr_CANbus_\d+\.Envr_ELMB_[A-Fa-f\d]+\.ELMB_Channel_\d+_Corr-BOn\*\(0(?P<Const6>-[.\d]+)\)\)/' +
        r'\(\(Envr_CANbus_\d+\.Envr_ELMB_[A-Fa-f\d]+\.ELMB_Channel_\d+_Corr-BOn\*\(0-[.\d]+\)\)' +
        r'\+\(Envr_CANbus_\d+\.Envr_ELMB_[A-Fa-f\d]+\.ELMB_Channel_\d+_Corr-BOn\*(?P<Const7>[.\d]+)\)\)' +
        r'(?P<Const2>[+-][.\d]+)')
 
    combPosNeg = re.compile(
        r'Envr_CANbus_\d+\.Envr_ELMB_[A-Fa-f\d]+\.ELMB_Channel_\d+ = (?P<Const1>[.\d]+)' +
        r'\*\(Envr_CANbus_\d+\.Envr_ELMB_[A-Fa-f\d]+\.ELMB_Channel_\d+_Corr-BOn\*(?P<Const6>[.\d]+)\)/' +
        r'\(\(Envr_CANbus_\d+\.Envr_ELMB_[A-Fa-f\d]+\.ELMB_Channel_\d+_Corr-BOn\*[.\d]+\)' +
        r'\+\(Envr_CANbus_\d+\.Envr_ELMB_[A-Fa-f\d]+\.ELMB_Channel_\d+_Corr-BOn\*\(0(?P<Const7>-[.\d]+)\)\)\)' +
        r'(?P<Const2>[+-][.\d]+)')
       
    combNegNeg = re.compile(
        r'Envr_CANbus_\d+\.Envr_ELMB_[A-Fa-f\d]+\.ELMB_Channel_\d+ = (?P<Const1>[.\d]+)' +
        r'\*\(Envr_CANbus_\d+\.Envr_ELMB_[A-Fa-f\d]+\.ELMB_Channel_\d+_Corr-BOn\*\(0(?P<Const6>-[.\d]+)\)\)/' +
        r'\(\(Envr_CANbus_\d+\.Envr_ELMB_[A-Fa-f\d]+\.ELMB_Channel_\d+_Corr-BOn\*\(0-[.\d]+\)\)' +
        r'\+\(Envr_CANbus_\d+\.Envr_ELMB_[A-Fa-f\d]+\.ELMB_Channel_\d+_Corr-BOn\*\(0(?P<Const7>-[.\d]+)\)\)\)' +
        r'(?P<Const2>[+-][.\d]+)')

    line1 = ''
    line2 = ''
    for line in fileinput.input(args):
        if corr.match(line1) and corr.match(line2):
            print line,
            negMatch1 = corrNeg.match(line1)
            negMatch2 = corrNeg.match(line2)
            posMatch1 = corrPos.match(line1)
            posMatch2 = corrPos.match(line2)
            pospos = combPosPos.match(line)
            negpos = combNegPos.match(line)
            posneg = combPosNeg.match(line)
            negneg = combNegNeg.match(line)
            if negMatch1 and negMatch2:
                match = negMatch1
            elif posMatch1 and posMatch2:
                match = posMatch1
            else:
                sys.stderr.write('Was unable to extract the calibration constants from first two lines')

            if pospos:
                fin = pospos
            elif negpos:
                fin = negpos
            elif posneg:
                fin = posneg
            elif negneg:
                fin = negneg
            else:
                sys.stderr.write('Was unable to extract the calibration constants of last line')


            print 'Const1 = ', fin.group('Const1')
            print 'Const2 = ', fin.group('Const2')
            print 'Const3 = ', match.group('Const3')
            print 'Const4 = ', match.group('Const4')
            print 'Const5 = ', match.group('Const5')
            print 'Const6 = ', fin.group('Const6')
            print 'Const7 = ', fin.group('Const7')
            print
                
        line2 = line1
        line1 = line
        

def usage():
    print "Usage:", sys.argv[0], '[input file]'
    print "  The program extracts the 7-constant calibration constants from"
    print "  an OPC config file. Reads from stdin if no input file given."

if __name__ == "__main__":
    main()
