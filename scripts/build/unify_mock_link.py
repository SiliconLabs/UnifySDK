#!/usr/bin/python3
# This script is designed to remove library implementations if a mock library of the
# same name is present 
import sys
import os

libs = sys.argv[1:]

mocks=[]
for l in libs:
  if( "_mock." in l ):
    mocks.append(l)

filter_libs = []
for l in libs:
  mock_match = None
  for m in mocks:
    basename = os.path.basename(m).split('.')[0]
    if( ((basename[:-5]+".") in l) ):
      mock_match = m
      break

  if(mock_match):
    filter_libs.append(mock_match)
  else:
    filter_libs.append(l)

print( "\n".join(filter_libs) )


