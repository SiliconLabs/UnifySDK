if(NOT DEFINED LLVMPATH)
  set(LLVMPATH /usr/local/opt/llvm)
  message(NOTICE "Using LLVMPATH=${LLVMPATH}. Override with: -DLLVMPATH=<yourpath>")
endif(NOT DEFINED LLVMPATH)

set(CMAKE_C_COMPILER ${LLVMPATH}/bin/clang)
set(CMAKE_CXX_COMPILER ${LLVMPATH}/bin/clang++)
