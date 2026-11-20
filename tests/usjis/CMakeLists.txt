cmake_minimum_required(VERSION 3.20.0)

if(NOT DEFINED ZMK_SOURCE_DIR OR NOT DEFINED OSKEY_SOURCE_DIR)
  message(FATAL_ERROR "Set ZMK_SOURCE_DIR and OSKEY_SOURCE_DIR to the pinned dependencies")
endif()
set(CONFIG_APPLICATION_DEFINED_SYSCALL true)
list(APPEND SYSCALL_INCLUDE_DIRS ${ZMK_SOURCE_DIR}/app/include)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(coron44bt_usjis_simulation)

target_include_directories(app PRIVATE
  ${ZMK_SOURCE_DIR}/app/include
  ${OSKEY_SOURCE_DIR}/include
)
zephyr_linker_sources(RODATA ${ZMK_SOURCE_DIR}/app/include/linker/zmk-events.ld)
# Use the exact HID and event implementations linked by the firmware.
# Only the endpoint transport and selected OS/layer state are test stubs.
target_sources(app PRIVATE
  src/main.c
  ${ZMK_SOURCE_DIR}/app/src/hid.c
  ${ZMK_SOURCE_DIR}/app/src/hid_listener.c
  ${ZMK_SOURCE_DIR}/app/src/event_manager.c
  ${ZMK_SOURCE_DIR}/app/src/events/keycode_state_changed.c
  ${ZMK_SOURCE_DIR}/app/src/events/endpoint_changed.c
)
