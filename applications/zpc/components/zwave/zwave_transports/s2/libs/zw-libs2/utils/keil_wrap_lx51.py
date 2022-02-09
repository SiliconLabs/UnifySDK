#!/usr/bin/python
# Copyright (c) 2018 Silicon Laboratories Inc.

import argparse
import os
import subprocess
import sys
import re
import platform
import random

if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    parser.add_argument('obj',metavar='obj',nargs='+')
    parser.add_argument('-o')
    parser.add_argument('-E')

    args = parser.parse_args()

    c51 = "LX51.EXE"
    ohx51 ="OHX51.EXE"
    if(args.E):
        c51=args.E+ os.path.sep + c51
        ohx51=args.E+ os.path.sep + ohx51
    elif("KEILPATH" in os.environ ):
        c51=os.environ['KEILPATH'] + os.path.sep +"BIN" + os.path.sep + c51
        ohx51=os.environ['KEILPATH'] + os.path.sep +"BIN" + os.path.sep + ohx51
        
    
    if(platform.system() != "Windows"):
        c51="LC_CTYPE= WINEDEBUG=-all wine " + c51
        ohx51="LC_CTYPE= WINEDEBUG=-all wine " + ohx51


    cmdfile_name = 'cmdfile' + str(random.randint(1, int(1e8)))
    f = open(cmdfile_name, "w")
    f.write( ", ".join(args.obj)+ " TO " + args.o+"\n" )
    f.write( " NOOVERLAY ")

    f.close()
    
    proc = subprocess.Popen(c51 + " @" + cmdfile_name , stdout=subprocess.PIPE, shell=True)
    (out, err) = proc.communicate()

    rc = proc.returncode;
    os.remove(cmdfile_name)
    if(rc<=1):
        if(platform.system() != "Windows"):
            os.system(ohx51+" %s HEXFILE\(%s.hex\)" % (args.o,args.o ) )
        else:
            os.system(ohx51+" %s HEXFILE(%s.hex)" % (args.o,args.o ) )
        sys.exit(0)
    else:
        print(out)
        print(err)
        sys.exit(rc)
