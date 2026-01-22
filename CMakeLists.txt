cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# Print toolchain file to check if visual studio overwrites VCPKG_ROOT (rename/delete .vcpkg-root to fix)
message(STATUS "CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")

# Include more modules, like Find for the FreeImage library
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

# Binary name
set(PROJECT_TARGETBIN "GLMaze")
# Human-readable project name
set(PROJECT_NAME "GLMaze")
set(PROJECT_DESCRIPTION "GLMaze is a Windows 95 Maze screensaver clone.")
# Identifier of project
set(PROJECT_IDENTIFIER "tech.ulti.glmaze")
# Versioning format: http://semver.org
set(PROJECT_MAJOR_VERSION 1)
set(PROJECT_MINOR_VERSION 0)
set(PROJECT_PATCH_VERSION 0)
set(PROJECT_COMPANY "ultitech")
string(TIMESTAMP YEAR "%Y")
set(PROJECT_COPYRIGHT "Copyright © 2006-${YEAR}, ${PROJECT_COMPANY}. All rights reserved.")

find_package(Git)
if(GIT_FOUND)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            OUTPUT_VARIABLE PROJECT_REVISION
            OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

# Define project name and language
project(${PROJECT_TARGETBIN} C)

# Add all source and header files
file(GLOB SRC "src/*.h" "src/*.c")
file(GLOB_RECURSE RESOURCES "res/*")

# Locate required libraries
Find_Package ( OpenGL REQUIRED )
Find_Package ( SDL2 CONFIG REQUIRED )
Find_Package ( GLEW CONFIG REQUIRED )

#Find_Package ( ZLIB REQUIRED )
Find_Package ( FreeImage CONFIG REQUIRED )

# Link all libraries
link_libraries (
  OpenGL::GL
  SDL2::SDL2
  GLEW::GLEW
  freeimage::FreeImage
)

if(WIN32)
  link_libraries(SDL2::SDL2main)

  # Hide console on Windows on release builds
  add_link_options($<$<NOT:$<CONFIG:Debug>>:/SUBSYSTEM:WINDOWS>)

  add_definitions(/D _CRT_SECURE_NO_WARNINGS)

  set(WIN_DLLS $<TARGET_FILE:GLEW::GLEW> $<TARGET_FILE:SDL2::SDL2> $<TARGET_FILE:freeimage::FreeImage>)
ENDIF(WIN32)

if(UNIX AND NOT APPLE)
  link_libraries(-lm)
endif()

# Create Executable
add_executable(${PROJECT_TARGETBIN} ${SRC} ${RESOURCES})
if(WIN32)
  add_executable(${PROJECT_TARGETBIN}_Screensaver ${SRC} ${RESOURCES})
  target_compile_definitions(${PROJECT_TARGETBIN}_Screensaver PRIVATE SCREENSAVER=1)
  set_target_properties(${PROJECT_TARGETBIN}_Screensaver PROPERTIES OUTPUT_NAME "GLMaze")
  set_target_properties(${PROJECT_TARGETBIN}_Screensaver PROPERTIES SUFFIX ".scr")
endif(WIN32)

if(UNIX AND NOT APPLE)
  set_target_properties(${PROJECT_TARGETBIN} PROPERTIES RUNTIME_OUTPUT_DIRECTORY GLMaze)
endif(UNIX AND NOT APPLE)

if(UNIX AND NOT APPLE OR WIN32)
  add_custom_command(TARGET ${PROJECT_TARGETBIN} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${PROJECT_SOURCE_DIR}/res $<TARGET_FILE_DIR:${PROJECT_TARGETBIN}>)
endif(UNIX AND NOT APPLE OR WIN32)

if(WIN32)
 foreach(DLL ${WIN_DLLS})
 	add_custom_command(TARGET ${PROJECT_TARGETBIN} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLL} $<TARGET_FILE_DIR:${PROJECT_TARGETBIN}>)
 endforeach(DLL)
endif(WIN32)

# Visual Studio specific settings
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Noice)
source_group(TREE "${PROJECT_SOURCE_DIR}/src" PREFIX "Source" FILES ${SRC})
source_group(TREE "${PROJECT_SOURCE_DIR}/res" PREFIX "Resources" FILES ${RESOURCES})
