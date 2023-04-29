cmake_minimum_required(VERSION 3.11)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

function(PROTOBUF_GENERATE_CPP_EXPTL SRCS HDRS)
  if(NOT ARGN)
    message(SEND_ERROR "Error: PROTOBUF_GENERATE_CPP_FLAG_EXPTL() called without any proto files")
    return()
  endif()

  if(PROTOBUF_GENERATE_CPP_APPEND_PATH) # This variable is common for all types of output.
    # Create an include path for each file specified
    foreach(FIL ${ARGN})
      get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
      get_filename_component(ABS_PATH ${ABS_FIL} PATH)
      list(FIND _protobuf_include_path ${ABS_PATH} _contains_already)
      if(${_contains_already} EQUAL -1)
          list(APPEND _protobuf_include_path -I ${ABS_PATH})
      endif()
    endforeach()
  else()
    set(_protobuf_include_path -I ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  if(DEFINED PROTOBUF_IMPORT_DIRS)
    foreach(DIR ${Protobuf_IMPORT_DIRS})
      get_filename_component(ABS_PATH ${DIR} ABSOLUTE)
      list(FIND _protobuf_include_path ${ABS_PATH} _contains_already)
      if(${_contains_already} EQUAL -1)
          list(APPEND _protobuf_include_path -I ${ABS_PATH})
      endif()
    endforeach()
  endif()

  set(${SRCS})
  set(${HDRS})
  foreach(FIL ${ARGN})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)

    list(APPEND ${SRCS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.cc")
    list(APPEND ${HDRS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.h")

    add_custom_command(
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.cc"
             "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.h"
      COMMAND  ${Protobuf_PROTOC_EXECUTABLE}
      ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
           --experimental_allow_proto3_optional
           ${_protobuf_include_path} ${ABS_FIL}
      DEPENDS ${ABS_FIL} ${Protobuf_PROTOC_EXECUTABLE}
      COMMENT "Running C++ protocol buffer compiler on ${FIL} with --experimental flag"
      VERBATIM)
  endforeach()

  set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
  set(${SRCS} ${${SRCS}} PARENT_SCOPE)
  set(${HDRS} ${${HDRS}} PARENT_SCOPE)
endfunction()

# protobuf_generate_cpp(PROTO_SOURCES PROTO_HEADERS 
#                                                   graph.proto
#                                                   map_renderer.proto
#                                                   svg.proto
#                                                   transport_catalogue.proto
#                                                   transport_router.proto)

protobuf_generate_cpp_exptl(PROTO_SOURCES PROTO_HEADERS 
                                                  graph.proto
                                                  map_renderer.proto
                                                  svg.proto
                                                  transport_catalogue.proto
                                                  transport_router.proto)

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