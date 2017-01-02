cmake_minimum_required(VERSION 2.6)

find_library(CMOCKA_LIBRARY NAMES cmocka)
if (NOT CMOCKA_LIBRARY)
    message(FATAL_ERROR "libcmocka unit test framework mising")
endif(NOT CMOCKA_LIBRARY)

find_package(PkgConfig)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)

SET(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library (tetrapol
    addr.c
    bch.c
    bit_utils.c
    cch.c
    data_frame.c
    frame.c
    frame_json.c
    hdlc_frame.c
    link.c
    log.c
    lsdu_cd.c
    lsdu_vch.c
    misc.c
    msg_coding.c
    phys_ch.c
    pch.c
    rch.c
    sdch.c
    tch.c
    terminal.c
    tetrapol.c
    tp_timer.c
    tpdu.c
    tsdu.c
    tsdu_json.c
    tsdu_print.c
    tetrapol/addr.h
    tetrapol/bch.h
    tetrapol/bit_utils.h
    tetrapol/cch.h
    tetrapol/data_frame.h
    tetrapol/hdlc_frame.h
    tetrapol/frame.h
    tetrapol/frame_json.h
    tetrapol/link.h
    tetrapol/log.h
    tetrapol/lsdu_vch.h
    tetrapol/misc.h
    tetrapol/msg_coding.h
    tetrapol/phys_ch.h
    tetrapol/pch.h
    tetrapol/rch.h
    tetrapol/sdch.h
    tetrapol/system_config.h
    tetrapol/tch.h
    tetrapol/tetrapol.h
    tetrapol/terminal.h
    tetrapol/tp_timer.h
    tetrapol/tpdu.h
    tetrapol/tsdu_json.h
    tetrapol/tsdu_print.h
)
target_link_libraries (tetrapol ${GLIB2_LIBRARIES})
include_directories(${GLIB2_INCLUDE_DIRS})

add_executable (test_data_frame
    bit_utils.c
    frame.c
    log.c
    test_data_frame.c)
target_link_libraries (test_data_frame ${CMOCKA_LIBRARY})

add_executable (test_frame
    bit_utils.c
    log.c
    test_frame.c)
target_link_libraries (test_frame ${CMOCKA_LIBRARY})

add_executable (test_bit_utils
    test_bit_utils.c)
target_link_libraries (test_bit_utils ${CMOCKA_LIBRARY})

add_executable (test_timer
    log.c
    test_tp_timer.c)
target_link_libraries (test_timer ${CMOCKA_LIBRARY})

add_test(test_data_frame ${CMAKE_CURRENT_BINARY_DIR}/test_data_frame)
add_test(test_frame ${CMAKE_CURRENT_BINARY_DIR}/test_frame)
add_test(test_bit_utils ${CMAKE_CURRENT_BINARY_DIR}/test_bit_utils)
add_test(test_timer ${CMAKE_CURRENT_BINARY_DIR}/test_timer)
