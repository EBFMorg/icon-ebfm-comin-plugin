# SPDX-FileCopyrightText: 2026 The ICON EBFM ComIn Plugin Authors
#
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.18)
project(ebfm_comin
  VERSION 0.0.1
  LANGUAGES C
)

include(CTest)

find_package(ComIn REQUIRED)
find_package(YAC REQUIRED)
find_package(MPI REQUIRED)

add_library(ebfm_comin MODULE
    main.c
)

add_library(tas_fake MODULE
    test/fake.c
)

add_executable(ebfm_receiver
    test/receiver.c
)

target_link_libraries(ebfm_comin ComIn::Plugin YAC::yac_mci MPI::MPI_C)
target_link_libraries(tas_fake ComIn::Plugin)
target_link_libraries(ebfm_receiver YAC::yac_mci)

# TODO: Provide replay data more fitting for EBFM
# comin_add_replay_data(
#   NAME ebfm_comin_replay_data
#   URL "TODO"
#   MD5HASH TODO
# )

## Smoke test with plugin sending data
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

## Test to run a receiver alongside the comin plugin; checking for exchange of
## values
comin_add_replay_test(
  NAME ebfm_comin_cpl
  REPLAY_DATA test_nwp_R02B04
  NUM_PROCS 1
)
comin_test_add_plugin(
  TEST ebfm_comin_cpl
  NAME "ebfm_comin"
  PLUGIN_LIBRARY "$<TARGET_FILE:ebfm_comin>"
  COMM "ebfm_comin_cpl"
)
comin_test_add_plugin(
  TEST ebfm_comin_cpl
  NAME "tas_fake"
  PLUGIN_LIBRARY "$<TARGET_FILE:tas_fake>"
)
# Receiver will get data from plugin and print it
comin_test_add_external_process(
  TEST ebfm_comin_cpl
  NUM_PROCS 1
  COMMAND "$<TARGET_FILE:ebfm_receiver>"
)
