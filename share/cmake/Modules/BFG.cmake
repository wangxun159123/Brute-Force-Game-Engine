INCLUDE(FindPackageHandleStandardArgs)

# 
# These are the definitions for a ultimate build system
# 
#
# INPUT: Name of the project to initialize
# 
# sets internal project name
# ------------------------------------------------------------------------------
# Some message wrappers
# worst case ever
MACRO(LOG_FATAL _ERROR_MESSAGE)
	MESSAGE(FATAL_ERROR "[E] ${_ERROR_MESSAGE}")
ENDMACRO(LOG_FATAL _ERROR_MESSAGE)

# Just display a warning about it
MACRO(LOG_WARNING _text)
	MESSAGE(STATUS "[w] ${_text}")
ENDMACRO(LOG_WARNING _text)

# Something has been configured, changed, done
MACRO(LOG_ACTION _text)
	MESSAGE(STATUS "[a] ${_text}")
ENDMACRO(LOG_ACTION _text)

# Marks the begin of a section
MACRO(LOG_SECTION _text)
	MESSAGE(STATUS "[s] ${_text}")
ENDMACRO(LOG_SECTION _text)

# Marks the begin of a section
MACRO(LOG_STATUS _text)
	MESSAGE(STATUS "${_text}")
ENDMACRO(LOG_STATUS _text)

# Generic message
MACRO(LOG _text)
	MESSAGE("    ${_text}")
ENDMACRO(LOG _text)

# Just for your personal testing purposes
MACRO(DUMP_LOG_MESSAGE_TYPES)
	LOG_WARNING("This is a warning")
	LOG_ACTION("This changed a path value")
	LOG_SECTION("Start of a new section")
	LOG_STATUS("Generic status message")
	LOG ("Simple log message")
	LOG_FATAL("This is a fatal error")
ENDMACRO(DUMP_LOG_MESSAGE_TYPES)

# ------------------------------------------------------------------------------
# find package wrapper
# Public Macro
#
# Note: Whatever you give in for ${_name}, it will be converted to UPPER-CASE
#
# Input:
# _name		Name of the package
# _libName	Name of the library, to look for
# _incName	Name of the specific include file, to look for
#
#
# Output:
# For the following assume ${_name} == XYZ
# 
# XYZ_FOUND			true, if package is found
# XYZ_INCLUDE_DIR	the directory, where _incName is located
# XYZ_LIBRARIES	    the fully-qualified library (incl. proper path)
# XYZ				is set
#
#
MACRO(CHECK_PACKAGE _name _libName _incName)
	STRING(TOUPPER ${_name} _NAME)
	SET(${_NAME}_FOUND 0)
	SET(_libDirs ${ARGV3} ${BFG_SEARCH_LIBDIRS})
	SET(_incDirs ${ARGV4} ${BFG_SEARCH_INCDIRS})
	FIND_LIBRARY(${_NAME}_LIBRARIES ${_libName} HINTS ${_libDirs})
	FIND_PATH(${_NAME}_INCLUDE_DIR  ${_incName} HINTS ${_incDirs})
	# let cmake provided function do the rest
	FIND_PACKAGE_HANDLE_STANDARD_ARGS(${_name} DEFAULT_MSG ${_NAME}_LIBRARIES ${_NAME}_INCLUDE_DIR)
	SET(${_NAME})
ENDMACRO(CHECK_PACKAGE _name _libName _includeName)

# ------------------------------------------------------------------------------

