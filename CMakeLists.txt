cmake_minimum_required(VERSION 3.18)
project(ebfm_comin
  VERSION 0.0.1
  LANGUAGES C
)

include(CTest)

find_package(ComIn REQUIRED)

add_library(ebfm_comin MODULE
    main.c
)

add_library(tas_fake MODULE
    test/fake.c
)

target_link_libraries(ebfm_comin ComIn::Plugin)
target_link_libraries(tas_fake ComIn::Plugin)

# TODO: Provide replay data more fitting for EBFM
# comin_add_replay_data(
#   NAME ebfm_comin_replay_data
#   URL "TODO"
#   MD5HASH TODO
# )

comin_add_replay_test(
  NAME ebfm_comin_replay
  # REPLAY_DATA ebfm_comin_replay_data
  REPLAY_DATA test_nwp_R02B04
  NUM_PROCS 1
)
comin_test_add_plugin(
  TEST ebfm_comin_replay
  NAME "ebfm_comin"
  PLUGIN_LIBRARY "$<TARGET_FILE:ebfm_comin>"
)
comin_test_add_plugin(
  TEST ebfm_comin_replay
  NAME "tas_fake"
  PLUGIN_LIBRARY "$<TARGET_FILE:tas_fake>"
)
