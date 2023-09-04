cmake_minimum_required(VERSION 3.0)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  option(PNGPARTS_BUILD_TESTING "Enable tests for png-parts" ON)
else (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  option(PNGPARTS_BUILD_TESTING "Enable tests for png-parts" OFF)
endif (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

if (PNGPARTS_BUILD_TESTING AND BUILD_TESTING)
  add_executable(pngparts_test_api "test-api.c")
  target_link_libraries(pngparts_test_api pngparts)

  add_executable(pngparts_test_z "test-z.c")
  target_link_libraries(pngparts_test_z pngparts)

  add_executable(pngparts_test_zread "test-zread.c")
  target_link_libraries(pngparts_test_zread pngparts)

  add_executable(pngparts_test_huff "test-huff.c")
  target_link_libraries(pngparts_test_huff pngparts)

  add_executable(pngparts_test_png "test-png.c")
  target_link_libraries(pngparts_test_png pngparts)

  add_executable(pngparts_test_pngread "test-pngread.c")
  target_link_libraries(pngparts_test_pngread pngparts)

  add_executable(pngparts_test_zwrite "test-zwrite.c")
  target_link_libraries(pngparts_test_zwrite pngparts)

  add_executable(pngparts_test_pngwrite "test-pngwrite.c")
  target_link_libraries(pngparts_test_pngwrite pngparts)

  add_executable(pngparts_test_hash "test-hash.c")
  target_link_libraries(pngparts_test_hash pngparts)

  add_executable(pngparts_test_flate "test-flate.c")
  target_link_libraries(pngparts_test_flate pngparts)

  if (PNGPARTS_INCLUDE_AUX)
    add_executable(pngparts_test_aux_pngread "test-aux-pngread.c")
    target_link_libraries(pngparts_test_aux_pngread pngparts)

    add_executable(pngparts_test_aux_pngwrite "test-aux-pngwrite.c")
    target_link_libraries(pngparts_test_aux_pngwrite pngparts)

    add_executable(pngparts_test_aux8_pngread "test-aux8-pngread.c")
    target_link_libraries(pngparts_test_aux8_pngread pngparts)

    add_executable(pngparts_test_aux8_pngwrite "test-aux8-pngwrite.c")
    target_link_libraries(pngparts_test_aux8_pngwrite pngparts)
  endif (PNGPARTS_INCLUDE_AUX)

  # adapted from:
  # https://cliutils.gitlab.io/modern-cmake/chapters/testing.html
  add_test(NAME pngparts_test_api COMMAND pngparts_test_api)
  add_test(NAME "pngparts_test_z::header_check"
    COMMAND pngparts_test_z "header_check" "-cm" "8" "-cinfo" "7"
      "-fdict" "0" "-flevel" "0" "-fcheck" "1")
  add_test(NAME "pngparts_test_huff::fixed"
    COMMAND pngparts_test_huff "-f")
  add_test(NAME "pngparts_test_huff::fixed_runtime"
    COMMAND pngparts_test_huff "-fx")
  add_test(NAME "pngparts_test_huff::ascii"
    COMMAND pngparts_test_huff "-z")
  add_test(NAME "pngparts_test_huff::variable"
    COMMAND pngparts_test_huff "-vc" "-s" "-")
  add_test(NAME "pngparts_test_flate::length_encode"
    COMMAND pngparts_test_flate "length_encode" "253")
  add_test(NAME "pngparts_test_flate::length_decode"
    COMMAND pngparts_test_flate "length_decode" "284" "26")
  add_test(NAME "pngparts_test_flate::distance_encode"
    COMMAND pngparts_test_flate "distance_encode" "7013")
  add_test(NAME "pngparts_test_flate::distance_decode"
    COMMAND pngparts_test_flate "distance_decode" "25" "868")
endif (PNGPARTS_BUILD_TESTING AND BUILD_TESTING)
