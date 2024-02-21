CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

PROJECT(home-security C)

# set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_BUILD_TYPE Debug)

SET(MQTT_TEST_BROKER    "tcp://localhost:1883" CACHE STRING "Hostname of a test MQTT broker to use")
SET(MQTT_WS_TEST_BROKER "ws://localhost:1883" CACHE STRING "WebSocket connection to a test MQTT broker")
SET(MQTT_TEST_PROXY     "tcp://localhost:1884" CACHE STRING "Hostname of the test proxy to use")
SET(MQTT_WS_TEST_PROXY  "tcp://localhost:1884" CACHE STRING "WebSocket connection of a test proxy to use")
SET(MQTT_SSL_HOSTNAME   "localhost" CACHE STRING "Hostname of a test SSL MQTT broker to use")

SET(CERTDIR ${CMAKE_SOURCE_DIR}/test/ssl)


IF (WIN32)
	SET(LIBS_SYSTEM ws2_32)
ELSEIF (UNIX)
	IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
		SET(LIBS_SYSTEM c dl pthread)
	ELSEIF (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
		SET(LIBS_SYSTEM compat pthread)
	ELSE()
		SET(LIBS_SYSTEM c pthread)
	ENDIF()
ENDIF()

add_compile_options(-Wextra -Wall -Werror)

include_directories( ../../lib/paho.mqtt.c/src )
link_directories(    ../../lib/paho.mqtt.c/build/src )

ADD_EXECUTABLE( home_security
  base64.c
  home_security.c
  message.c
  event.c
  )

target_link_libraries (home_security paho-mqtt3c yaml )
