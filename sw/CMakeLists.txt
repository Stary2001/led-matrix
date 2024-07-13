cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)

project(led_matrix C CXX ASM)

# Initialize the SDK
set(PICO_BOARD pico_w)
pico_sdk_init()

add_subdirectory(picowota)
set(FREERTOS_KERNEL_PATH ../FreeRTOS-Kernel)
include(FreeRTOS_Kernel_import.cmake)

add_executable(pio_hub75)


function(picowota_retrieve_variable name hidden)
	if (DEFINED ENV{${name}} AND (NOT ${name}))
		set(${name} $ENV{${name}} PARENT_SCOPE)
		if (hidden)
			set(log_value "hidden")
		else()
			set(log_value "'$ENV{${name}}'")
		endif()
		message("Using ${name} from environment (${log_value})")
	endif()
endfunction()

picowota_retrieve_variable(PICOWOTA_WIFI_SSID false)
picowota_retrieve_variable(PICOWOTA_WIFI_PASS true)

if ((NOT PICOWOTA_WIFI_SSID) OR (NOT PICOWOTA_WIFI_PASS))
        message(FATAL_ERROR
		"WiFi SSID/Pass not set, please set PICOWOTA_WIFI_SSID/PICOWOTA_WIFI_PASS."
	)
endif ()

# TODO: This causes a full rebuild if they change, configure_file might
# be better.
target_compile_definitions(pio_hub75 PUBLIC WIFI_SSID=\"${PICOWOTA_WIFI_SSID}\")
target_compile_definitions(pio_hub75 PUBLIC WIFI_PASSWORD=\"${PICOWOTA_WIFI_PASS}\")
target_compile_definitions(pio_hub75 PUBLIC MG_ARCH=MG_ARCH_FREERTOS MG_ENABLE_LWIP=1 MG_ENABLE_PACKED_FS=1 HTTP_URL=\"http://0.0.0.0\")
pico_generate_pio_header(pio_hub75 ${CMAKE_CURRENT_LIST_DIR}/hub75.pio)

pico_enable_stdio_usb(pio_hub75 1)
pico_enable_stdio_uart(pio_hub75 0)

target_sources(pio_hub75 PRIVATE main.c hub75.c mongoose/net.c mongoose/packed_fs.c mongoose/mongoose.c mongoose/main.c)

target_include_directories(pio_hub75 PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/mongoose)

target_compile_definitions(pio_hub75 PRIVATE
	PICO_DEFAULT_UART_TX_PIN=28
	PICO_DEFAULT_UART_RX_PIN=29
)

target_link_libraries(pio_hub75 PRIVATE pico_stdlib hardware_pio pico_multicore pico_cyw43_arch_lwip_sys_freertos FreeRTOS-Kernel-Heap4)
pico_add_extra_outputs(pio_hub75)

picowota_build_combined(pio_hub75)
