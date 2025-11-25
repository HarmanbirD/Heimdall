cmake_minimum_required(VERSION 3.24)

project(Heimdall
        VERSION 0.2.1
        DESCRIPTION ""
        LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(SANITIZE TRUE)

set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

set(SOURCE_LIST
    ${SOURCE_DIR}/main.c
    ${SOURCE_DIR}/daemonize.c
    ${SOURCE_DIR}/hashing.c
    ${SOURCE_DIR}/utils.c
    ${SOURCE_DIR}/logging.c
    ${SOURCE_DIR}/daemonize_control.c
    ${SOURCE_DIR}/parser.c
)

add_compile_definitions(
    _POSIX_C_SOURCE=200809L
    _XOPEN_SOURCE=700
    _GNU_SOURCE
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_compile_definitions(_DARWIN_C_SOURCE)
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    add_compile_definitions(__BSD_VISIBLE)
endif ()

include_directories(${INCLUDE_DIR})

add_compile_options("-Wall"
        "-Wextra"
        "-Wpedantic"
        "-Wshadow"
        "-Wstrict-overflow=4"
        "-Wswitch-default"
        "-Wswitch-enum"
        "-Wunused"
        "-Wunused-macros"
        "-Wdate-time"
        "-Winvalid-pch"
        "-Wmissing-declarations"
        "-Wmissing-include-dirs"
        "-Wmissing-prototypes"
        "-Wstrict-prototypes"
        "-Wundef"
        "-Wnull-dereference"
        "-Wstack-protector"
        "-Wdouble-promotion"
        "-Wvla"
        "-Walloca"
        "-Woverlength-strings"
        "-Wdisabled-optimization"
        "-Winline"
        "-Wcast-qual"
        "-Wfloat-equal"
        "-Wformat=2"
        "-Wfree-nonheap-object"
        "-Wshift-overflow"
        "-Wwrite-strings")

if(${SANITIZE})
    add_compile_options(
        -fsanitize=address
        -fsanitize=undefined
        -fsanitize-address-use-after-scope
        -fstack-protector-all
        -fdelete-null-pointer-checks
        -fno-omit-frame-pointer
    )
    if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        add_compile_options(-fsanitize=leak)
    endif()
    add_link_options(
        -fsanitize=address
        -fsanitize=bounds
    )
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenSSL REQUIRED)
if (NOT OpenSSL_FOUND)
    message(FATAL_ERROR "OpenSSL not found! Install openssl-devel or libssl-dev.")
else()
    message(STATUS "Found OpenSSL version: ${OpenSSL_VERSION}")
endif()

add_executable(Heimdall ${SOURCE_LIST})

target_include_directories(Heimdall PRIVATE ${INCLUDE_DIR})
target_link_libraries(Heimdall PRIVATE OpenSSL::Crypto pthread)

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_include_directories(Heimdall PRIVATE /usr/include)
endif ()

set_target_properties(Heimdall PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME "Heimdall"
)

install(TARGETS Heimdall DESTINATION bin)

set(SYSTEMD_UNIT_DIR "/etc/systemd/system")
set(SERVICE_FILE "${CMAKE_SOURCE_DIR}/system/heimdall.service")

if(EXISTS ${SERVICE_FILE})
    install(FILES ${SERVICE_FILE} DESTINATION ${SYSTEMD_UNIT_DIR})
    message(STATUS "Systemd service file will be installed to ${SYSTEMD_UNIT_DIR}")
else()
    message(WARNING "Systemd service file not found at ${SERVICE_FILE}")
endif()

install(CODE "
    execute_process(COMMAND systemctl daemon-reload)
    execute_process(COMMAND systemctl try-restart heimdall.service)
")
