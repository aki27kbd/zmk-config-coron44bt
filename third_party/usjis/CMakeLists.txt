# SPDX-License-Identifier: MIT
# Adapted from goyamamoto/zmk-kb1-usjis; see LICENSE and README.md.

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_sources(app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/usjis_resolver.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/usjis.c
)

# ZMK iterates event subscriptions in link order. Run after hold-tap,
# sticky-key, caps-word and key-repeat, immediately before hid_listener.
function(coron44bt_place_usjis_listener)
  get_property(listener GLOBAL PROPERTY CORON44BT_USJIS_LISTENER_SOURCE)
  get_target_property(sources app SOURCES)
  list(REMOVE_ITEM sources ${listener})
  set(index -1)
  set(i 0)
  foreach(source ${sources})
    if(source MATCHES "/hid_listener\\.c$")
      set(index ${i})
    endif()
    math(EXPR i "${i} + 1")
  endforeach()
  if(index EQUAL -1)
    message(FATAL_ERROR "coron44bt US-JIS: hid_listener.c missing")
  endif()
  list(INSERT sources ${index} ${listener})
  set_property(TARGET app PROPERTY SOURCES "${sources}")
  message(STATUS "coron44bt US-JIS: listener at index ${index}, before hid_listener")
endfunction()
set_property(GLOBAL PROPERTY CORON44BT_USJIS_LISTENER_SOURCE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/usjis.c)
cmake_language(DEFER DIRECTORY ${CMAKE_SOURCE_DIR} CALL coron44bt_place_usjis_listener)
