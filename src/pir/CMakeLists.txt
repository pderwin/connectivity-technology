zephyr_library()

zephyr_library_sources(
	pir.c
	)


zephyr_include_directories(".")
zephyr_include_directories("$ENV{ZEPHYR_BASE}/drivers/sensor/lis2dh")
