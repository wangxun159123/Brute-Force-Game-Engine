IF (Bfg_INCLUDE_DIR AND Bfg_LIBRARIES)
	SET (Bfg_FOUND TRUE)
ELSE ()
	# only useful with unix system and correctly installed boost log
	# on windows the paths should be set manually
	FIND_PATH (Bfg_INCLUDE_DIR "bfg/Core/Math.h"
		/usr/include
		/usr/local
		${BOOSTLOG_ROOT}/include
		${BOOST_ROOT}/include
		${BOOSTROOT}/include
	)
	# on 64 bit linux /usr/lib64 is a symlink to /usr/lib
	FIND_LIBRARY (Bfg_LIBRARIES NAMES libbfgCore.so
		PATHS
		/usr/lib
		/usr/local/lib
		${BOOSTLOG_ROOT}/lib
		${BOOST_ROOT}/lib
		${BOOSTROOT}/lib
	)
	
	IF (Bfg_INCLUDE_DIR AND Bfg_LIBRARIES)
		SET (Bfg_FOUND TRUE)
		MESSAGE (STATUS "Found Bfg: ${Bfg_INCLUDE_DIR}, ${Bfg_LIBRARIES}")
	ELSE ()
		SET (Bfg_FOUND FALSE)
		MESSAGE(STATUS "Bfg not found.")
	ENDIF ()
	
	MARK_AS_ADVANCED(Bfg_INCLUDE_DIR Bfg_LIBRARIES)
ENDIF ()
