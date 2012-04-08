# ------------------------------- #
# This is the root CMakeLists.txt #
# ------------------------------- #

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
CMAKE_POLICY(SET CMP0011 NEW) # :TODO: verify need of this policy

# Our helper scripts are located in this folder
LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/share/cmake/Modules)

INCLUDE(Enuminit)

ENUMINIT()

# ---------------------- #
# Optional Configuration #
# ---------------------- #

SET (BFG_DEPENDENCY_BUNDLE_PATH "" CACHE PATH "This is useful for Win32 users, the location of the dependency bundle may be hinted with this variable.")

# -------- #
# Includes #
# -------- #

INCLUDE(BFG)
INCLUDE(DependencyBundle)
INCLUDE(FindAllDependencies)

# --------------------#
# Project Declaration #
# --------------------#

BFG_INIT_PROJECT("Brute-Force Game Engine" "bfg")

IF(NOT BFG_ENGINE_INCLUDE_DIR)
	SET(BFG_ENGINE_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/code/engine/")
ENDIF(NOT BFG_ENGINE_INCLUDE_DIR)

# ---------------------- #
# Windows Specific Stuff #
# -----------------------#

IF(WIN32)
	# For Visual Studio 2008 or 2010
	IF(${MSVC_VERSION} EQUAL "1500" OR ${MSVC_VERSION} EQUAL "1600")
		ADD_DEFINITIONS(-D_WIN32_WINNT=0x0501)                 # Target: WinXP
		ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)             # Disable std::copy warnings
		ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)             # Disable sprintf warnings
		ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)                  # Use Boost.Log as shared lib
		ADD_DEFINITIONS(-DBOOST_ALL_DYN_LINK )                 # Use Boost as shared libs
		#ADD_DEFINITIONS(-DBOOST_LIB_DIAGNOSTIC)

		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251;4275") # Disable some DLL warnings
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")          # Enable MultiProcess compiling

		# Temporary solution for starting and debugging applications within Visual Studio.
		# A better way would be to use the install targets and to split engine and user builds.
		SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin CACHE PATH "Library output path")
		SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin CACHE PATH "Executable output path")
		SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin CACHE PATH "Executable output path")
	ENDIF(${MSVC_VERSION} EQUAL "1500" OR ${MSVC_VERSION} EQUAL "1600")
ENDIF(WIN32)

#DUMP_LOG_MESSAGE_TYPES()

IF(BFG_DEPENDENCY_BUNDLE_PATH)
	LOG_ACTION("Using dependency bundle at: ${BFG_DEPENDENCY_BUNDLE_PATH}")
	SETUP_BUNDLE_PATHS(${BFG_DEPENDENCY_BUNDLE_PATH})
ENDIF()

FIND_ALL_DEPENDENCIES()

# ------------------------------------- #
# Locally modified thirdparty libraries #
# ------------------------------------- #

SET(TINYXML_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/tinyxml")
SET(ODE_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/ODE_r1727/include")

# ------------------ #
# Output Directories #
# ------------------ #

IF(UNIX)
	SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/Linux/")
	SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib/")
	SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib/")
ENDIF(UNIX)

ADD_SUBDIRECTORY(thirdparty)
ADD_SUBDIRECTORY(code)

# ------------------------ #
# Documentation Generation #
# ------------------------ #

FIND_PACKAGE (Doxygen)
IF(DOXYGEN_FOUND)
	ADD_CUSTOM_TARGET (doc
		COMMAND doxygen ${PROJECT_SOURCE_DIR}/doc/Doxyfile
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/doc
	)
ENDIF(DOXYGEN_FOUND)

# ---------------#
# Install Shared #
# ---------------#

INSTALL(DIRECTORY share/cmake DESTINATION "share/bfg")
