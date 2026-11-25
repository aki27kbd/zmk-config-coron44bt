cmake_minimum_required(VERSION 3.20.0)
if(NOT DEFINED ZMK_SOURCE_DIR OR NOT DEFINED OSKEY_SOURCE_DIR)
  message(FATAL_ERROR "Set the pinned ZMK_SOURCE_DIR and OSKEY_SOURCE_DIR")
endif()
set(CONFIG_APPLICATION_DEFINED_SYSCALL true)
list(APPEND DTS_ROOT ${ZMK_SOURCE_DIR}/app)
list(APPEND SYSCALL_INCLUDE_DIRS ${ZMK_SOURCE_DIR}/app/include)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(coron44bt_z_shift_simulation)
target_include_directories(app PRIVATE ${ZMK_SOURCE_DIR}/app/include ${OSKEY_SOURCE_DIR}/include)
zephyr_linker_sources(RODATA
  ${ZMK_SOURCE_DIR}/app/include/linker/zmk-events.ld
  ${ZMK_SOURCE_DIR}/app/include/linker/zmk-behaviors.ld
)
target_sources(app PRIVATE
  src/main.c
  src/keymap.c
  ${ZMK_SOURCE_DIR}/app/src/behavior.c
  ${ZMK_SOURCE_DIR}/app/src/behaviors/behavior_key_press.c
  ${ZMK_SOURCE_DIR}/app/src/hid.c
  ${ZMK_SOURCE_DIR}/app/src/hid_listener.c
  ${ZMK_SOURCE_DIR}/app/src/event_manager.c
  ${ZMK_SOURCE_DIR}/app/src/events/keycode_state_changed.c
  ${ZMK_SOURCE_DIR}/app/src/events/position_state_changed.c
  ${ZMK_SOURCE_DIR}/app/src/events/endpoint_changed.c
)
