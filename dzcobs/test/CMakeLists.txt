# ===-----------------------------------------------------------------------===#
# Distributed under the 3-Clause BSD License. See accompanying file LICENSE or
# copy at https://opensource.org/licenses/BSD-3-Clause).
# SPDX-License-Identifier: BSD-3-Clause
# ===-----------------------------------------------------------------------===#

# ==============================================================================
# Build instructions
# ==============================================================================

set(MAIN_TEST_TARGET_NAME ${MODULE_TARGET_NAME}_test)

asap_push_module("${MAIN_TEST_TARGET_NAME}")

asap_add_test(
  ${MAIN_TEST_TARGET_NAME}
  UNIT_TEST
  SRCS
  "main.cpp"
  "checksum/test_checksum.cpp"
  "dzcobs/test_dzcobs.cpp"
  "dictionary/test_dictionary.cpp"
  LINK
  CppUTest::CppUTest
  CppUTest::CppUTestExt
  dzcobs::dzcobs
  COMMENT
  "unit tests"
)
target_include_directories(${MAIN_TEST_TARGET_NAME} PRIVATE "../src")

asap_pop_module("${MAIN_TEST_TARGET_NAME}")
