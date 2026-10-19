# CMake generated Testfile for 
# Source directory: C:/Users/Dell/Desktop/System_Projects/Latency-Aware_Backtester/tests
# Build directory: C:/Users/Dell/Desktop/System_Projects/Latency-Aware_Backtester/build/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(BacktesterTests "C:/Users/Dell/Desktop/System_Projects/Latency-Aware_Backtester/build/tests/Debug/tests.exe")
  set_tests_properties(BacktesterTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/Dell/Desktop/System_Projects/Latency-Aware_Backtester/tests/CMakeLists.txt;11;add_test;C:/Users/Dell/Desktop/System_Projects/Latency-Aware_Backtester/tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(BacktesterTests "C:/Users/Dell/Desktop/System_Projects/Latency-Aware_Backtester/build/tests/Release/tests.exe")
  set_tests_properties(BacktesterTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/Dell/Desktop/System_Projects/Latency-Aware_Backtester/tests/CMakeLists.txt;11;add_test;C:/Users/Dell/Desktop/System_Projects/Latency-Aware_Backtester/tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(BacktesterTests "C:/Users/Dell/Desktop/System_Projects/Latency-Aware_Backtester/build/tests/MinSizeRel/tests.exe")
  set_tests_properties(BacktesterTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/Dell/Desktop/System_Projects/Latency-Aware_Backtester/tests/CMakeLists.txt;11;add_test;C:/Users/Dell/Desktop/System_Projects/Latency-Aware_Backtester/tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(BacktesterTests "C:/Users/Dell/Desktop/System_Projects/Latency-Aware_Backtester/build/tests/RelWithDebInfo/tests.exe")
  set_tests_properties(BacktesterTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/Dell/Desktop/System_Projects/Latency-Aware_Backtester/tests/CMakeLists.txt;11;add_test;C:/Users/Dell/Desktop/System_Projects/Latency-Aware_Backtester/tests/CMakeLists.txt;0;")
else()
  add_test(BacktesterTests NOT_AVAILABLE)
endif()
subdirs("../_deps/catch2-build")
