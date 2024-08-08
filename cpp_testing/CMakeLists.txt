cmake_minimum_required(VERSION 3.10)
project(MyProject)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)

# Find Python3 and NumPy
find_package(Python3 COMPONENTS Development REQUIRED)

# Manually set NumPy include directory
set(Python3_NumPy_INCLUDE_DIR "/usr/local/Cellar/numpy/2.0.1/lib/python3.12/site-packages/numpy/_core/include")

# Include directories
include_directories(${Python3_INCLUDE_DIRS})
include_directories(${Python3_NumPy_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/matplotlib-cpp) # Include matplotlib-cpp header

# Add library
add_library(my_library STATIC src/my_library.cpp)

# Add executable
add_executable(my_executable src/main.cpp)

# Link the Python libraries
target_link_libraries(my_executable PRIVATE ${Python3_LIBRARIES})
target_link_libraries(my_executable PRIVATE my_library)