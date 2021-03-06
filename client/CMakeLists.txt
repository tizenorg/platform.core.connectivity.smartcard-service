CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(smartcard-service CXX)

SET(LIB_NAME "smartcard-service")
SET(VERSION_MAJOR 1)
SET(VERSION ${VERSION_MAJOR}.0.0)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../common/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR} SRCS)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
        SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs_client REQUIRED gthread-2.0 dlog)

MESSAGE("${LIB_NAME} ld flag : ${pkgs_client_LDFLAGS}")

FOREACH(flag ${pkgs_client_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

FOREACH(flag ${pkgs_client_CFLAGS})
	SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} ${flag}")
ENDFOREACH(flag)


#SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -finstrument-functions")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
#SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")
#SET(CMAKE_C_FLAGS_RELEASE "-O2")

#SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} -finstrument-functions")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXXFLAGS}")
#SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
#SET(CMAKE_CXX_FLAGS_RELEASE "-O2")

FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")
IF("${ARCH}" STREQUAL "arm")
	ADD_DEFINITIONS("-DTARGET")
	MESSAGE("add -DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")
MESSAGE("CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DSLP_DEBUG")

ADD_DEFINITIONS("-DLOG_TAG=\"SCARD_CLIENT\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRCS})

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES VERSION ${VERSION})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_client_LDFLAGS})

SET(EXPORT_HEADER 
	../common/include/smartcard-types.h
	../common/include/ByteArray.h
	../common/include/Debug.h
	../common/include/Synchronous.h
	../common/include/APDUHelper.h
	../common/include/Channel.h
	../common/include/Serializable.h
	../common/include/SEServiceHelper.h
	../common/include/ReaderHelper.h
	../common/include/SessionHelper.h
#	../common/include/FCI.h
#	../common/include/
	include/SEServiceListener.h
	include/SEService.h
	include/Reader.h
	include/Session.h
	include/ClientChannel.h
#	include/
)

#CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${LIB_NAME}.pc.in ${CMAKE_CURRENT_SOURCE_DIR}/${LIB_NAME}.pc)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${LIB_NAME}.pc DESTINATION lib/pkgconfig)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION lib)
FOREACH(hfile ${EXPORT_HEADER})
        INSTALL(FILES ${hfile} DESTINATION include/${LIB_NAME})
ENDFOREACH(hfile)
