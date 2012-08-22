CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(smartcard-daemon CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../common/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/ SRCS)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs_server REQUIRED glib-2.0 gobject-2.0 security-server vconf dlog)

FOREACH(flag ${pkgs_server_CFLAGS})
	SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} ${flag}")
ENDFOREACH(flag)

MESSAGE("CHECK MODULE in ${PROJECT_NAME} ${pkgs_server_LDFLAGS}")

# this for NFC flag

SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} -pipe -fomit-frame-pointer -Wall -Wno-trigraphs  -fno-strict-aliasing -Wl,-zdefs -fvisibility=hidden")

SET(ARM_CXXFLAGS "${ARM_CXXLAGS} -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -msoft-float -Uarm -fno-common -fpic")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXXFLAGS}")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")

FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")
IF("${ARCH}" MATCHES "^arm.*")
	ADD_DEFINITIONS("-DTARGET")
	MESSAGE("add -DTARGET")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARM_CXXFLAGS}")
ENDIF()

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DLOG_TAG=\"SCARD_SERVER\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_server_LDFLAGS} "-L../common" "-lsmartcard-service-common" "-pie -ldl")

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION bin)
