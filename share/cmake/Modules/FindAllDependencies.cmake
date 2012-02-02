MACRO(FIND_ALL_DEPENDENCIES)

	# ----- #
	# Boost #
	# ----- #

	FIND_PACKAGE(Boost 1.47 REQUIRED date_time thread filesystem program_options system regex)

	IF(Boost_FOUND)
		LOG("Found boost ${Boost_VERSION} in ${Boost_LIBRARY_DIRS} and ${Boost_INCLUDE_DIR}")
	ELSE(Boost_FOUND)
		LOG_FATAL("You need boost in order to build this project")
	ENDIF(Boost_FOUND)

	# --------- #
	# Boost.Log #
	# --------- #

	FIND_PACKAGE(BoostLog REQUIRED)

	# -------#
	# OpenAL #
	# -------#

	FIND_PACKAGE(OpenAL REQUIRED)

	# ---------- #
	# libSndFile #
	# ---------- #

	IF(WIN32)
		CHECK_PACKAGE(LibSndFile "libsndfile/lib/libsndfile-1.lib" "libsndfile/include/sndfile.h")
		SET(LIBSNDFILE_LIBRARY ${LIBSNDFILE_LIBRARIES})
		SET(LIBSNDFILE_INCLUDE_DIR "${LIBSNDFILE_INCLUDE_DIR}/libsndfile/include")
	ELSEIF(UNIX)
		FIND_PACKAGE(LibSndFile REQUIRED)
	ELSEIF (APPLE)
		LOG_FATAL("Apple support is not implemented yet")
	ELSE (WIN32)
		LOG_FATAL("Unknown error or a not supported OS")
	ENDIF(WIN32)

	# ---- #
	# OGRE #
	# ---- #

	IF(WIN32)
		SET(OGRE_PATH "ogre_v1-7-3")
		SET(OGRE_INCLUDE_FILE "${OGRE_PATH}/OgreMain/include/ogre.h")
		SET(OGRE_LIB_RELEASE "${OGRE_PATH}/lib/Release/ogremain.lib")
		SET(OGRE_LIB_DEBUG   "${OGRE_PATH}/lib/Debug/ogremain_d.lib")

		IF(CMAKE_CONFIGURATION_TYPES)
			CHECK_PACKAGE(OGRE_DEBUG ${OGRE_LIB_DEBUG} ${OGRE_INCLUDE_FILE})
			CHECK_PACKAGE(OGRE_RELEASE ${OGRE_LIB_RELEASE} ${OGRE_INCLUDE_FILE})

			IF(OGRE_DEBUG_FOUND AND OGRE_RELEASE_FOUND)
				SET(OGRE_LIBRARIES debug ${OGRE_DEBUG_LIBRARIES}
				               optimized ${OGRE_RELEASE_LIBRARIES})

				SET(OGRE_FOUND ${OGRE_DEBUG_FOUND})
				SET(OGRE_INCLUDE_DIR ${OGRE_DEBUG_INCLUDE_DIR})

			ELSE(OGRE_DEBUG_FOUND AND OGRE_RELEASE_FOUND)
				LOG_FATAL("Need both ogre libraries for multi-target solution")
			ENDIF(OGRE_DEBUG_FOUND AND OGRE_RELEASE_FOUND)
		ELSE(CMAKE_CONFIGURATION_TYPES)

			IF(RELEASE)
				SET(OGRE_MAIN_LIB ${OGRE_LIB_RELEASE})
			ELSE(RELEASE)
				SET(OGRE_MAIN_LIB ${OGRE_LIB_DEBUG})
			ENDIF(RELEASE)

		# Check for either package
		CHECK_PACKAGE(OGRE ${OGRE_MAIN_LIB} ${OGRE_INCLUDE_FILE})

		ENDIF(CMAKE_CONFIGURATION_TYPES)

		SET(OGRE_INCLUDE_DIR "${OGRE_INCLUDE_DIR}/${OGRE_PATH}/OgreMain/include")

	ELSEIF(UNIX)

		CHECK_PACKAGE(OGRE "libOgreMain.so" "OGRE/Ogre.h")
		SET(OGRE_INCLUDE_DIR ${OGRE_INCLUDE_DIR}/OGRE)

	ELSEIF (APPLE)

		MESSAGE(FATAL_ERROR "Apple support is not implemented yet")

	ELSE (WIN32)

		MESSAGE(FATAL_ERROR "Unknown error or a not supported OS")

	ENDIF(WIN32)

	IF(NOT OGRE_FOUND)
		LOG_FATAL("Ogre could not be found")
	ENDIF(NOT OGRE_FOUND)

	# --- #
	# OIS #
	# --- #

	IF(WIN32)
		SET(OIS_BUNDLE_PATH "${OGRE_PATH}/Dependencies")
		SET(OIS_DEBUG_LIB "${OIS_BUNDLE_PATH}/lib/Debug/OIS_d.lib")
		SET(OIS_RELEASE_LIB "${OIS_BUNDLE_PATH}/lib/Release/OIS.lib")
		SET(OIS_INCLUDE_FILE "${OIS_BUNDLE_PATH}/include/OIS/OIS.h")
		IF(CMAKE_CONFIGURATION_TYPES)
			CHECK_PACKAGE(OIS_DEBUG ${OIS_DEBUG_LIB} ${OIS_INCLUDE_FILE})
			CHECK_PACKAGE(OIS_RELEASE ${OIS_RELEASE_LIB} ${OIS_INCLUDE_FILE})
			IF(OIS_DEBUG_FOUND AND OIS_RELEASE_FOUND)
				SET(OIS_LIBRARIES debug ${OIS_DEBUG_LIBRARIES}
				              optimized ${OIS_RELEASE_LIBRARIES})
				SET(OIS_FOUND ${OIS_DEBUG_FOUND})
				SET(OIS_INCLUDE_DIR ${OIS_DEBUG_INCLUDE_DIR})
			ELSE(OIS_DEBUG_FOUND AND OIS_RELEASE_FOUND)
				LOG_FATAL("Need both OIS libraries for multi-target solution")
			ENDIF(OIS_DEBUG_FOUND AND OIS_RELEASE_FOUND)
		ELSE(CMAKE_CONFIGURATION_TYPES)
			IF(RELEASE)
				SET(OIS_LIB ${OIS_RELEASE_LIB})
			ELSE(RELEASE)
				SET(OIS_LIB ${OIS_DEBUG_LIB})
			ENDIF(RELEASE)
			CHECK_PACKAGE(OIS ${OIS_LIB} ${OIS_INCLUDE_FILE})
		ENDIF(CMAKE_CONFIGURATION_TYPES)
		SET(OIS_INCLUDE_DIR "${OIS_INCLUDE_DIR}/${OIS_BUNDLE_PATH}/include/OIS")
	ELSEIF(UNIX)
		CHECK_PACKAGE(OIS "libOIS.so" "OIS/OIS.h")
		SET(OIS_INCLUDE_DIR ${OIS_INCLUDE_DIR}/OIS)
	ELSEIF (APPLE)
		MESSAGE(FATAL_ERROR "Apple support is not implemented yet")
	ELSE (WIN32)
		MESSAGE(FATAL_ERROR "Unknown error or a not supported OS")
	ENDIF(WIN32)

	IF(NOT OIS_FOUND)
		LOG_FATAL("OpenInputSystem could not be found")
	ENDIF(NOT OIS_FOUND)

	# ----- #
	# MyGUI #
	# ----- #

	IF(WIN32)
		SET(MYGUI_PATH "MyGUI")
		SET(MYGUI_LIB_PATH "${MYGUI_PATH}/Build/lib")

		# General MyGUI
		SET(MYGUI_RELEASE_LIB "${MYGUI_LIB_PATH}/Release/MyGUIEngine.lib")
		SET(MYGUI_DEBUG_LIB "${MYGUI_LIB_PATH}/Debug/MyGUIEngine_d.lib")
		SET(MYGUI_INCLUDE_FILE "${MYGUI_PATH}/MyGUIEngine/include/MyGUI.h")
		# Settings for MyGUI OGRE connection
		SET(MYGUI_OGRE_RELEASE_LIB "${MYGUI_LIB_PATH}/Release/MyGUI.OgrePlatform.lib")
		SET(MYGUI_OGRE_DEBUG_LIB "${MYGUI_LIB_PATH}/Debug/MyGUI.OgrePlatform_d.lib")
		SET(MYGUI_OGRE_INCLUDE_FILE "${MYGUI_PATH}/Platforms/Ogre/OgrePlatform/include/MyGUI_OgrePlatform.h")
		IF(CMAKE_CONFIGURATION_TYPES)
			CHECK_PACKAGE(MYGUI_DEBUG ${MYGUI_DEBUG_LIB} ${MYGUI_INCLUDE_FILE})
			CHECK_PACKAGE(MYGUI_RELEASE ${MYGUI_RELEASE_LIB} ${MYGUI_INCLUDE_FILE})
			CHECK_PACKAGE(MYGUI_OGRE_DEBUG ${MYGUI_OGRE_DEBUG_LIB} ${MYGUI_OGRE_INCLUDE_FILE})
			CHECK_PACKAGE(MYGUI_OGRE_RELEASE ${MYGUI_OGRE_RELEASE_LIB} ${MYGUI_OGRE_INCLUDE_FILE})
			IF(MYGUI_DEBUG_FOUND AND MYGUI_RELEASE_FOUND)
				SET(MYGUI_FOUND ${MYGUI_DEBUG_FOUND})
				SET(MYGUI_LIBRARIES debug ${MYGUI_DEBUG_LIBRARIES}
				                optimized ${MYGUI_RELEASE_LIBRARIES})
				SET(MYGUI_INCLUDE_DIR ${MYGUI_DEBUG_INCLUDE_DIR})
			ELSE(MYGUI_DEBUG_FOUND AND MYGUI_RELEASE_FOUND)
				LOG_FATAL("Need both MYGUI-Engine libraries for multi-target solution")
			ENDIF(MYGUI_DEBUG_FOUND AND MYGUI_RELEASE_FOUND)
			IF(MYGUI_OGRE_DEBUG_FOUND AND MYGUI_OGRE_RELEASE_FOUND)
				SET(MYGUI_OGRE_FOUND ${MYGUI_OGRE_DEBUG_FOUND})
				SET(MYGUI_OGRE_LIBRARIES debug ${MYGUI_OGRE_DEBUG_LIBRARIES}
				                     optimized ${MYGUI_OGRE_RELEASE_LIBRARIES})
				SET(MYGUI_OGRE_INCLUDE_DIR ${MYGUI_OGRE_DEBUG_INCLUDE_DIR})
			ELSE(MYGUI_OGRE_DEBUG_FOUND AND MYGUI_OGRE_RELEASE_FOUND)
				LOG_FATAL("Need both MYGUI-OGRE libraries for multi-target solution")
			ENDIF(MYGUI_OGRE_DEBUG_FOUND AND MYGUI_OGRE_RELEASE_FOUND)
		ELSE(CMAKE_CONFIGURATION_TYPES)
			IF(RELEASE)
				SET(MYGUI_LIB ${MYGUI_RELEASE_LIB})
				SET(MYGUI_OGRE_LIB ${MYGUI_OGRE_RELEASE_LIB})
			ELSE(RELEASE)
				SET(MYGUI_LIB ${MYGUI_DEBUG_LIB})
				SET(MYGUI_OGRE_LIB ${MYGUI_OGRE_DEBUG_LIB})
			ENDIF(RELEASE)
			CHECK_PACKAGE(MYGUI ${MYGUI_LIB} ${MYGUI_INCLUDE_FILE})
			CHECK_PACKAGE(MYGUI_OGRE ${MYGUI_OGRE_LIB} ${MYGUI_OGRE_INCLUDE_FILE})
		ENDIF(CMAKE_CONFIGURATION_TYPES)
			SET(MYGUI_INCLUDE_DIR "${MYGUI_INCLUDE_DIR}/${MYGUI_PATH}/MyGUIEngine/include")
			SET(MYGUI_OGRE_INCLUDE_DIR "${MYGUI_OGRE_INCLUDE_DIR}/${MYGUI_PATH}/Platforms/Ogre/OgrePlatform/include")
	ELSEIF(UNIX)
		CHECK_PACKAGE(MYGUI "libMyGUIEngine.so" "MYGUI/MyGUI.h")
		CHECK_PACKAGE(MYGUI_OGRE "libMyGUI.OgrePlatform.a" "MYGUI/MyGUI.h")
		SET(MYGUI_INCLUDE_DIR ${MYGUI_INCLUDE_DIR}/MYGUI)
	ELSEIF (APPLE)
		MESSAGE(FATAL_ERROR "Apple support is not implemented yet")
	ELSE (WIN32)
		MESSAGE(FATAL_ERROR "Unknown error or a not supported OS")
	ENDIF(WIN32)

	IF(NOT MYGUI_FOUND)
	LOG_FATAL("MyGUI could not be found")
	ENDIF(NOT MYGUI_FOUND)

	# ------- #
	# TinyXML #
	# ------- #

	SET(TINYXML_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/tinyxml")

	# -------------- #
	# Boost.Geometry #
	# -------------- #

	SET(BOOST_GEOMETRY_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/boost-geometry")

	# --- #
	# ODE #
	# --- #

	SET(ODE_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/ODE_r1727/include")

	# ------ #
	# Vorbis #
	# ------ #

	IF(WIN32)
		SET(VORBIS_PATH "libvorbis")
		SET(VORBIS_RELEASE_LIB "${VORBIS_PATH}/lib/Release/libvorbis_static.lib")
		SET(VORBIS_FILE_RELEASE_LIB "${VORBIS_PATH}/lib/Release/libvorbisfile_static.lib")
		SET(VORBIS_DEBUG_LIB "${VORBIS_PATH}/lib/Debug/libvorbis_static.lib")
		SET(VORBIS_FILE_DEBUG_LIB "${VORBIS_PATH}/lib/Debug/libvorbisfile_static.lib")

		SET(VORBIS_INCLUDE_PREFIX "${VORBIS_PATH}/include")
		SET(VORBIS_INCLUDE_FILE "${VORBIS_INCLUDE_PREFIX}/vorbis/vorbisenc.h")
		SET(VORBIS_FILE_INCLUDE_FILE "${VORBIS_INCLUDE_PREFIX}/vorbis/vorbisfile.h")

		IF(CMAKE_CONFIGURATION_TYPES)
			CHECK_PACKAGE(VORBIS_DEBUG ${VORBIS_DEBUG_LIB} ${VORBIS_INCLUDE_FILE})
			CHECK_PACKAGE(VORBIS_RELEASE ${VORBIS_RELEASE_LIB} ${VORBIS_INCLUDE_FILE})
			CHECK_PACKAGE(VORBIS_FILE_DEBUG ${VORBIS_FILE_DEBUG_LIB} ${VORBIS_FILE_INCLUDE_FILE})
			CHECK_PACKAGE(VORBIS_FILE_RELEASE ${VORBIS_FILE_RELEASE_LIB} ${VORBIS_FILE_INCLUDE_FILE})
			IF(VORBIS_DEBUG_FOUND AND VORBIS_RELEASE_FOUND)
				SET(VORBIS_INCLUDE_DIR ${VORBIS_DEBUG_INCLUDE_DIR})
				SET(VORBIS_FOUND ${VORBIS_DEBUG_FOUND})
				SET(VORBIS_LIBRARIES debug ${VORBIS_DEBUG_LIBRARIES}
				                 optimized ${VORBIS_RELEASE_LIBRARIES})
				LOG("Set VORBIS to ${VORBIS_LIBRARIES}")
			ELSE(VORBIS_DEBUG_FOUND AND VORBIS_RELEASE_FOUND)
				LOG_FATAL("Need both libraries of VORBIS for multi-target solution")
			ENDIF(VORBIS_DEBUG_FOUND AND VORBIS_RELEASE_FOUND)
			IF(VORBIS_FILE_DEBUG_FOUND AND VORBIS_FILE_RELEASE_FOUND)
				SET(VORBIS_FILE_INCLUDE_DIR ${VORBIS_DEBUG_INCLUDE_DIR})
				SET(VORBIS_FILE_FOUND ${VORBIS_FILE_DEBUG_FOUND})
				SET(VORBIS_FILE_LIBRARIES debug ${VORBIS_FILE_DEBUG_LIBRARIES}
				                      optimized ${VORBIS_FILE_RELEASE_LIBRARIES})
			ELSE(VORBIS_FILE_DEBUG_FOUND AND VORBIS_FILE_RELEASE_FOUND)
				LOG_FATAL("Need both libraries of VORBIS_FILE for multi-target solution")
			ENDIF(VORBIS_FILE_DEBUG_FOUND AND VORBIS_FILE_RELEASE_FOUND)
		ELSE(CMAKE_CONFIGURATION_TYPES)
			IF(RELEASE)
				SET(VORBIS_LIB ${VORBIS_RELEASE_LIB})
				SET(VORBIS_FILE_LIB ${VORBIS_FILE_RELEASE_LIB})
			ELSE(RELEASE)
				SET(VORBIS_LIB ${VORBIS_DEBUG_LIB})
				SET(VORBIS_FILE_LIB ${VORBIS_FILE_DEBUG_LIB})
			ENDIF(RELEASE)
			CHECK_PACKAGE(VORBIS ${VORBIS_LIB} ${VORBIS_INCLUDE_FILE})
			CHECK_PACKAGE(VORBIS_FILE ${VORBIS_FILE_LIB} ${VORBIS_FILE_INCLUDE_FILE})
		ENDIF(CMAKE_CONFIGURATION_TYPES)
	ELSEIF(UNIX)
		CHECK_PACKAGE(VORBIS "libvorbis.so" "vorbis/vorbisenc.h")
		CHECK_PACKAGE(VORBIS_FILE "libvorbisfile.so" "vorbis/vorbisfile.h")
	ELSEIF (APPLE)
		MESSAGE(FATAL_ERROR "Apple support is not implemented yet")
	ELSE (WIN32)
		MESSAGE(FATAL_ERROR "Unknown error or a not supported OS")
	ENDIF(WIN32)

	IF(NOT VORBIS_FOUND)
	LOG("Vorbis include: ${VORBIS_INCLUDE_DIR}")
	LOG_FATAL("Vorbis could not be found")
	ELSE(NOT VORBIS_FOUND)  
	SET(VORBIS_INCLUDE_DIR "${VORBIS_INCLUDE_DIR}/${VORBIS_INCLUDE_PREFIX}")
	ENDIF(NOT VORBIS_FOUND)

	# --- #
	# OGG #
	# --- #

	IF(WIN32)
		SET(OGG_PATH "libogg")
		SET(OGG_INCLUDE_PATH "${OGG_PATH}/include")
		SET(OGG_DEBUG_LIB "${OGG_PATH}/lib/Debug/libogg_static.lib")
		SET(OGG_RELEASE_LIB "${OGG_PATH}/lib/Release/libogg_static.lib")
		SET(OGG_INCLUDE_FILE "${OGG_INCLUDE_PATH}/ogg/ogg.h")
		IF(CMAKE_CONFIGURATION_TYPES)
			CHECK_PACKAGE(OGG_DEBUG ${OGG_DEBUG_LIB} ${OGG_INCLUDE_FILE})
			CHECK_PACKAGE(OGG_RELEASE ${OGG_RELEASE_LIB} ${OGG_INCLUDE_FILE})
			IF(OGG_DEBUG_FOUND AND OGG_RELEASE_FOUND)
				SET(OGG_FOUND ${OGG_DEBUG_FOUND})
				SET(OGG_INCLUDE_DIR ${OGG_DEBUG_INCLUDE_DIR})
				SET(OGG_LIBRARIES debug ${OGG_DEBUG_LIBRARIES}
				              optimized ${OGG_RELEASE_LIBRARIES})
			ELSE(OGG_DEBUG_FOUND AND OGG_RELEASE_FOUND)
				LOG_FATAL("Need both OGG libraries for multi-target solution")
			ENDIF(OGG_DEBUG_FOUND AND OGG_RELEASE_FOUND)
		ELSE(CMAKE_CONFIGURATION_TYPES)
			IF(RELEASE)
				SET(OGG_LIB ${OGG_RELEASE_LIB})
			ELSE(RELEASE)
				SET(OGG_LIB ${OGG_DEBUG_LIB})
			ENDIF(RELEASE)
			SET(OGG_LIB ${OGG_LIB})
			CHECK_PACKAGE(OGG ${OGG_LIB} ${OGG_INCLUDE_FILE})
		ENDIF(CMAKE_CONFIGURATION_TYPES)
	ELSEIF(UNIX)
		CHECK_PACKAGE(OGG "libogg.so" "ogg/ogg.h")
	ELSEIF (APPLE)
		MESSAGE(FATAL_ERROR "Apple support is not implemented yet")
	ELSE (WIN32)
		MESSAGE(FATAL_ERROR "Unknown error or a not supported OS")
	ENDIF(WIN32)

	IF(NOT OGG_FOUND)
		LOG_FATAL("Ogg could not be found")
	ELSE(NOT OGG_FOUND)
		SET(OGG_INCLUDE_DIR "${OGG_INCLUDE_DIR}/${OGG_INCLUDE_PATH}")
	ENDIF(NOT OGG_FOUND)

ENDMACRO(FIND_ALL_DEPENDENCIES)