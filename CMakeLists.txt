if(CONFIG_ZMK_OS_DETECTION AND CONFIG_ZMK_OSKEY_STATE)
  target_sources(app PRIVATE src/auto_oskey.c)
endif()

if(CONFIG_ZMK_USJIS)
  add_subdirectory(third_party/usjis)
endif()
