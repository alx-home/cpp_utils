cmake_minimum_required(VERSION 3.20)

project(alx-home_cpp_utils LANGUAGES CXX)

win32_library(TARGET_NAME alx-home_cpp_utils
  FILES
    src/String.cpp
    src/Nonce.cpp
    src/MessageQueue.cpp
    src/encryption.cpp
)
add_library(alx-home::cpp_utils ALIAS alx-home_cpp_utils)
target_link_libraries(alx-home_cpp_utils PRIVATE crypt32)

execute_process(
    COMMAND powershell -Command "$bytes = New-Object byte[] 32; [System.Security.Cryptography.RandomNumberGenerator]::Create().GetBytes($bytes); [BitConverter]::ToString($bytes).Replace('-', '').ToLower()"
    OUTPUT_VARIABLE UUID
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "Generated UUID: ${UUID}")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uuid_config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/uuid/uuid.h
    @ONLY
)

target_include_directories(alx-home_cpp_utils PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}/uuid/
)
