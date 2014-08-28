# Requirements:
#  - HEADER_FILES, SOURCE_FILES, RESOURCE_FILES, ADDITIONAL_FILES
#  - ADDITIONAL_INCLUDE_DIRECTORIES 

MACRO(SASL_CONFIG_LIBRARY ProjectName)

INCLUDE_DIRECTORIES(
	${SASL_HOME_DIR}
	${SALVIA_HOME_DIR}
	${SALVIA_BOOST_INCLUDE_DIR}
	${ADDITIONAL_INCLUDE_DIRECTORIES}
)

SOURCE_GROUP( "Header Files"	FILES ${HEADER_FILES} )
SOURCE_GROUP( "Source Files"	FILES ${SOURCE_FILES} )

ADD_LIBRARY( ${ProjectName} STATIC ${SOURCE_FILES} ${HEADER_FILES} ${ADDITIONAL_FILES} )
SET_TARGET_PROPERTIES(${ProjectName} PROPERTIES FOLDER "Shader Libs")
SALVIA_CONFIG_OUTPUT_PATHS(${ProjectName})

ENDMACRO ( SASL_CONFIG_LIBRARY )

MACRO(SASL_CONFIG_TEST ProjectName)
INCLUDE_DIRECTORIES(
	${SALVIA_HOME_DIR}
	${SALVIA_BOOST_INCLUDE_DIR}
	${ADDITIONAL_INCLUDE_DIRECTORIES}
)

SET_TARGET_PROPERTIES(${ProjectName} PROPERTIES FOLDER "Shader Tests")
ENDMACRO( SASL_CONFIG_TEST )