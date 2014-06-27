PROJECT(freetype)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.9)

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

IF(COMMAND cmake_policy)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

SET(FREETYPE_HOME_DIRECTORY "${CMAKE_HOME_DIRECTORY}")

INCLUDE(${FREETYPE_HOME_DIRECTORY}/../../cmake/SystemInfo.cmake)
INCLUDE(${FREETYPE_HOME_DIRECTORY}/../../cmake/ConfigCompiler.cmake)
INCLUDE(${FREETYPE_HOME_DIRECTORY}/../../cmake/Variables.cmake)

IF(MSVC)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /WX-")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX-")
ENDIF()

# The project settings

SET(LIB_NAME freetype)

SET(FREETYPE_FT_MODULES_SOURCE_FILES
	${FREETYPE_HOME_DIRECTORY}/src/base/ftbbox.c
	${FREETYPE_HOME_DIRECTORY}/src/base/ftgxval.c
	${FREETYPE_HOME_DIRECTORY}/src/base/ftlcdfil.c
	${FREETYPE_HOME_DIRECTORY}/src/base/ftmm.c
	${FREETYPE_HOME_DIRECTORY}/src/base/ftotval.c
	${FREETYPE_HOME_DIRECTORY}/src/base/ftpatent.c
	${FREETYPE_HOME_DIRECTORY}/src/base/ftpfr.c
	${FREETYPE_HOME_DIRECTORY}/src/base/ftsynth.c
	${FREETYPE_HOME_DIRECTORY}/src/base/fttype1.c
	${FREETYPE_HOME_DIRECTORY}/src/base/ftwinfnt.c
	${FREETYPE_HOME_DIRECTORY}/src/base/ftxf86.c
	${FREETYPE_HOME_DIRECTORY}/src/pcf/pcf.c
	${FREETYPE_HOME_DIRECTORY}/src/pfr/pfr.c
	${FREETYPE_HOME_DIRECTORY}/src/psaux/psaux.c
	${FREETYPE_HOME_DIRECTORY}/src/pshinter/pshinter.c
	${FREETYPE_HOME_DIRECTORY}/src/psnames/psmodule.c
	${FREETYPE_HOME_DIRECTORY}/src/raster/raster.c
	${FREETYPE_HOME_DIRECTORY}/src/sfnt/sfnt.c
	${FREETYPE_HOME_DIRECTORY}/src/truetype/truetype.c
	${FREETYPE_HOME_DIRECTORY}/src/type1/type1.c
	${FREETYPE_HOME_DIRECTORY}/src/cid/type1cid.c
	${FREETYPE_HOME_DIRECTORY}/src/type42/type42.c
	${FREETYPE_HOME_DIRECTORY}/src/winfonts/winfnt.c
)
SET(FREETYPE_SOURCE_FILES
	${FREETYPE_HOME_DIRECTORY}/src/autofit/autofit.c
	${FREETYPE_HOME_DIRECTORY}/src/bdf/bdf.c
	${FREETYPE_HOME_DIRECTORY}/src/cff/cff.c
	${FREETYPE_HOME_DIRECTORY}/src/base/ftbase.c
	${FREETYPE_HOME_DIRECTORY}/src/base/ftbitmap.c
	${FREETYPE_HOME_DIRECTORY}/src/cache/ftcache.c
	${FREETYPE_HOME_DIRECTORY}/builds/win32/ftdebug.c
	${FREETYPE_HOME_DIRECTORY}/src/base/ftfstype.c
	${FREETYPE_HOME_DIRECTORY}/src/base/ftgasp.c
	${FREETYPE_HOME_DIRECTORY}/src/base/ftglyph.c
	${FREETYPE_HOME_DIRECTORY}/src/gzip/ftgzip.c
	${FREETYPE_HOME_DIRECTORY}/src/base/ftinit.c
	${FREETYPE_HOME_DIRECTORY}/src/lzw/ftlzw.c
	${FREETYPE_HOME_DIRECTORY}/src/base/ftstroke.c
	${FREETYPE_HOME_DIRECTORY}/src/base/ftsystem.c
	${FREETYPE_HOME_DIRECTORY}/src/smooth/smooth.c
)
SET(FREETYPE_HEADER_FILES
	${FREETYPE_HOME_DIRECTORY}/include/ft2build.h
	${FREETYPE_HOME_DIRECTORY}/include/freetype/config/ftconfig.h
	${FREETYPE_HOME_DIRECTORY}/include/freetype/config/ftheader.h
	${FREETYPE_HOME_DIRECTORY}/include/freetype/config/ftmodule.h
	${FREETYPE_HOME_DIRECTORY}/include/freetype/config/ftoption.h
	${FREETYPE_HOME_DIRECTORY}/include/freetype/config/ftstdlib.h
)
SOURCE_GROUP("Source Files" FILES ${FREETYPE_SOURCE_FILES})
SOURCE_GROUP("Source Files\\FT_MODULES" FILES ${FREETYPE_FT_MODULES_SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${FREETYPE_HEADER_FILES})

ADD_DEFINITIONS(-DFT2_BUILD_LIBRARY)
IF (MSVC)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /DFT_DEBUG_LEVEL_ERROR /DFT_DEBUG_LEVEL_TRACE")
ENDIF()

INCLUDE_DIRECTORIES(${FREETYPE_HOME_DIRECTORY}/include)
ADD_LIBRARY(
	${LIB_NAME} STATIC
	${FREETYPE_FT_MODULES_SOURCE_FILES} ${FREETYPE_SOURCE_FILES} ${FREETYPE_HEADER_FILES}
)
  
INSTALL(TARGETS ${LIB_NAME}
  ARCHIVE DESTINATION .
)
  
TARGET_LINK_LIBRARIES(${LIB_NAME})