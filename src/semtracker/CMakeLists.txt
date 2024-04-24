zephyr_library()

zephyr_library_sources(
  config.c
  downlink.c
  lr11xx_firmware.c
  semtracker.c
  )

zephyr_include_directories( . )
