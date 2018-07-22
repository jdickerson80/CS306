cmake_minimum_required( VERSION 2.8 )

# Name the project
project( rempsd )

# Set the active build type
set( CMAKE_BUILD_TYPE Release )
#set( CMAKE_BUILD_TYPE Debug )

# Add compiler flags
set( CMAKE_C_FLAGS "-Wall -std=gnu99" )

# Add all of the files to the build
file( GLOB_RECURSE Headers "*.h" )
file( GLOB_RECURSE Sources "*.c" )

# Add the executable output
add_executable( ${PROJECT_NAME} ${Headers} ${Sources} )
