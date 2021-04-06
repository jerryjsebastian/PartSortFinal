cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
# Create Project
project(deinSchrankFinal)
add_executable(deinSchrankFinal src/FinalApplication.cpp src/objectDetection2D.cpp src/acquisition.cpp src/sampleclient.cpp)

# Find Packages
# Find OpenCV
find_package(OpenCV 4.1 REQUIRED)

if( OpenCV_FOUND )
  # [C/C++]>[General]>[Additional Include Directories]
  include_directories(${OpenCV_INCLUDE_DIRS})

  # [C/C++]>[Preprocessor]>[Preprocessor Definitions]
  add_definitions(${OpenCV_DEFINITIONS})

  # [Linker]>[General]>[Additional Library Directories]
  link_directories(${OpenCV_LIBRARY_DIR})

  # [Linker]>[Input]>[Additional Dependencies]
  target_link_libraries(deinSchrankFinal ${OpenCV_LIBRARIES})
endif()