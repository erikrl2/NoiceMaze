cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(NoiceMaze LANGUAGES C CXX)

find_package(OpenGL REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(glew CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED)

file(GLOB GLMaze_SRC CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/*.h"
  "${PROJECT_SOURCE_DIR}/src/*.c"
)
file(GLOB Noice_SRC CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/Noice/*.hpp"
  "${PROJECT_SOURCE_DIR}/src/Noice/*.h"
  "${PROJECT_SOURCE_DIR}/src/Noice/*.cpp"
  "${PROJECT_SOURCE_DIR}/src/Noice/*.c"
)
file(GLOB GLMaze_SHADER CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/res/Shader/*.glsl"
)
file(GLOB Noice_SHADER CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/res/Shader/Noice/*.glsl"
)

add_library(Noice STATIC ${Noice_SRC})
target_link_libraries(Noice PRIVATE OpenGL::GL GLEW::GLEW glm::glm)
target_compile_features(Noice PRIVATE cxx_std_20)

add_library(NoiceMaze_deps INTERFACE)
target_link_libraries(NoiceMaze_deps INTERFACE
  OpenGL::GL
  SDL2::SDL2
  GLEW::GLEW
  Noice
)

if(WIN32)
  target_link_libraries(NoiceMaze_deps INTERFACE SDL2::SDL2main)
  target_compile_definitions(NoiceMaze_deps INTERFACE _CRT_SECURE_NO_WARNINGS)
  target_link_options(NoiceMaze_deps INTERFACE $<$<NOT:$<CONFIG:Debug>>:/SUBSYSTEM:WINDOWS>)
endif()

add_executable(NoiceMaze ${GLMaze_SRC})
target_link_libraries(NoiceMaze PRIVATE NoiceMaze_deps)
target_include_directories(NoiceMaze PRIVATE ${Stb_INCLUDE_DIR})

add_custom_command(TARGET NoiceMaze POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${PROJECT_SOURCE_DIR}/res"
          "$<TARGET_FILE_DIR:NoiceMaze>"
)

if(WIN32)
  add_executable(NoiceMaze_Screensaver ${GLMaze_SRC})
  target_link_libraries(NoiceMaze_Screensaver PRIVATE NoiceMaze_deps)
  target_compile_definitions(NoiceMaze_Screensaver PRIVATE SCREENSAVER=1)
  set_target_properties(NoiceMaze_Screensaver PROPERTIES OUTPUT_NAME "NoiceMaze" SUFFIX ".scr")
endif()

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT NoiceMaze)
source_group(TREE "${PROJECT_SOURCE_DIR}/src" PREFIX "Source" FILES ${GLMaze_SRC} ${Noice_SRC})
target_sources(NoiceMaze PRIVATE ${GLMaze_SHADER})
target_sources(Noice PRIVATE ${Noice_SHADER})
source_group(TREE "${PROJECT_SOURCE_DIR}/res/Shader" PREFIX "Shader" FILES ${GLMaze_SHADER} ${Noice_SHADER})
