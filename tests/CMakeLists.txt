# The Qt SDK's first tests.
#
# What is worth testing here is the same thing that is worth testing in an
# adapter: a value in, a value out, with nothing behind it. The TLS fields are
# exactly that shape, and they are shared precisely so that every adapter reads
# them the same way - which is a claim a test can hold.
add_executable(tlsconfig_tests
    tlsconfig_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/qt/tlsconfig.cpp
)
target_include_directories(tlsconfig_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(tlsconfig_tests PRIVATE Qt6::Core phi::adapter-contract)

add_test(NAME tlsconfig_tests COMMAND tlsconfig_tests)
set_tests_properties(tlsconfig_tests PROPERTIES TIMEOUT 60)
