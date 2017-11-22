
SET(COMMON_SOURCES
    db/blockdata.cpp
    db/itemdata.cpp
    biomes.cpp
  )

add_library(common STATIC ${COMMON_SOURCES})
