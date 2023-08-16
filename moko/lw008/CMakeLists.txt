# /*============================================================================*
#  *         Copyright © 2019-2021 Signetik, LLC -- All Rights Reserved         *
#  *----------------------------------------------------------------------------*
#  *                                                              Signetik, LLC *
#  *                                                           www.signetik.com *
#  *                                          SPDX-License-Identifier: Sigentik *
#  *                                                                            *
#  * Customer may modify, compile, assemble and convert this source code into   *
#  * binary object or executable code for use on Signetk products purchased     *
#  * from Signetik or its distributors.                                         *
#  *                                                                            *
#  * Customer may incorporate or embed an binary executable version of the      *
#  * software into the Customer’s product(s), which incorporate a Signetik      *
#  * product purchased from Signetik or its distributors. Customer may          *
#  * manufacture, brand and distribute such Customer’s product(s) worldwide to  *
#  * its End-Users.                                                             *
#  *                                                                            *
#  * This agreement must be formalized with Signetik before Customer enters     *
#  * production and/or distributes products to Customer's End-Users             *
#  *============================================================================*/

cmake_minimum_required(VERSION 3.20)

set(LORA_BASICS_ROOT       $ENV{ZEPHYR_BASE}/../SWSD004 )

set(APPS                   ${LORA_BASICS_ROOT}/apps )
set(LORA_BASICS_MODEM      ${LORA_BASICS_ROOT}/lora_basics_modem/lora_basics_modem )
set(SMTC_MODEM_CORE        ${LORA_BASICS_MODEM}/smtc_modem_core )
set(SMTC_MODEM_SERVICES    ${SMTC_MODEM_CORE}/smtc_modem_services )
set(LR1MAC                 ${SMTC_MODEM_CORE}/lr1mac )

set(GEOLOCATION_MIDDLEWARE ${LORA_BASICS_ROOT}/geolocation_middleware )
set(SMTC_HAL               ${LORA_BASICS_ROOT}/smtc_hal )
set(PRINTERS               ${LORA_BASICS_ROOT}/lora_basics_modem/printers )
set(SHIELDS                ${LORA_BASICS_ROOT}/shields )
set(NRF52840               ${LORA_BASICS_ROOT}/smtc_hal/Nordic/nRF52840 )

set(FIRMWARE_UPDATE_ROOT   $ENV{ZEPHYR_BASE}/../SWTL001 )

set(CONNECTIVITY_TECHNOLOGY $ENV{ZEPHYR_BASE}/../connectivity-technology )

set(ZEPHYR_EXTRA_MODULES
        ${CONNECTIVITY_TECHNOLOGY}
        ${LORA_BASICS_ROOT}
        ${FIRMWARE_UPDATE_ROOT}
        $ENV{ZEPHYR_BASE}/../nrf
        $ENV{ZEPHYR_BASE}/../modules/hal/nordic
        $ENV{ZEPHYR_BASE}/../modules/hal/cmsis
        $ENV{ZEPHYR_BASE}/../modules/debug/segger
        $ENV{ZEPHYR_BASE}/../modules/crypto/tinycrypt
        $ENV{ZEPHYR_BASE}/../nrfxlib
  )

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(client)

zephyr_compile_definitions( REGION_US_915=1 )
zephyr_compile_definitions( LR11XX=1 )
zephyr_compile_definitions( LR11XX_TRANSCEIVER=1 )
zephyr_compile_definitions( MODEM_HAL_DBG_TRACE=0 )

# zephyr_compile_options( -Wextra -Werror )


#        src/gnss.c
#        src/gnss_scan.c

#        src/gpio_mapper.c

set(sources
        src/LoRa_task.c
        src/bt_task.c
        src/debug_gpio.c
        src/gatt_config_service.c
        src/led_task.c
        src/main.c
        src/rtc.c
        src/uart_task.c
        src/vars.c
        src/wdt_task.c
        src/stubs.c
)

# Main app
target_sources(app PRIVATE ${sources})

#
# _sigver.c is a generated file from the ver.sh script.  Don't need it for now.
#
# target_sources(app PRIVATE _sigver.c)

