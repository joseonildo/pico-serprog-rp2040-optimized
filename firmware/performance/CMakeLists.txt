cmake_minimum_required(VERSION 3.13)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include(pico_sdk_import.cmake)
project(pico_serprog)

pico_sdk_init()

add_executable(pico_serprog)
target_sources(pico_serprog PRIVATE main.c usb_descriptors.c)
target_link_libraries(pico_serprog PRIVATE pico_stdlib hardware_spi
        hardware_dma tinyusb_device)
pico_add_extra_outputs(pico_serprog)

# ============================================================
# EXP05 - RP2040 USB Bulk IN sem Errata 15 / UFRAME workaround
# ============================================================
#
# O Pico SDK adiciona:
#   PICO_RP2040_USB_DEVICE_UFRAME_FIX=1
# através de tinyusb_device_unmarked.
#
# Para este experimento removemos somente essa definição da
# interface e aplicamos =0 ao firmware final.
#
# FAST_IRQ permanece =1 para que a única variável experimental
# seja o UFRAME workaround.

if(TARGET tinyusb_device_unmarked)
    get_target_property(_exp05_defs
        tinyusb_device_unmarked
        INTERFACE_COMPILE_DEFINITIONS
    )

    if(_exp05_defs)
        list(REMOVE_ITEM _exp05_defs
            PICO_RP2040_USB_DEVICE_UFRAME_FIX=1
        )

        set_target_properties(tinyusb_device_unmarked PROPERTIES
            INTERFACE_COMPILE_DEFINITIONS "${_exp05_defs}"
        )
    endif()
else()
    message(FATAL_ERROR
        "EXP05: target tinyusb_device_unmarked não encontrado"
    )
endif()

target_compile_definitions(pico_serprog PRIVATE
    PICO_RP2040_USB_DEVICE_UFRAME_FIX=0
    PICO_RP2040_USB_FAST_IRQ=1
)
