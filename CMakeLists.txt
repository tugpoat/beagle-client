cmake_minimum_required(VERSION 3.10)

project(beagle-client)

find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME}
	main.cpp
	src/beagle_client.cpp
	src/yacardemu_client.cpp
	src/base64.cpp
	src/utility.cpp
)

if (WIN32)
else()
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(NFC REQUIRED libnfc)
	#pkg_check_modules(LIBARCHIVE REQUIRED libarchive)
endif()

include_directories(
	${THREAD_INLCUDE_DIRS}
	${PROJECT_SOURCE_DIR}/inc
	${PROJECT_SOURCE_DIR}/lib/cpp-httplib
	${PROJECT_SOURCE_DIR}/lib/mINI/src
	${PROJECT_SOURCE_DIR}/lib/filesystem/include
	${PROJECT_SOURCE_DIR}/lib/libarchive_cpp_wrapper
)

#if (ARM64)
	#set( libarchive_LIBRARIES "/usr/aarch64-unknown-linux-gnu/usr/lib/libarchive.so" )
  	#set( libarchive_INCLUDE_DIRS "" )
#else()

#endif()

add_subdirectory(lib/spdlog)
add_subdirectory(lib/libarchive_cpp_wrapper)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
	include(CheckIPOSupported)
	check_ipo_supported(RESULT supported OUTPUT error)
	if (supported)
		set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
	endif()
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)

if (MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE /W4)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
else()
	target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(${PROJECT_NAME}
	${CMAKE_THREAD_LIBS_INIT}
	spdlog::spdlog
	nfc
	archive_cpp_wrapper
)

#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#	COMMAND ${CMAKE_COMMAND} -E copy_directory
#		${CMAKE_CURRENT_SOURCE_DIR}/public
#		${CMAKE_CURRENT_BINARY_DIR}/public
#)

if (WIN32)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
			${CMAKE_CURRENT_SOURCE_DIR}/config.ini.sample
			${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/config.ini.sample
	)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
			${CMAKE_CURRENT_SOURCE_DIR}/license.txt
			${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/license.txt
	)
#	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#		COMMAND ${CMAKE_COMMAND} -E copy_directory
#			${CMAKE_CURRENT_SOURCE_DIR}/public
#			${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/public
#	)
endif()
