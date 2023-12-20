cmake_minimum_required(VERSION 3.27.1)

project(victo VERSION 1.0.1)

find_package(OpenSSL REQUIRED)

message(STATUS "OpenSSL: Version ${OPENSSL_VERSION}")
message(STATUS "OpenSSL: include dir at ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OpenSSL: libraries at ${OPENSSL_LIBRARIES} ${OPENSSL_SSL_LIBRARIES}")

include_directories(${OPENSSL_INCLUDE_DIR})

add_library(${PROJECT_NAME} SHARED main.c)
add_executable(${PROJECT_NAME}-exe main.c)

add_subdirectory(src)
add_subdirectory(src/engine)
add_subdirectory(src/servers)
add_subdirectory(src/utils)

# target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES} ${OPENSSL_INCLUDE_DIR})
# target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_INCLUDE_DIR})

# include_directories(${OPENSSL_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} src engine servers utils m ${OPENSSL_LIBRARIES} ${OPENSSL_SSL_LIBRARIES})
target_link_libraries(${PROJECT_NAME}-exe src engine servers utils m ${OPENSSL_LIBRARIES} ${OPENSSL_SSL_LIBRARIES})





