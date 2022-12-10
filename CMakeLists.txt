# CMakeList.txt : CMake project for MedialooksTestProject, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("MedialooksTestProject")

# Add source to this project's executable.
add_executable (MedialooksTestProject "main.cpp" "ISplitter.h" "Splitter.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET MedialooksTestProject PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
