cmake_minimum_required(VERSION 3.30)

project(lsp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(deps/googletest)
enable_testing()

add_subdirectory(deps/json)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

# set(LSPP_INSTALL_DIR "~/LSP_install")

add_executable(main src/main.cpp src/Message.cpp src/Server.cpp src/ProtocolStructures.cpp src/textDocument.cpp)
target_include_directories(main
    PRIVATE
    include/
    deps/json/include/
)
target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic)

add_library(LSPP SHARED
    src/Message.cpp
    src/Server.cpp
    src/ProtocolStructures.cpp
    src/textDocument.cpp
)

set_target_properties(LSPP PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Propagate include directories to consumers
target_include_directories(LSPP
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps/json/include>
    $<INSTALL_INTERFACE:include/nlohmann>
)

install(
    TARGETS LSPP 
    EXPORT LSPPTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Install headers
install(
    DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

install(
    EXPORT LSPPTargets
    FILE LSPPTargets.cmake
    NAMESPACE LSPP::
    DESTINATION lib/cmake/LSPP)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "LSPPConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

install(FILES "LSPPConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/LSPPConfigVersion.cmake"
    DESTINATION lib/cmake/LSPP)

# Install JSON headers to separate directory to avoid conflicts
install(
    DIRECTORY deps/json/single_include/nlohmann/
    DESTINATION include/nlohmann
    FILES_MATCHING PATTERN "*.hpp"
)

# Tests
add_executable(test_message test/test_message.cpp src/Message.cpp)
target_include_directories(test_message PRIVATE include/ deps/json/include/)
target_link_libraries(test_message gtest gtest_main)
add_test(NAME test_message COMMAND test_message)

add_executable(test_server test/test_server.cpp)
target_link_libraries(test_server LSPP gtest gtest_main)
add_test(NAME test_server COMMAND test_server)

add_executable(test_json test/test_json.cpp src/ProtocolStructures.cpp)
target_include_directories(test_json PRIVATE include/ deps/json/include/)
target_link_libraries(test_json gtest gtest_main)
add_test(NAME test_json COMMAND test_json)

add_executable(test_textDocument test/test_textDocument.cpp src/textDocument.cpp)
target_include_directories(test_textDocument PRIVATE include/ deps/json/include/)
target_link_libraries(test_textDocument gtest gtest_main)
add_test(NAME test_textDocument COMMAND test_textDocument)