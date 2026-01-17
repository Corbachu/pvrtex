cmake_minimum_required(VERSION 3.16)

#1/17/26 ~CA: Coversion from Makefile to CMakeLists.txt
project(pvrtex LANGUAGES C CXX)

# Options roughly matching the Makefile behavior.
option(PVRTX_MARCH_NATIVE "Enable -march=native on GCC/Clang (Release)" ON)
option(PVRTX_ENABLE_PROFILING "Enable -pg in Debug (GCC/Clang)" OFF)
option(PVRTX_BUILD_README "Add a README generation target (requires fmt)" ON)

set(PVRTX_SOURCES
  avstring.c
  bprint.c
  crc.c
  elbg.c
  lfg.c
  log.c
  md5.c
  mem.c
  mycommon.c
  optparse_impl.c
  palette.c
  pvr_texture.c
  pvr_texture_decoder.c
  pvr_texture_encoder.c
  stb_image_impl.c
  stb_image_resize_impl.c
  stb_image_write_impl.c
  tddither.c
  vqcompress.c
  file_common.c
  file_pvr.c
  file_tex.c
  file_dctex.c
  main.c

  dither.cpp
)

add_executable(pvrtex ${PVRTX_SOURCES})

target_include_directories(pvrtex PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/libavutil
)

target_compile_definitions(pvrtex PRIVATE
  CONFIG_MEMORY_POISONING=0
  HAVE_FAST_UNALIGNED=0
)

# Warnings / basic flags
if (MSVC)
  target_compile_options(pvrtex PRIVATE /W4)
else()
  target_compile_options(pvrtex PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
    -Wno-sign-compare
    $<$<COMPILE_LANGUAGE:C>:-Wno-pointer-sign>
  )
endif()

# Optimization / profiling settings (mirror Makefile intent)
if (NOT MSVC)
  target_compile_options(pvrtex PRIVATE
    $<$<CONFIG:Debug>:-Og>
    $<$<CONFIG:Debug>:-g>
    $<$<AND:$<CONFIG:Debug>,$<BOOL:${PVRTX_ENABLE_PROFILING}>>:-pg>

    $<$<CONFIG:Release>:-O3>
    $<$<AND:$<CONFIG:Release>,$<BOOL:${PVRTX_MARCH_NATIVE}>>:-march=native>
  )
  target_link_options(pvrtex PRIVATE
    $<$<AND:$<CONFIG:Debug>,$<BOOL:${PVRTX_ENABLE_PROFILING}>>:-pg>
  )
endif()

# Link libm on Unix (Makefile uses -lm)
if (UNIX)
  target_link_libraries(pvrtex PRIVATE m)
endif()

# LTO (Makefile uses -flto=auto in Release)
include(CheckIPOSupported)
check_ipo_supported(RESULT _ipo_supported OUTPUT _ipo_error)
if (_ipo_supported)
  set_property(TARGET pvrtex PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()

# README generation target (Makefile: fmt -s readme_unformatted.txt > README)
if (PVRTX_BUILD_README)
  find_program(PVRTX_FMT_EXECUTABLE fmt)
  if (PVRTX_FMT_EXECUTABLE)
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/README
      COMMAND ${CMAKE_COMMAND}
        -DFMT=${PVRTX_FMT_EXECUTABLE}
        -DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/readme_unformatted.txt
        -DOUTPUT=${CMAKE_CURRENT_SOURCE_DIR}/README
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateReadme.cmake
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/readme_unformatted.txt
      VERBATIM
    )
    add_custom_target(readme ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/README)
    add_dependencies(pvrtex readme)
  endif()
endif()

include(GNUInstallDirs)
install(TARGETS pvrtex RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/pvrtex-completion.bash
  DESTINATION ${CMAKE_INSTALL_DATADIR}/bash-completion/completions
  RENAME pvrtex.bash
)
