cmake_minimum_required(VERSION 3.0)

project(pngparts C)

option(BUILD_SHARED_LIBS "Build shared libraries")
option(PNGPARTS_INCLUDE_AUX "Compile the auxiliary modules")
set(PNGPARTS_PNGWRITE_CHUNK_SIZE CACHE STRING
  "Default IDAT chunk size (default: 7000)")

set(pngparts_src
  api.c api.h
  z.c z.h
  zread.c zread.h
  flate.h flate.c
  inflate.h inflate.c
  png.c png.h
  pngread.c pngread.h
  zwrite.c zwrite.h
  deflate.c deflate.h
  pngwrite.c pngwrite.h
  )
if (PNGPARTS_INCLUDE_AUX)
  list(APPEND pngparts_src auxi.c auxi.h)
endif (PNGPARTS_INCLUDE_AUX)

add_library(pngparts ${pngparts_src})
if (BUILD_SHARED_LIBS)
  if (WIN32)
    target_compile_definitions(pngparts PUBLIC "PNGPARTS_API_SHARED")
  endif(WIN32)
endif(BUILD_SHARED_LIBS)
if (PNGPARTS_PNGWRITE_CHUNK_SIZE GREATER 0)
  target_compile_definitions(pngparts
    PRIVATE "PNGPARTS_PNGWRITE_CHUNK_SIZE=${PNGPARTS_PNGWRITE_CHUNK_SIZE}")
endif(PNGPARTS_PNGWRITE_CHUNK_SIZE GREATER 0)
