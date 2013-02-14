CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(smartcard-test-client CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../common/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../client/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/ SRCS)
#SET(SRCS "${CMAKE_CURRENT_SOURCE_DIR}/test-client-async.cpp")
SET(SRCS "${CMAKE_CURRENT_SOURCE_DIR}/test-client.cpp")

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs_test_client REQUIRED glib-2.0 gthread-2.0 gobject-2.0 security-server vconf dlog)

FOREACH(flag ${pkgs_test_client_CFLAGS})
	SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} ${flag}")
ENDFOREACH(flag)

MESSAGE("CHECK MODULE in ${PROJECT_NAME} ${pkgs_test_client_LDFLAGS}")

# this for NFC flag

SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} -pipe -fomit-frame-pointer -Wall -Wno-trigraphs  -fno-strict-aliasing -Wl,-zdefs -fvisibility=hidden")
SET(ARM_CXXFLAGS "${ARM_CXXLAGS} -mapcs -mno-sched-prolog -mabi=aapcs-linux -Uarm -fno-common -fPIC")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXXFLAGS}")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")
IF("${ARCH}" MATCHES "^arm.*")
	ADD_DEFINITIONS("-DTARGET")
	MESSAGE("add -DTARGET")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARM_CXXFLAGS}")
ENDIF()

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
#ADD_DEFINITIONS("-DSLP_DEBUG")
ADD_DEFINITIONS("-DLOG_TAG=\"SMARTCARD_SERVICE_TEST\"")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_test_client_LDFLAGS} "-L../common" "-lsmartcard-service-common" "-L../client" "-lsmartcard-service" "-pie -ldl")
#INSTALL(TARGETS ${PROJECT_NAME} DESTINATION bin)
