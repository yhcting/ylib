from __future__ import print_function
import sys
import os
import re


def mktokrepl(tokt):
    def tokrepl(mobj):
        repltok = "%s%s%s" % (mobj.group(1), tokt, mobj.group(2))
        #print("From(%s) to (%s)\n" % (mobj.group(0), repltok))
        return repltok
    return tokrepl


def main():
    if len(sys.argv) < 4:
        print("Usage: %s <tok-from> <tok-to> <file> [<file> ...]\n"
              % sys.argv[0], file=sys.stderr)
        sys.exit(1)
    tokf = sys.argv[1]
    tokt = sys.argv[2]
    pat = re.compile(r'(^|\W)%s(\W|$)' % re.escape(tokf), re.M | re.S)
    for fname in sys.argv[3:]:
        with open(fname, 'rb') as f:
            data = f.read()
        newdata = re.sub(pat, mktokrepl(tokt), data)
        backupfilename = fname + '.orig'
        if os.path.exists(backupfilename):
            print("Warning: Fail to create backup file %s (Already exists)." %
                    backupfilename, file=sys.stderr)
        else:
            os.rename(fname, backupfilename)
        with open(fname, 'wb') as f:
            f.write(newdata)

if '__main__' == __name__:
    main()