
include_directories(../mc2tcpd)

add_executable(
        unit_tests
        ../mc2tcpd/ipaddress.h
        ../mc2tcpd/ipaddress.cpp
        ipaddress_test.cpp
        test.cpp
)


target_link_libraries(
        unit_tests
        gtest_main
)

add_test(
        NAME
        unit
        COMMAND
        ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/unit_tests
)
