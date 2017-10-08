cmake_minimum_required (VERSION 3.0.2)
project (cppcraft C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

include_directories(../src)
include_directories(../library/include)
include_directories(../inc)
include_directories(../inc/glm)
include_directories(Catch/include)

set(SOURCES
    test_lighting.cpp
    test_sector.cpp
    catch.cpp
    mock_generator.cpp
    mock_player.cpp
    mock_precompq.cpp
    mock_sectors.cpp
    ../src/block.cpp
    ../src/db/blockdb.cpp
    ../src/gameconf.cpp
    ../src/lighting.cpp
    ../src/lighting_algos.cpp
    ../src/sector.cpp
    ../src/sectors.cpp
    ../src/spiders.cpp
    ../src/spiders_modify.cpp
    ../src/spiders_world.cpp
    ../src/world.cpp
  )

set(LIB_SOURCES
    ../library/library/config.cpp
    ../library/library/log.cpp
  )

add_executable(unittests ${SOURCES} ${LIB_SOURCES})
