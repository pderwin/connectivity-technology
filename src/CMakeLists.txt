# Add a custom command that produces version.cpp, plus
# a dummy output that's not actually produced, in order
# to force version.cmake to always be re-run before the build

zephyr_library()

zephyr_library_include_directories( ./led )
zephyr_library_include_directories( ./pe4259 )

zephyr_library_sources(
  ct_main.c
  debug_gpio.c
  gpio_mapper.c
  stubs.c
  wifi.c
)

add_subdirectory(accel)
add_subdirectory(bluetooth)
add_subdirectory(driveway_sensor)
add_subdirectory(led)
add_subdirectory(pe4259)
add_subdirectory(pir)
add_subdirectory(temperature_humidity)
