cmake_minimum_required(VERSION 3.28)
project(dunderdb)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find pkg-config (used to locate libzmq install paths)
find_package(PkgConfig REQUIRED)

# Ask pkg-config for ZeroMQ details
pkg_check_modules(ZMQ REQUIRED libzmq)

# third party libraries
add_library(rapidjson INTERFACE)

target_include_directories(rapidjson
        INTERFACE
        ${CMAKE_SOURCE_DIR}/third_party/rapidjson/include
)

# util library
add_library(util INTERFACE)

target_include_directories(util INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/util/include
)
# network library
add_library(network
        network/src/ingestion_receiver.cpp
)

target_include_directories(network PUBLIC
        network/include
        ${ZMQ_INCLUDE_DIRS}
)

target_link_libraries(network PUBLIC
        ${ZMQ_LIBRARIES}
        util
        storage
)

target_link_directories(network PUBLIC
        ${ZMQ_LIBRARY_DIRS}
)

# schema library
add_library(schema
        schema/src/column.cpp
        schema/src/schema.cpp
)

target_include_directories(schema PUBLIC
    schema/include
)

target_link_libraries(schema PUBLIC
    rapidjson
        util
)

# indexes library
add_library(index
        index/src/service_index.cpp
        index/src/index_map.cpp
)

target_include_directories(index PUBLIC
        index/include
)


# storage library
add_library(storage
        storage/src/validator.cpp
        storage/src/buffer_map.cpp
        storage/src/service_buffer.cpp
        storage/src/disk_writer.cpp
)

target_include_directories(storage PUBLIC
        storage/include
)

target_link_libraries(storage PUBLIC
        schema
        index
)

# startup library
add_library(startup
        startup/src/index_loader.cpp
        startup/src/schema_loader.cpp
)

target_include_directories(startup PUBLIC
        startup/include
)

target_link_libraries(startup PUBLIC
        schema
        index
        util
)

# query library
add_library(query
        query/src/select_handler.cpp
)

target_include_directories(query PUBLIC
        query/include
)

target_link_libraries(query PUBLIC
        schema
        index
        storage
        rapidjson
)

#executable

add_executable(dunderdb main.cpp)

target_link_libraries(dunderdb
        PUBLIC
        network
        util
        schema
        storage
        index
        startup
)

add_executable(test test.cpp)

target_link_libraries(test
        PUBLIC
        network
        util
        schema
        storage
        startup
)