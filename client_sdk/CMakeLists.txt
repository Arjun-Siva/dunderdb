add_library(dunderdb-client
        src/field.cpp
        src/message.cpp
        src/send_request.cpp
        src/reply.cpp
        src/query_request.cpp
        src/client.cpp
)

target_include_directories(dunderdb-client
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)