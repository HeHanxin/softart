cmake_minimum_required(VERSION 2.6.1)

INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

set( LIBPNG_SOURCES 
	png.c
	pngerror.c pngget.c pngmem.c 
	pngpread.c pngread.c pngrio.c pngrtran.c 
	pngrutil.c pngset.c pngtest.c pngtrans.c
	pngwio.c pngwrite.c pngwtran.c pngwutil.c png.h pnglibconf.h pngconf.h 
	pngdebug.h pnginfo.h pngpriv.h pngstruct.h	
)

IF(FREEIMAGE_BUILD_PNG_STATIC)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)

  add_library ( LibPNG_s STATIC ${LIBPNG_SOURCES} )

  IF(FREEIMAGE_BUILD_STATIC)
    INSTALL(TARGETS 
	LibPNG_s
	LIBRARY DESTINATION Dist
	ARCHIVE DESTINATION Dist
    )
    INSTALL(FILES png.h pngconf.h pnglibconf.h DESTINATION Dist/LibPNG)
  ENDIF()

ENDIF(FREEIMAGE_BUILD_PNG_STATIC)

IF(FREEIMAGE_BUILD_PNG_DLL)
  set( LIBPNG_DLL_SOURCES 
	${LIBPNG_SOURCES}
  )

  ADD_DEFINITIONS(-DPNG_BUILD_DLL -DZLIB_DLL -D_CRT_SECURE_NO_WARNINGS)

  add_library ( FreeImagePNG SHARED ${LIBPNG_DLL_SOURCES} )
  target_link_libraries( FreeImagePNG FreeImageZlib )

  INSTALL(TARGETS 
	FreeImagePNG
	RUNTIME DESTINATION Dist
	LIBRARY DESTINATION Dist
	ARCHIVE DESTINATION Dist
  )
  INSTALL(FILES png.h pngconf.h pnglibconf.h DESTINATION Dist/LibPNG)
ENDIF(FREEIMAGE_BUILD_PNG_DLL)