MACRO(BFG_INIT_PROJECT _PROJECT_NAME _PROJECT_PREFIX)

	PROJECT(${_PROJECT_NAME})

	LOG_STATUS("-----------------------------------------------------")
	LOG_STATUS("Starting to configure Project '${_PROJECT_NAME}'")
	LOG_STATUS("-----------------------------------------------------")

	OPTION(RELEASE "Enable release build" OFF)
	IF(RELEASE)
		SET(CMAKE_BUILD_TYPE Release CACHE INTERNAL "Build type" FORCE)
	ELSE(RELEASE)
		SET(CMAKE_BUILD_TYPE Debug CACHE INTERNAL "Build type" FORCE)
	ENDIF(RELEASE)

	MARK_AS_ADVANCED(CMAKE_BUILD_TYPE)

	LOG_ACTION("CMAKE_BUILD_TYPE:  ${CMAKE_BUILD_TYPE}")

	IF(WIN32)
		# This Regex avoids problems in paths, mainly under windows
		STRING(REGEX REPLACE "\\\\" "/" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
	ENDIF(WIN32)

	SET(BFG_PROJECT_PREFIX ${_PROJECT_PREFIX})

	LOG_ACTION("Installation Prefix: ${CMAKE_INSTALL_PREFIX}")
	LOG_ACTION("File and Directory Prefix (BFG): ${BFG_PROJECT_PREFIX}")

	IF(WIN32)

		SET(BFG_PROJECT_BUNDLE TRUE)

	ELSE(WIN32)

		SET(BFG_PROJECT_BUNDLE FALSE)

		# See Makro CHECK_PACKAGE
		LIST(APPEND BFG_SEARCH_INCDIRS "/usr/local/include/" "/usr/include/")
		LIST(APPEND BFG_SEARCH_LIBDIRS "/usr/local/lib/" "/usr/lib")

	ENDIF(WIN32)

ENDMACRO(BFG_INIT_PROJECT _PROJECT_NAME)

# ------------------------------------------------------------------------------

MACRO(BFG_INIT_PACKAGE _PACKAGE_NAME _PACKAGE_VERSION)

	SET(BFG_PACKAGE_NAME "${BFG_PROJECT_PREFIX}${_PACKAGE_NAME}")

	# Version extraction
	STRING(REGEX REPLACE "[\\.\\-\\_]" ";" _tmp_components ${_PACKAGE_VERSION})
	LIST(GET _tmp_components 0 PACKAGE_VERSION_MAJOR)
	LIST(GET _tmp_components 1 PACKAGE_VERSION_MINOR)
	LIST(GET _tmp_components 2 PACKAGE_VERSION_PATCH)

	SET(_tmp_components)
	MARK_AS_ADVANCED(PACKAGE_VERSION_MAJOR)
	MARK_AS_ADVANCED(PACKAGE_VERSION_MINOR)
	MARK_AS_ADVANCED(PACKAGE_VERSION_PATCH)
  
	#LOG("Version: ${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_PATCH}")

	# ------------------------------------
	# COMPILE_<PACKAGE> is set as a compile define, so that sources may check if built
	STRING(TOUPPER "COMPILE_${_PACKAGE_NAME}" COMPILE_PACKAGE)
	# replace any disturbing characters
	STRING(REPLACE "-" "_" COMPILE_PACKAGE ${COMPILE_PACKAGE})
	# Set compiler define, that might come in handy
	ADD_DEFINITIONS( -D${COMPILE_PACKAGE}=1 )
	#LOG("## define ${COMPILE_PACKAGE}")

	# ------------------------------------
	# Now comes the part, where all the directories are handled
	IF(NOT BFG_PROJECT_PREFIX)
		FATAL_ERROR("Initialize project first, using: 'BFG_INIT_PROJECT'")
	ENDIF(NOT BFG_PROJECT_PREFIX)

	SET(BFG_BIN_DIR                 "bin")
	SET(BFG_LIB_DIR                 "lib")
	SET(BFG_INCLUDE_DIR             "include/${BFG_PROJECT_PREFIX}")
	SET(BFG_DATA_DIR                "share")
	SET(BFG_CONF_DIR                "etc")
	SET(BFG_LOCALE_DIR              "locale")
	SET(BFG_DOC_DIR                 "doc")

	# Package specifics
	SET(BFG_PACKAGE_BIN_DIR         "${BFG_BIN_DIR}/${_PACKAGE_NAME}")
	SET(BFG_PACKAGE_INCLUDE_DIR     "${BFG_INCLUDE_DIR}/${_PACKAGE_NAME}")
	SET(BFG_PACKAGE_DOC_DIR         "${BFG_DOC_DIR}/${_PACKAGE_NAME}")

#	LOG_STATUS("--------------------------------")
	LOG_STATUS("Configured package: ${BFG_PACKAGE_NAME}")
#	LOG_STATUS("------ Global definitions ------")
#	LOG_STATUS("  system : ${CMAKE_HOST_SYSTEM}")
#	LOG_STATUS("   build : ${CMAKE_BUILD_TYPE}")
#	LOG_STATUS("  prefix : ${CMAKE_INSTALL_PREFIX}")
#	LOG_STATUS("------ Project definitions ------")
#	LOG_STATUS("     bin : ${BFG_BIN_DIR}")
#	LOG_STATUS("     lib : ${BFG_LIB_DIR}")
#	LOG_STATUS(" include : ${BFG_INCLUDE_DIR}")
#	LOG_STATUS("    data : ${BFG_DATA_DIR}")
#	LOG_STATUS("    conf : ${BFG_CONF_DIR}")
#	LOG_STATUS("  locale : ${BFG_LOCALE_DIR}")
#	LOG_STATUS("     doc : ${BFG_DOC_DIR}")
#	LOG_STATUS("----- Package specific ----- ")
#	LOG_STATUS("     bin : ${BFG_PACKAGE_BIN_DIR}")
#	LOG_STATUS(" include : ${BFG_PACKAGE_INCLUDE_DIR}")
#	LOG_STATUS("     doc : ${BFG_PACKAGE_DOC_DIR}")

ENDMACRO(BFG_INIT_PACKAGE _PACKAGE_NAME _PACKAGE_VERSION)

