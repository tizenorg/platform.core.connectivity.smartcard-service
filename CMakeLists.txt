CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

MESSAGE("build smartcard-service")

SET(CMAKE_VERBOSE_MAKEFILE OFF)

IF("${USE_GDBUS}" STREQUAL "1")
	ADD_DEFINITIONS("-DUSE_GDBUS")
	
	IF("${USE_AUTOSTART}" STREQUAL "1")
		ADD_DEFINITIONS("-DUSE_AUTOSTART")
	ENDIF()

	FIND_PROGRAM(GDBUS_CODEGEN NAMES gdbus-codegen)
	EXEC_PROGRAM(${GDBUS_CODEGEN} ARGS
		" \\
		--generate-c-code ${CMAKE_CURRENT_SOURCE_DIR}/common/smartcard-service-gdbus \\
		--c-namespace SmartcardService\\
		--interface-prefix org.tizen.SmartcardService. \\
		${CMAKE_CURRENT_SOURCE_DIR}/common/smartcard-service-gdbus.xml \\
		")
ELSE()
	ADD_DEFINITIONS("-DCLIENT_IPC_THREAD")
	ADD_DEFINITIONS("-DUSE_IPC_EPOLL")

	IF("${USE_AUTOSTART}" STREQUAL "1")
		ADD_DEFINITIONS("-DUSE_AUTOSTART")

		FIND_PROGRAM(DBUS_BINDING_TOOL NAMES dbus-binding-tool)
		EXEC_PROGRAM("${DBUS_BINDING_TOOL}" ARGS "--prefix=smartcard_service ${CMAKE_CURRENT_SOURCE_DIR}/common/smartcard-service.xml --mode=glib-server --output=${CMAKE_CURRENT_SOURCE_DIR}/common/include/smartcard-service-binding.h")
		EXEC_PROGRAM("${DBUS_BINDING_TOOL}" ARGS "--prefix=smartcard_service ${CMAKE_CURRENT_SOURCE_DIR}/common/smartcard-service.xml --mode=glib-client --output=${CMAKE_CURRENT_SOURCE_DIR}/common/include/smartcard-service-glue.h")
	ENDIF()
ENDIF()

ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(client)
ADD_SUBDIRECTORY(server)

IF("${TEST_CLIENT}" STREQUAL "1")
	ADD_SUBDIRECTORY(test-client)
ENDIF()

