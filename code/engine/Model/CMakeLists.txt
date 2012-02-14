cmake_minimum_required(VERSION 2.6)

BFG_INIT_PACKAGE(Model "0.3.3")

FILE(GLOB Model_Root *.cpp *.h *.hh)
FILE(GLOB Model_Events Events/*.cpp Events/*.h)
FILE(GLOB Model_Loader Loader/*.cpp Loader/*.h)
FILE(GLOB Model_Property Property/*.cpp Property/*.h)
FILE(GLOB Model_Concepts Property/Concepts/*.cpp Property/Concepts/*.h)

FILE(GLOB MODEL_FILES
	${Model_Root}
	${Model_Events}
	${Model_Loader}
	${Model_Property}
	${Model_Concepts}
	*.xml
)

SOURCE_GROUP("." FILES ${Model_Root})
SOURCE_GROUP("Events" FILES ${Model_Events})
SOURCE_GROUP("Loader" FILES ${Model_Loader})
SOURCE_GROUP("Model" FILES ${Model_Model})
SOURCE_GROUP("Property" FILES ${Model_Property})
SOURCE_GROUP("Property\\Concepts" FILES ${Model_Concepts})

INCLUDE_DIRECTORIES(${BoostLog_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${TINYXML_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${OGRE_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${OIS_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${BOOST_GEOMETRY_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${MYGUI_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(..)
INCLUDE_DIRECTORIES(.)	# For subdirectories

ADD_DEFINITIONS(-DTIXML_USE_STL)

IF(WIN32)
	ADD_DEFINITIONS(-DMODEL_EXPORTS)
ENDIF(WIN32)

ADD_CUSTOM_TARGET(ModelEnums 
	bfgEnumGen --input ${CMAKE_CURRENT_SOURCE_DIR}/Enums.xml
	        --header ${CMAKE_CURRENT_SOURCE_DIR}/Enums.hh
	        --source ${CMAKE_CURRENT_SOURCE_DIR}/Enums.cpp
	DEPENDS bfgEnumGen
)

ADD_LIBRARY(bfgModel SHARED ${MODEL_FILES})

TARGET_LINK_LIBRARIES(bfgModel
	${BoostLog_LIBRARIES}
	${Boost_LIBRARIES}
	TinyXML
	bfgBase
	bfgController
	bfgCore
	bfgEventCore
	bfgPhysics
	bfgView
)

ADD_DEPENDENCIES(bfgModel CoreEnums ModelEnums)

INSTALL(TARGETS bfgModel
        LIBRARY DESTINATION ${BFG_LIB_DIR}
        RUNTIME DESTINATION ${BFG_LIB_DIR}
        COMPONENT devel
)

INSTALL(FILES ${Model_Root}
        DESTINATION ${BFG_PACKAGE_INCLUDE_DIR}
        COMPONENT devel)

INSTALL(FILES ${Model_Events}
        DESTINATION "${BFG_PACKAGE_INCLUDE_DIR}/Events"
        COMPONENT devel)

INSTALL(FILES ${Model_Loader}
        DESTINATION "${BFG_PACKAGE_INCLUDE_DIR}/Loader"
        COMPONENT devel)

INSTALL(FILES ${Model_Property}
        DESTINATION "${BFG_PACKAGE_INCLUDE_DIR}/Property"
        COMPONENT devel)
