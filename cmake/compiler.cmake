if(CMAKE_COMPILER_IS_GNUCXX)
  execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
  message(STATUS "Using GNU C++ compiler, version ${GCC_VERSION}.")
  if(GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7)
    message(STATUS "Your compiler supports C++11 standard.")
    add_definitions("-std=gnu++11 -fPIC")
  elseif(GCC_VERSION VERSION_GREATER 4.3 OR GCC_VERSION VERSION_EQUAL 4.3)
    message(WARNING "Your compiler version is known to support C++11 standard only partially.
                     If you'd get any compilation errors, consider upgrading to a compiler version
                     which fully supports C++11 standard.")
    add_definitions("-std=gnu++0x")
  else()
    message(FATAL_ERROR "Your compiler version does not support C++11 standard.
                         You need a GNU C++ compiler with version higher than 4.3.")
  endif()
  add_definitions("-Wno-uninitialized")
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  execute_process(COMMAND ${CMAKE_C_COMPILER} --version OUTPUT_VARIABLE CLANG_VERSION)
  message(STATUS "Using LLVM/Clang C++ compiler, version info: ${CLANG_VERSION}")
  if(NOT ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 3.1)
    message(STATUS "Your compiler supports C++11 standard.")
  else()
    message(WARNING "Your compiler may not support all the necessary C++11 standard features
                     to build this application. If you'd get any compilation errors, consider
                     upgrading to a compiler version which fully supports the C++11 standard.")
  endif()
  add_definitions("-std=c++11 -Wno-uninitialized -Wno-null-conversion -Wno-format -Wno-deprecated")
  add_definitions("-Wmismatched-tags -fPIC")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS_INIT} $ENV{LDFLAGS} -fsanitize=undefined")
  find_library(LIBCPP NAMES libc++.so libc++.so.1.0 libc++.dylib OPTIONAL)
  if(LIBCPP)
    message(STATUS "Using native Clang's C++ standard library: ${LIBCPP}")
    add_definitions("-stdlib=libc++ -DHAVELIBCPP")
  endif()
else()
  message(WARNING "Your C++ compiler is not officially supported for building of this application.
                   If you'd get any compilation errors, consider upgrading to a compiler version
                   which fully supports the C++11 standard.")
endif(CMAKE_COMPILER_IS_GNUCXX)
