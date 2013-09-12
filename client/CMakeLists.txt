INCLUDE_DIRECTORIES(include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/common)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/common/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/server/include)

AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR} SRCS)

pkg_check_modules(pkgs_client REQUIRED  glib-2.0 dlog)
FOREACH(flag ${pkgs_client_CFLAGS})
	SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} ${flag}")
ENDFOREACH(flag)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXXFLAGS}")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DLOG_TAG=\"SMARTCARD_SERVICE_CLIENT\"")

ADD_LIBRARY(${CLIENT_LIB} SHARED ${SRCS})
SET_TARGET_PROPERTIES(${CLIENT_LIB} PROPERTIES VERSION ${FULLVER} SOVERSION ${MAJORVER})
TARGET_LINK_LIBRARIES(${CLIENT_LIB} ${pkgs_client_LDFLAGS})

SET(EXPORT_HEADER
	../common/include/smartcard-types.h
	../common/include/ByteArray.h
	../common/include/Debug.h
	../common/include/Exception.h
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
	include/smartcard-service.h
#	include/
)

CONFIGURE_FILE(${CLIENT_LIB}.pc.in ${CLIENT_LIB}.pc @ONLY)

INSTALL(FILES ${CLIENT_LIB}.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
INSTALL(TARGETS ${CLIENT_LIB} DESTINATION ${LIB_INSTALL_DIR})

FOREACH(hfile ${EXPORT_HEADER})
	INSTALL(FILES ${hfile} DESTINATION include/${CLIENT_LIB})
ENDFOREACH(hfile)
