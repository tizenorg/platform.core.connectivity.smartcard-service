CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

MESSAGE("build smartcard-service")

SET(CMAKE_VERBOSE_MAKEFILE OFF)

IF(DEFINED USE_GDBUS)
	ADD_DEFINITIONS("-DUSE_GDBUS")

	# apply smack
	ADD_DEFINITIONS("-DUSER_SPACE_SMACK")

	IF(DEFINED USE_AUTOSTART)
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

	IF(DEFINED USE_AUTOSTART)
		ADD_DEFINITIONS("-DUSE_AUTOSTART")

		FIND_PROGRAM(DBUS_BINDING_TOOL NAMES dbus-binding-tool)
		EXEC_PROGRAM("${DBUS_BINDING_TOOL}" ARGS "--prefix=smartcard_service ${CMAKE_CURRENT_SOURCE_DIR}/common/smartcard-service.xml --mode=glib-server --output=${CMAKE_CURRENT_SOURCE_DIR}/common/include/smartcard-service-binding.h")
		EXEC_PROGRAM("${DBUS_BINDING_TOOL}" ARGS "--prefix=smartcard_service ${CMAKE_CURRENT_SOURCE_DIR}/common/smartcard-service.xml --mode=glib-client --output=${CMAKE_CURRENT_SOURCE_DIR}/common/include/smartcard-service-glue.h")
	ENDIF()
ENDIF()

ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(client)
ADD_SUBDIRECTORY(server)

IF(DEFINED TEST_CLIENT)
	ADD_SUBDIRECTORY(test-client)
ENDIF()