# target_sources_ifdef(CONFIG_SIGNETIK_APP_LORATEST app PRIVATE src/loratest_task.c)

zephyr_include_directories( ${APPS}/common )

zephyr_include_directories( ${GEOLOCATION_MIDDLEWARE}/common )
zephyr_include_directories( ${GEOLOCATION_MIDDLEWARE}/gnss/src )
zephyr_include_directories( ${GEOLOCATION_MIDDLEWARE}/wifi/src )


zephyr_include_directories( ${LORA_BASICS_MODEM} )
zephyr_include_directories( ${LORA_BASICS_MODEM}/smtc_modem_api )
zephyr_include_directories( ${LORA_BASICS_MODEM}/smtc_modem_core/radio_drivers/lr11xx_driver/src )
zephyr_include_directories( ${LORA_BASICS_MODEM}/smtc_modem_core/radio_planner/src )
zephyr_include_directories( ${LORA_BASICS_MODEM}/smtc_modem_core/smtc_ral/src )
zephyr_include_directories( ${LORA_BASICS_MODEM}/smtc_modem_core/smtc_ralf/src )
zephyr_include_directories( ${LORA_BASICS_MODEM}/smtc_modem_core/smtc_modem_crypto )
zephyr_include_directories( ${LORA_BASICS_MODEM}/smtc_modem_core/smtc_modem_crypto/smtc_secure_element )
zephyr_include_directories( ${LORA_BASICS_MODEM}/smtc_modem_hal )
zephyr_include_directories( ${LORA_BASICS_MODEM}/utilities/user_app/smtc_hal_l4 )

zephyr_include_directories( ${LR1MAC} )
zephyr_include_directories( ${LR1MAC}/src )
zephyr_include_directories( ${LR1MAC}/src/lr1mac_class_b )
zephyr_include_directories( ${LR1MAC}/src/lr1mac_class_c )
zephyr_include_directories( ${LR1MAC}/src/services )
zephyr_include_directories( ${LR1MAC}/src/smtc_real/src )

zephyr_include_directories( ${NRF52840}/inc )  # smtc_hal_flash_mapping.h

zephyr_include_directories( ${PRINTERS} )

zephyr_include_directories( ${SHIELDS} )
zephyr_include_directories( ${SHIELDS}/interface )
zephyr_include_directories( ${SHIELDS}/LR11XX/radio_drivers_hal )
zephyr_include_directories( ${SHIELDS}/LR11XX/smtc_shield_lr11xx/common/inc )
zephyr_include_directories( ${SHIELDS}/LR11XX/LR1110TRK1xKS/BSP/board )
zephyr_include_directories( ${SHIELDS}/LR11XX/LR1110TRK1xKS/BSP/peripherals/Leds )
zephyr_include_directories( ${SHIELDS}/LR11XX/LR1110TRK1xKS/BSP/peripherals/hall_effect )
zephyr_include_directories( ${SHIELDS}/LR11XX/LR1110TRK1xKS/BSP/peripherals/usr_button )

zephyr_include_directories( ${SMTC_HAL}/board )
zephyr_include_directories( ${SMTC_HAL}/inc )

zephyr_include_directories( ${SMTC_MODEM_CORE}/device_management )
zephyr_include_directories( ${SMTC_MODEM_CORE}/lorawan_api )
zephyr_include_directories( ${SMTC_MODEM_CORE}/modem_config )
zephyr_include_directories( ${SMTC_MODEM_CORE}/modem_services )
zephyr_include_directories( ${SMTC_MODEM_CORE}/modem_supervisor )

zephyr_include_directories( ${SMTC_MODEM_SERVICES} )
zephyr_include_directories( ${SMTC_MODEM_SERVICES}/headers )
zephyr_include_directories( ${SMTC_MODEM_SERVICES}/src )
zephyr_include_directories( ${SMTC_MODEM_SERVICES}/src/stream )

zephyr_include_directories( ${FIRMWARE_UPDATE_ROOT}/application/inc )
zephyr_include_directories( ${FIRMWARE_UPDATE_ROOT}/lr1110_modem_driver/src )
