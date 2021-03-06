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
 

add_definitions(-std=gnu++11 -pthread)

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
	PlayerFrame.cpp TextPanel.cpp File.cpp ResCheckedListCtrl.cpp)

 
# If we build for windows systems, we also include the resource file
#containing the manifest, icon and other resources
#if(WIN32)
#	 set(SRCS ${SRCS} minimal.rc)
#endif(WIN32)

set(DEBUG ON CACHE BOOL "Load debug symbols or not") 

if (${DEBUG} STREQUAL ON)
	message(STATUS "enabling debugging flag")
	add_definitions(-g)
endif(${DEBUG} STREQUAL ON)
 
# Here we define the executable minimal.exe or minimal on other systems
# the above paths and defines will be used in this build

add_executable(Player WIN32 ${SRCS})


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules")
#configure MediainfoLib
if (UNIX)
	find_package(MediaInfoLib 0.7.71 REQUIRED)
elseif (WIN32)
	set(USE_MEDIAINFO_DLL FALSE CACHE BOOL "If true loads uses dll, else tries to use static library")
	if (${USE_MEDIAINFO_DLL} STREQUAL ON)
		add_definitions(-DMEDIAINFO_DLL)
		message("using media info dll")
	endif(${USE_MEDIAINFO_DLL} STREQUAL ON)
 
	  set(MEDIAINFOLIB_DEFINITIONS -DMEDIAINFOLIB)
	  set(MEDIAINFOLIB_SOURCE CACHE PATH "Path to MediaInfoLib source") 
	  if (${USE_MEDIAINFO_DLL} STREQUAL ON)
		  set(MEDIAINFOLIB_DLL CACHE FILEPATH "Path to MediaInfo .dll, or .so file")
	  else (${USE_MEDIAINFO_DLL} STREQUAL ON)
		  set(MEDIAINFOLIB_LIB CACHE FILEPATH "Path to MediaInfo .lib or .a file")
	  endif(${USE_MEDIAINFO_DLL} STREQUAL ON)
	  set(ZENLIB_PATH CACHE PATH  "Path to Zenlib source")
	  if ((EXISTS ${MEDIAINFOLIB_SOURCE}) AND (EXISTS ${MEDIAINFOLIB_LIB} OR EXISTS ${MEDIAINFOLIB_DLL}))
	  	set(MEDIAINFOLIB_INCLUDE_DIRS "${MEDIAINFOLIB_SOURCE}/Source/")
	  	if (${USE_MEDIAINFO_DLL} STREQUAL ON)
			set(MEDIAINFOLIB_LIBRARIES ${MEDIAINFOLIB_DLL})
		else()
			set(MEDIAINFOLIB_LIBRARIES ${MEDIAINFOLIB_LIB})
		endif (${USE_MEDIAINFO_DLL} STREQUAL ON)
			
		set(MEDIAINFOLIB_FOUND TRUE)
		include_directories(${ZENLIB_PATH}/Source/)
	else()
		message(FATAL_ERROR "Paths to mediainfolib not set, library not found")
	endif()
endif(UNIX)
if (MEDIAINFOLIB_FOUND)
	#	add_definitions(-lmediainfo -lzen -lz -lpthread -DUNICODE)
	include_directories(${MEDIAINFOLIB_INCLUDE_DIRS})
	message("MediaInfo include dirs:" ${MEDIAINFOLIB_INCLUDE_DIRS})
	message("MediaInfo libs: " ${MEDIAINFOLIB_LIBRARIES})
	message("MediaInfo definitions: " ${MEDIAINFOLIB_DEFINITIONS})
	message("wxWidgets libraries: " ${wxWidgets_LIBRARIES})
	target_link_libraries(Player ${wxWidgets_LIBRARIES} ${MEDIAINFOLIB_LIBRARIES})
endif()

set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )
