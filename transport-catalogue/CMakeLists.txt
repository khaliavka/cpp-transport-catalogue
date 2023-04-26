cmake_minimum_required(VERSION 3.11)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SOURCES PROTO_HEADERS transport_catalogue.proto)

set(CATALOGUE_SOURCES domain.cc
                      domain.h
                      geo.cc
                      geo.h
                      graph.h
                      json_builder.cc
                      json_builder.h
                      json_reader.cc
                      json_reader.h
                      json.cc
                      json.h
                      main.cc
                      map_renderer.cc
                      map_renderer.h
                      ranges.h
                      request_handler.cc
                      request_handler.h
                      router.h
                      serialization.cc
                      serialization.h
                      svg.cc
                      svg.h
                      transport_catalogue.cc
                      transport_catalogue.h
                      transport_router.cc
                      transport_router.h)

add_executable(transport_catalogue ${PROTO_SOURCES} ${PROTO_HEADERS}
                                    ${CATALOGUE_SOURCES})

include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)