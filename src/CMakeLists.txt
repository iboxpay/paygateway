PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ../proto/cup_payment.proto)

set(PAYGATEWAY_SOURCES main.cc socket.cc httpd.cc init_daemon.cc  base/base64.cpp ${PROTO_SRCS} ${PROTO_HDRS})

if (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNDEBUG")
endif (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")

if (EVHTP_DISABLE_SSL)
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DEVHTP_DISABLE_SSL")
endif()

add_executable(paygateway ${PAYGATEWAY_SOURCES})
target_link_libraries(paygateway ${PAYGATEWAY_EXTERNAL_LIBS})
INSTALL(TARGETS paygateway DESTINATION bin)
