# Add a custom command that produces version.cpp, plus
# a dummy output that's not actually produced, in order
# to force version.cmake to always be re-run before the build

set (VERSION_C_FILE ${CMAKE_CURRENT_BINARY_DIR}/stress_version.c)

ADD_CUSTOM_COMMAND(
    OUTPUT  ${VERSION_C_FILE} ${CMAKE_CURRENT_BINARY_DIR}/_version.c
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/update_version.pl ${CMAKE_CURRENT_SOURCE_DIR} ${VERSION_C_FILE}
    )

zephyr_library()
zephyr_library_postprocess()

zephyr_library_compile_options(
    -Wextra
    -Werror
)

zephyr_library_include_directories(
    include
    ../src/bootrom/src
    ${CMAKE_ZEPHYR_INCPATH}
    ${CMAKE_CURRENT_BINARY_DIR}
)

zephyr_library_sources(
  ${VERSION_C_FILE}
)

zephyr_library_sources(
  lora.c
)
