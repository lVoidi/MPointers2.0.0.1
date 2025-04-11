# CMake generated Testfile for 
# Source directory: C:/Users/roarb/Documents/Projects/mpointers
# Build directory: C:/Users/roarb/Documents/Projects/mpointers/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test([=[MPointerBasicTest]=] "C:/Users/roarb/Documents/Projects/mpointers/build/Debug/mpointer_test.exe" "localhost" "9090")
  set_tests_properties([=[MPointerBasicTest]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/roarb/Documents/Projects/mpointers/CMakeLists.txt;86;add_test;C:/Users/roarb/Documents/Projects/mpointers/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test([=[MPointerBasicTest]=] "C:/Users/roarb/Documents/Projects/mpointers/build/Release/mpointer_test.exe" "localhost" "9090")
  set_tests_properties([=[MPointerBasicTest]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/roarb/Documents/Projects/mpointers/CMakeLists.txt;86;add_test;C:/Users/roarb/Documents/Projects/mpointers/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test([=[MPointerBasicTest]=] "C:/Users/roarb/Documents/Projects/mpointers/build/MinSizeRel/mpointer_test.exe" "localhost" "9090")
  set_tests_properties([=[MPointerBasicTest]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/roarb/Documents/Projects/mpointers/CMakeLists.txt;86;add_test;C:/Users/roarb/Documents/Projects/mpointers/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test([=[MPointerBasicTest]=] "C:/Users/roarb/Documents/Projects/mpointers/build/RelWithDebInfo/mpointer_test.exe" "localhost" "9090")
  set_tests_properties([=[MPointerBasicTest]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/roarb/Documents/Projects/mpointers/CMakeLists.txt;86;add_test;C:/Users/roarb/Documents/Projects/mpointers/CMakeLists.txt;0;")
else()
  add_test([=[MPointerBasicTest]=] NOT_AVAILABLE)
endif()
