cmake_minimum_required(VERSION 3.20)
project(GalaxyNativeFoundation LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PETARI "${CMAKE_CURRENT_SOURCE_DIR}/../Petari" CACHE PATH "Petari checkout")
add_library(petari_state STATIC
    ${PETARI}/src/Game/LiveActor/Nerve.cpp
    ${PETARI}/src/Game/LiveActor/Spine.cpp
    ${PETARI}/src/Game/LiveActor/ActorStateKeeper.cpp
    ${PETARI}/src/Game/LiveActor/ActorStateBase.cpp
    ${PETARI}/src/Game/System/NerveExecutor.cpp
    ${PETARI}/src/Game/NameObj/NameObj.cpp
    ${PETARI}/src/Game/NameObj/NameObjRegister.cpp
    ${PETARI}/src/Game/NameObj/NameObjAdaptor.cpp
    ${PETARI}/src/Game/Map/SwitchWatcher.cpp
    ${PETARI}/src/Game/Map/ActorAppearSwitchListener.cpp
    ${PETARI}/src/Game/Util/SwitchEventFunctorListener.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/switch_watcher_port.cpp
    ${PETARI}/src/Game/LiveActor/HitSensor.cpp
    ${PETARI}/src/Game/LiveActor/HitSensorInfo.cpp
    ${PETARI}/src/Game/LiveActor/HitSensorKeeper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sensor_hash_port.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sensor_group_port.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sensor_checker_port.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sensor_routing_port.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sensor_messages_port.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/live_actor_messages_port.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/live_actor_nerve_port.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/actor_timing_port.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/actor_state_util_port.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/coin_rewards_port.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/player_status_port.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/galaxy_progress_port.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stage_result_port.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stage_memory_port.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/restart_cube_port.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/area_volume_port.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/death_area_port.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/player_event_sequence_port.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/player_death_event_port.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/miss_layout_port.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wipe_koopa_port.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bck_settings_port.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/jmap_port.cpp
    ${PETARI}/src/JSystem/JGadget/hashcode.cpp)
target_include_directories(petari_state PUBLIC include ${PETARI}/include ${PETARI}/libs/JSystem/include)
add_executable(galaxy-probe src/main.cpp src/sdk_matrix_port.cpp src/hash_sort_table_port.cpp)
target_link_libraries(galaxy-probe PRIVATE petari_state)
if(WIN32)
    target_link_libraries(galaxy-probe PRIVATE bcrypt d3d11 dxgi user32)
    if(MINGW)
        target_link_libraries(galaxy-probe PRIVATE d3dcompiler_47)
    else()
        target_link_libraries(galaxy-probe PRIVATE d3dcompiler)
    endif()
else()
    message(FATAL_ERROR "The asset decryption backend currently requires Windows CNG")
endif()
enable_testing()
add_test(NAME state_machine COMMAND galaxy-probe --self-test)



























