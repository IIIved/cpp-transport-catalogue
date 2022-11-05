cmake_minimum_required(VERSION 3.10)


project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)


find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)


protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto svg.proto map_renderer.proto graph.proto transport_router.proto )

set(FILES main.cpp domain.h domain.cpp geo.h graph.h graph.proto json.h json.cpp map_renderer.h map_renderer.cpp map_renderer.proto  ranges.h router.h svg.h svg.cpp svg.proto transport_catalogue.h transport_catalogue.cpp transport_catalogue.proto transport_router.h transport_router.cpp transport_router.proto json_builder.cpp json_builder.h json_reader.cpp json_reader.h serialization.h serialization.cpp request_handler.h request_handler.cpp)


add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${FILES})
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})


string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)