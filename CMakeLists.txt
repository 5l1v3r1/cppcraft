cmake_minimum_required (VERSION 3.0.2)
project (cppcraft C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -march=native")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=thread")
set(BUILD_SHARED_LIBRARIES OFF)


if (UNIX)
  include(FindPkgConfig)

  pkg_check_modules(GLEW REQUIRED glew)
  pkg_check_modules(GLFW REQUIRED glfw3)
  include_directories("${GLEW_STATIC_INCLUDE_DIRS} ${GLFW_STATIC_INCLUDE_DIRS}")
  link_directories("${GLEW_STATIC_LIBRARY_DIRS} ${GLFW_STATIC_LIBRARY_DIRS}")

  pkg_check_modules(LUA REQUIRED lua-5.3)
  include_directories("${LUA_STATIC_INCLUDE_DIRS}")
  link_directories("${LUA_STATIC_LIBRARY_DIRS}")
endif()
if (WIN32)
  find_package(OpenGL REQUIRED)
  SET(CMAKE_EXE_LINKER_FLAGS "-static")

endif()

add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

include_directories(inc)
include_directories(inc/glm)
include_directories(inc/LuaBridge/Source/LuaBridge)
include_directories(library/include)

add_subdirectory(library)
add_subdirectory(src)
