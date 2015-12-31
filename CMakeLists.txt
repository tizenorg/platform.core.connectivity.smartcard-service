CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

MESSAGE("build smartcard-service")

SET(CMAKE_VERBOSE_MAKEFILE OFF)

IF(DEFINED USE_AUTOSTART)
	ADD_DEFINITIONS("-DUSE_AUTOSTART")
ENDIF()

# make string from entire content of byte array
ADD_DEFINITIONS("-DTO_STRING_ALL")
ADD_DEFINITIONS("-DUSE_CYNARA")

SET(GC_SECTIONS_FLAGS "-fdata-sections -ffunction-sections -Wl,--gc-sections")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GC_SECTIONS_FLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GC_SECTIONS_FLAGS}")

FIND_PROGRAM(GDBUS_CODEGEN NAMES gdbus-codegen)
EXEC_PROGRAM(${GDBUS_CODEGEN} ARGS
	" \\
	--generate-c-code ${CMAKE_CURRENT_SOURCE_DIR}/common/smartcard-service-gdbus \\
	--c-namespace SmartcardService\\
	--interface-prefix org.tizen.SmartcardService. \\
	${CMAKE_CURRENT_SOURCE_DIR}/common/smartcard-service-gdbus.xml \\
	")

ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(client)
ADD_SUBDIRECTORY(server)

IF(DEFINED TEST_CLIENT)
	ADD_SUBDIRECTORY(test-client)
ENDIF()

