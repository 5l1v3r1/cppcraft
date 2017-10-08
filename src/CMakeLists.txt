
set(SOURCES
    atmosphere.cpp
    biome.cpp
    blockmodels.cpp
    blockmodels_crosses.cpp
    blockmodels_cubes.cpp
    blockmodels_doors.cpp
    blockmodels_fences.cpp
    blockmodels_ladders.cpp
    blockmodels_lanterns.cpp
    blockmodels_leafs.cpp
    blockmodels_player.cpp
    blockmodels_poles.cpp
    blockmodels_stairs.cpp
    blocks_bordered.cpp
    block.cpp
    blocks_vfaces.cpp
    camera.cpp
    chat.cpp
    chunks.cpp
    columns.cpp
    compiler_schedule.cpp
    compilers.cpp
    compressor.cpp
    db/blockdb.cpp
    drawq.cpp
    drawq_rendergrid.cpp
    gameconf.cpp
    generator/biomegen/biome.cpp
    generator/biomegen/biomegen.cpp
    generator/biomegen/colortable.cpp
    generator/blocks.cpp
    generator.cpp
    generator/objectq.cpp
    generator/objects/basic_house.cpp
    generator/objects/basic_tree.cpp
    generator/objects/helpers.cpp
    generator/objects/jungle_tree.cpp
    generator/processing/oregen.cpp
    generator/processing/postproc.cpp
    generator/terragen.cpp
    generator/terrain/helpers.cpp
    generator/terrain/noise.cpp
    generator/terrain/terrain.cpp
    generator/terrain/terrains.cpp
    generator/terrain/t_grass.cpp
    generator/terrain/t_snow.cpp
    gui/bordered_frame.cpp
    gui/button.cpp
    gui/checkbox.cpp
    gui/inventory.cpp
    gui/item_renderer.cpp
    gui/menu.cpp
    gui/progress.cpp
    gui/window.cpp
    items.cpp
    lighting_algos.cpp
    lighting.cpp
    main.cpp
    meshes/vemitcross.cpp
    meshes/vemitter.cpp
    minimap.cpp
    netplayer.cpp
    netplayers.cpp
    particles.cpp
    particles_render.cpp
    player_actions.cpp
    player_actions_handlers.cpp
    player_actions_inputs.cpp
    player.cpp
    player_inputs.cpp
    player_logic.cpp
    player_physics.cpp
    player_translate.cpp
    precompiler.cpp
    precomp_optimize.cpp
    precompq.cpp
    precomp_thread_ao.cpp
    precomp_thread.cpp
    precomp_thread_index.cpp
    precomp_thread_light.cpp
    precomp_thread_process.cpp
    precomp_vdoors.cpp
    precomp_vfences.cpp
    precomp_vladders.cpp
    precomp_vlantern.cpp
    precomp_vpoles.cpp
    precomp_vsloped.cpp
    precomp_vstairs.cpp
    render_fs.cpp
    render_fsflare.cpp
    render_gui_compass.cpp
    render_gui.cpp
    render_gui_crosshair.cpp
    render_gui_helditem.cpp
    render_gui_interfaces.cpp
    render_gui_inventory.cpp
    render_gui_playerhand.cpp
    renderman.cpp
    render_player_selection.cpp
    render_scene_camera.cpp
    render_scene.cpp
    render_scene_terrain.cpp
    render_sky_clouds.cpp
    render_sky.cpp
    seamless.cpp
    sector.cpp
    sectors.cpp
    shaderman.cpp
    sound/channel.cpp
    soundman.cpp
    sound/sound.cpp
    sound/soundsystem.cpp
    sound/stream.cpp
    spiders.cpp
    spiders_modify.cpp
    spiders_world.cpp
    sun.cpp
    textureman.cpp
    textureman_generated.cpp
    threading.cpp
    threadpool.cpp
    tiles.cpp
    voxelmodels.cpp
    world.cpp
    worldmanager.cpp
    worldmanager_init.cpp
    worldmanager_teleport.cpp
  )

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "${ORIGIN}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath -Wl,.")

add_executable(cppcraft ${SOURCES})
set_target_properties(cppcraft PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if (WIN32)
  add_library(glfw STATIC IMPORTED)
  set_target_properties(glfw PROPERTIES LINKER_LANGUAGE C)
  set_target_properties(glfw PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/win64/libglfw3.a)

  add_library(glew STATIC IMPORTED)
  set_target_properties(glew PROPERTIES LINKER_LANGUAGE C)
  set_target_properties(glew PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/win64/glew32s.lib)

  add_library(bass STATIC IMPORTED)
  set_target_properties(bass PROPERTIES LINKER_LANGUAGE C)
  set_target_properties(bass PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/win64/bass.lib)

  add_library(lua STATIC IMPORTED)
  set_target_properties(lua PROPERTIES LINKER_LANGUAGE C)
  set_target_properties(lua PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/win64/liblua53.a)

  target_link_libraries(cppcraft imagehlp library lua bass glfw glew ${OPENGL_gl_LIBRARY})
endif()
if (UNIX)
  #add_library(bass SHARED IMPORTED)
  #set_target_properties(bass PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/libbass.so)

  target_link_libraries(cppcraft -static-libgcc -static-libstdc++ -pthread)
  target_link_libraries(cppcraft library -lbass libGLEW.a GL ${GLFW_STATIC_LIBRARIES} ${LUA_STATIC_LIBRARIES})
endif()

install(TARGETS cppcraft DESTINATION ${CMAKE_INSTALL_PREFIX})
