set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost 1.38.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()
