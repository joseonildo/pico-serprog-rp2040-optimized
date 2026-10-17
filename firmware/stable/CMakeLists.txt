cmake_minimum_required(VERSION 3.13)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include(pico_sdk_import.cmake)
project(pico_serprog)

pico_sdk_init()

add_executable(pico_serprog)
target_sources(pico_serprog PRIVATE main.c usb_descriptors.c)
target_link_libraries(pico_serprog PRIVATE pico_stdlib hardware_spi
        hardware_dma tinyusb_device pico_usb_reset)
pico_add_extra_outputs(pico_serprog)


# AutoBOOT via USB vendor interface.
target_compile_definitions(pico_serprog PRIVATE
    PICO_ENABLE_USB_RESET_VIA_VENDOR_INTERFACE=1
    PICO_USB_RESET_INCLUDE_DEFAULT_APP_DRIVER_CB=1
    PICO_USB_RESET_SUPPORT_RESET_TO_BOOTSEL=1
    PICO_USB_RESET_SUPPORT_RESET_TO_FLASH_BOOT=1
    PICO_ENABLE_USB_RESET_VIA_BAUD_RATE=0
    PICO_USB_RESET_SUPPORT_MS_OS_20_DESCRIPTOR=0
)
