cmake_minimum_required(VERSION 3.1)
project(theorize C CXX)

find_library(theoraenc_lib NAMES theoraenc)
find_library(theoradec_lib NAMES theoradec)
find_path(theora_include NAMES "theora/theoraenc.h")

set(theorize_SOURCES
	"src/main.cpp"
  )

add_executable(theorize "${theorize_SOURCES}")
target_include_directories(theorize
	PRIVATE "${theora_include}")
target_link_libraries(theorize
	PRIVATE "${theoraenc_lib}" "${theoradec_lib}")
