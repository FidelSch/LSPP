cmake_minimum_required(VERSION 3.5)

project(lsp)

add_subdirectory(lib/googletest)
include_directories(lib/googletest/include)
link_directories(lib/googletest/lib)
enable_testing()

add_subdirectory(lib/json)
include_directories(lib/json/include)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

include_directories(include/)
link_directories(include/)



add_executable(main src/main.cpp src/Message.cpp src/Server.cpp src/ProtocolStructures.cpp )


add_executable(test_message test/test_message.cpp src/Message.cpp)
target_link_libraries(test_message gtest gtest_main)
add_test(NAME test_message 
    COMMAND test_message
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build
)

add_executable(test_server test/test_server.cpp src/Server.cpp src/Message.cpp src/ProtocolStructures.cpp )
target_link_libraries(test_server gtest gtest_main)
add_test(NAME test_server 
    COMMAND test_server
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build
)

add_executable(test_json test/test_json.cpp src/ProtocolStructures.cpp)
target_link_libraries(test_json gtest gtest_main)
add_test(NAME test_json 
    COMMAND test_json
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build
)

add_executable(test_textDocument test/test_textDocument.cpp src/ProtocolStructures.cpp)
target_link_libraries(test_textDocument gtest gtest_main)
add_test(NAME test_textDocument 
    COMMAND test_textDocument
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build 
)