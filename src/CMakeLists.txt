# Add a custom command that produces version.cpp, plus
# a dummy output that's not actually produced, in order
# to force version.cmake to always be re-run before the build

zephyr_library()
zephyr_library_compile_options( -Wextra -Werror )

zephyr_library_include_directories( )

zephyr_library_sources(
  ct_main.c
)

add_subdirectory(accel)
