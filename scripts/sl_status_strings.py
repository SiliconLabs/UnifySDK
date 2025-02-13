#!/usr/bin/env python3

import os
import sys
from io import open
from string import Template

test_runner_template = '''
#include <sl_status.h>

const char* sl_status_string(int status) {
  switch (status) {
${SL_STATUS_AS_STRINGS}
    default: return "UNKNOWN";
  }
}

const char* sl_status_string_verbose(int status) {
  switch (status) {
${SL_STATUS_AS_STRINGS_VERBOSE}
    default: return "Unknown error.";
  }
}
'''

if __name__ == '__main__':
    include_folder = "../include"

    if len(sys.argv) == 2:
        include_folder = sys.argv[1]

    sl_status_as_strings = ""
    sl_status_as_strings_verbose = ""
    file_path = f'{include_folder}/sl_status.h'

    if not os.path.exists(file_path):
        sys.exit(f'File not found: {file_path}')

    for line in open(file_path):
        if "#define" and "SL_STATUS" in line and not "SL_STATUS_H" in line:
            status = line.split()[1] # in sl_status.h the second column in #define lines is always the SL_STATUS_NAME
            message = line.partition("///< ")[2].strip('\n') # line.partition splits the line in 3 parts (before, delimiter, after)
            sl_status_as_strings += f'    case {status}: return "{status}";\n'
            sl_status_as_strings_verbose += f'    case {status}: return "{message}";\n'

    t = Template(test_runner_template).substitute({
        'SL_STATUS_AS_STRINGS': sl_status_as_strings,
        'SL_STATUS_AS_STRINGS_VERBOSE': sl_status_as_strings_verbose
    })

    print(t)
