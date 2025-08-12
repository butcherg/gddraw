cmake_minimum_required(VERSION 3.19)
project(gddraw VERSION 1.0)

#set(CMAKE_CXX_STANDARD 20)


string(TIMESTAMP BUILDDATE)
add_compile_options(-ggdb -Wno-deprecated-enum-enum-conversion -DVERSION="${CMAKE_PROJECT_VERSION}"  -DBUILDDATE="${BUILDDATE}")

add_executable(gddraw src/gddraw.cpp )

find_package(PkgConfig REQUIRED)

#opencv:
pkg_check_modules(GD REQUIRED IMPORTED_TARGET gdlib)
if(GD_FOUND)
	include_directories( ${GD_INCLUDE_DIRS} )
	target_link_libraries( gddraw PkgConfig::GD )
else()
	message(FATAL_ERROR "libgd not found")
endif(GD_FOUND)

if(WIN32)
#	target_link_libraries(contours wsock32 ws2_32 psapi)
endif(WIN32)

add_subdirectory(src)

