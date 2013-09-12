INCLUDE_DIRECTORIES(include)

FIND_PROGRAM(GDBUS_CODEGEN NAMES gdbus-codegen)
EXEC_PROGRAM(${GDBUS_CODEGEN} ARGS
	" --generate-c-code ${CMAKE_CURRENT_SOURCE_DIR}/smartcard-service-gdbus"
	" --c-namespace SmartcardService"
	" --interface-prefix org.tizen.SmartcardService."
	" ${CMAKE_CURRENT_SOURCE_DIR}/smartcard-service-gdbus.xml"
)

AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR} COMMON_SRCS)

pkg_check_modules(pkgs_common REQUIRED glib-2.0 gio-unix-2.0 dlog aul libssl pkgmgr pkgmgr-info)
FOREACH(flag ${pkgs_common_CFLAGS})
	SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} ${flag}")
ENDFOREACH(flag)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXXFLAGS}")

ADD_DEFINITIONS("-DLOG_TAG=\"SMARTCARD_SERVICE_COMMON\"")

SET(CMAKE_C_FLAGS "${EXTRA_CXXFLAGS}")

ADD_LIBRARY(${COMMON_LIB} SHARED ${COMMON_SRCS})
SET_TARGET_PROPERTIES(${COMMON_LIB} PROPERTIES VERSION ${FULLVER} SOVERSION ${MAJORVER})
TARGET_LINK_LIBRARIES(${COMMON_LIB} ${pkgs_common_LDFLAGS})

SET(EXPORT_HEADER
	include/smartcard-types.h
	include/Debug.h
	include/Exception.h
	include/ByteArray.h
	include/Lock.h
	include/Synchronous.h
	include/TerminalInterface.h
	include/Terminal.h
	include/SignatureHelper.h
)

CONFIGURE_FILE(${COMMON_LIB}.pc.in ${COMMON_LIB}.pc @ONLY)
INSTALL(FILES ${COMMON_LIB}.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
INSTALL(TARGETS ${COMMON_LIB} DESTINATION ${LIB_INSTALL_DIR})
FOREACH(hfile ${EXPORT_HEADER})
	INSTALL(FILES ${hfile} DESTINATION include/${COMMON_LIB})
ENDFOREACH(hfile)
