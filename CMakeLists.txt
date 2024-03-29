cmake_minimum_required(VERSION 3.1)
project(theorize C CXX)

find_library(theoraenc_lib NAMES theoraenc)
find_library(theoradec_lib NAMES theoradec)
find_path(theora_include NAMES "theora/theoraenc.h")
find_path(ogg_include NAMES "ogg/ogg.h")
find_library(ogg_lib NAMES ogg)

set(PNGPARTS_INCLUDE_AUX ON)
add_subdirectory("deps/png-parts")

set(theorize_SOURCES
	"src/main.cpp"
	"src/yccbox.cpp"      "src/yccbox.hpp"
	"src/pngycc.cpp"      "src/pngycc.hpp"
  )

add_executable(theorize "${theorize_SOURCES}")
target_compile_features(theorize
	PRIVATE cxx_nullptr cxx_constexpr
	cxx_deleted_functions
	)
target_include_directories(theorize
	PRIVATE "${theora_include}" "${ogg_include}")
target_link_libraries(theorize
	PRIVATE pngparts "${theoraenc_lib}" "${theoradec_lib}"
	"${ogg_lib}")

if (UNIX)
  target_link_libraries(theorize PRIVATE m)
endif (UNIX)
