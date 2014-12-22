cmake_minimum_required(VERSION 2.6.2)
 
#our project is called Player for now 
project(Player)

 
# Location where cmake first looks for cmake modules.
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")
 
##---------------------------------------------------
## Please set your wxWidgets configuration here
##--------------------------------------------------- 
 
# Here you can define what libraries of wxWidgets you need for your
# application. You can figure out what libraries you need here;
# http://www.wxwidgets.org/manuals/2.8/wx_librarieslist.html
# We need the Find package for wxWidgets to work
# NOTE: if you're using aui, include aui in this required components list.
 
# It was noticed that when using MinGW gcc it is essential that 'core' is mentioned before 'base'.
find_package(wxWidgets REQUIRED media core base)
 
##---------------------------------------------------
## Actual config file starts here
##--------------------------------------------------- 
 
# wxWidgets include (this will do all the magic to configure everything)
include( "${wxWidgets_USE_FILE}" )
 
# For convenience we define the sources as a variable. You can add 
# header files and cpp/c files and CMake will sort them out
set(SRCS PlayerMain.cpp checkedlistctrl.cpp FileManager.cpp 
	PlayerFrame.cpp)

add_definitions(-std=gnu++11 -pthread)
 
# If we build for windows systems, we also include the resource file
#containing the manifest, icon and other resources
#if(WIN32)
#	 set(SRCS ${SRCS} minimal.rc)
#endif(WIN32)

if(WIN32)
	add_definitions(-static)
endif(WIN32)
	
set(DEBUG ON CACHE BOOL "Load debug symbols or not") 

if (${DEBUG} STREQUAL ON)
	message(STATUS "enabling debugging flag")
	add_definitions(-g)
endif(${DEBUG} STREQUAL ON)
 
# Here we define the executable minimal.exe or minimal on other systems
# the above paths and defines will be used in this build

add_executable(Player WIN32 ${SRCS})
 
# We add to our target 'minimal' the wxWidgets libraries. These are
# set for us by the find script. If you need other libraries, you
# can add them here as well. 
target_link_libraries(Player ${wxWidgets_LIBRARIES})

set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )
