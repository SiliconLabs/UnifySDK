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

    parser.add_argument('src',metavar='src')
    parser.add_argument('-c',action='store_true')
    parser.add_argument('-I',action='append')
    parser.add_argument('-D',action='append')
    parser.add_argument('-o')
    parser.add_argument('-E')
    parser.add_argument('-S',action='store_true')
    parser.add_argument('--listfile', action='store_true')

    parser.add_argument('-WL')

    args = parser.parse_args()

    c51 = "C51.EXE"
    if(args.E):
        c51=args.E+ os.path.sep + c51
    elif("KEILPATH" in os.environ ):
        c51=os.environ['KEILPATH'] + os.path.sep +"BIN" + os.path.sep + c51
    
    if(platform.system() != "Windows"):
        c51="LC_CTYPE= WINEDEBUG=-all wine " + c51
    

    k= args.src+"\r\n"
    if args.o:
        k=k+"OBJECT("+args.o+")\r\n"

    if(args.S):
        k=k+"SRC("+args.o+".SRC)\r\n"

    if args.WL:
        k=k+"WARNINGLEVEL("+args.WL+")\r\n"

    if args.I:
        k=k+"INCDIR("+ ";".join(args.I)+")\r\n"

    if args.D:
        k=k+"DEFINE("+ ",".join(args.D)+")\r\n"

    if args.listfile:
        k = k + "PR(" + args.o + ".lst) " + "\n"

    # A hack these should be passed in from CMAKE
    # App:
    # standard_zwave_c_flags = 'DEBUG OBJECTADVANCED OBJECTEXTEND LARGE CODE LISTINCLUDE  NOINTPROMOTE NOCOND OPTIMIZE(11,SIZE) SYMBOLS'
    # Lib:
    # Omitted params: RF ($(TARGET).reg) IV($(INT_VEC)) ID($(C51INC))
    standard_zwave_c_flags = 'OA OE LA CD SB LC NOCO NOAM  OT(11,SIZE)  CODE SYMBOLS'
    k = k + standard_zwave_c_flags + "\r\n"
    # Hack for assembling a51 files without teaching CMake about ax51
    if args.src.endswith(".a51"):
        ax51 = "AX51.EXE"
        if(args.E):
            ax51 = args.E + os.path.sep + ax51
        elif("KEILPATH" in os.environ):
            ax51 = os.environ['KEILPATH'] + os.path.sep + "BIN" + os.path.sep + ax51

        if(platform.system() != "Windows"):
            ax51 = "LC_CTYPE= WINEDEBUG=-all wine " + ax51
        # Note: AX51 chokes on line endings of type 0D 0D 0A. Avoid using "\r\n"
        # (at least on windows)
        k = args.src + "\n"
        if args.I:
            k = k + "INCDIR(" + ";".join(args.I) + ")\n"
        k = k + "NORB XR CASE" + "\n"
        k = k + "OJ(" + args.o + ")" + "\n"
        k = k + "PR(" + args.o + ".lst) " + "\n"
        # std_zwave_asm_flags = "NOREGISTERBANK XREF"
        std_zwave_asm_flags = "NORB XR CASE"
        k = k + std_zwave_asm_flags

        cmdfile_name = 'cmdfile' + str(random.randint(1, int(1e8)))
        f = open(cmdfile_name, 'w')
        f.write(k)
        f.write("\n")
        f.close()

        proc = subprocess.Popen(ax51 + " @" + cmdfile_name , stdout=subprocess.PIPE, shell=True)
        (out, err) = proc.communicate()
        print("AX51 output:", out)
        for m in re.finditer("^\*\*\* (\w+) (C\d+) IN LINE (\d+) OF (.*?)(?=\:\ )(.*)", out, re.MULTILINE):
            err_wrn, code, line, file, msg = m.groups()
            print("%s:%s:0: %s" % (file, line, err_wrn + " " + code + " " + msg.replace(':', '')))
        rc = proc.returncode;

        os.remove(cmdfile_name)

        sys.exit(rc)
    else:
        cmdfile_name = 'cmdfile' + str(random.randint(1, int(1e8)))
        f = open(cmdfile_name, 'w')
        f.write(k)
        f.write("\n")
        f.close()


        proc = subprocess.Popen(c51 + " @" + cmdfile_name , stdout=subprocess.PIPE, shell=True)
        (out, err) = proc.communicate()

#        print "program output:", out
        for m in re.finditer("^\*\*\* (\w+) (C\d+) IN LINE (\d+) OF (.*?)(?=\:\ )(.*)", out, re.MULTILINE):
            err_wrn, code, line, file, msg = m.groups()
            print("%s:%s:0: %s" % (file, line, err_wrn + " " + code + " " + msg.replace(':', '')))
        rc = proc.returncode;

        os.remove(cmdfile_name)

        sys.exit(rc)
