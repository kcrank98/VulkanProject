cmake_minimum_required(VERSION 3.2)

set(PROJECT_NAME Vulkan_Lab_Four)
project(${PROJECT_NAME})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
	PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

file(GLOB SOURCE_FILES CONFIGURE_DEPENDS ./*.h ./*.cpp)
file(GLOB SHADER_FILES CONFIGURE_DEPENDS ./*.hlsl ./*.glsl)

if(WIN32)
        # by default CMake selects "ALL_BUILD" as the startup project
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
	PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
	#this will tell Visual Studio to put the shaders under their own group in the solution explorer 
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
   	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VERTEX_SHADERS})
        source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PIXEL_SHADERS})
endif()

# currently using unicode on win32
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

if (WIN32)
	# shaderc_combined.lib in Vulkan requires this for debug & release (runtime shader compiling)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MD")
	add_executable (${PROJECT_NAME} ${SOURCE_FILES} ${SHADER_FILES})
	target_include_directories(${PROJECT_NAME} PUBLIC $ENV{VULKAN_SDK}/Include/)
	target_link_directories(${PROJECT_NAME} PUBLIC $ENV{VULKAN_SDK}/Lib/)
endif(WIN32)

if(UNIX AND NOT APPLE)
	# libshaderc_combined.a is required for runtime shader compiling
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lX11 -lshaderc_combined")
        find_package(X11)
	find_package(Vulkan REQUIRED)
        link_libraries(${X11_LIBRARIES})
        include_directories(${X11_INCLUDE_DIR})
        include_directories(${Vulkan_INCLUDE_DIR}) 
	link_libraries(${Vulkan_LIBRARIES})
        link_libraries(/usr/lib/x86_64-linux-gnu/libshaderc_combined.a)
        add_executable (${PROJECT_NAME} ${SOURCE_FILES} ${SHADER_FILES})
endif(UNIX AND NOT APPLE)

if(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fmodules -fcxx-modules")
	set(Architecture ${CMAKE_OSX_ARCHITECTURES})
	find_package(Vulkan REQUIRED)
	include_directories(${Vulkan_INCLUDE_DIR}) 
	#link_directories(${Vulkan_LIBRARY}) this is currently not working
	link_libraries(${Vulkan_LIBRARIES})
	# libshaderc_combined.a is required for runtime shader compiling
	# the path is (properly)hardcoded because "${Vulkan_LIBRARY}" currently does not 
	# return a proper path on MacOS (it has the .dynlib appended)
	link_libraries(/usr/local/lib/libshaderc_combined.a)
	add_executable (${PROJECT_NAME} main.mm)
endif(APPLE)

#exclude shaders from build, we compile at run-time
set_source_files_properties(${SHADER_FILES} PROPERTIES VS_TOOL_OVERRIDE "none")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND git add --all || true
	COMMAND git commit -a -m "auto commit" || true
	COMMAND git push
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	VERBATIM USES_TERMINAL
)

