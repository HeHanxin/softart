
set( LIBJPEG_SOURCES 
	jaricom.c jcapimin.c jcapistd.c jcarith.c jccoefct.c 
	jccolor.c jcdctmgr.c jchuff.c jcinit.c jcmainct.c jcmarker.c 
	jcmaster.c jcomapi.c jcparam.c jcprepct.c jcsample.c jctrans.c 
	jdapimin.c jdapistd.c jdarith.c jdatadst.c jdatasrc.c 
	jdcoefct.c jdcolor.c jddctmgr.c jdhuff.c jdinput.c jdmainct.c 
	jdmarker.c jdmaster.c jdmerge.c jdpostct.c jdsample.c jdtrans.c 
	jerror.c jfdctflt.c jfdctfst.c jfdctint.c jidctflt.c jidctfst.c 
	jidctint.c jmemmgr.c 
	jmemnobs.c jquant1.c jquant2.c jutils.c 
	transupp.c
	jconfig.h 
	jdct.h jerror.h jinclude.h jmemsys.h jmorecfg.h jpegint.h 
	jpeglib.h jversion.h 
	transupp.h
)

IF(FREEIMAGE_BUILD_JPEG_STATIC)
  add_library ( LibJPEG_s STATIC ${LIBJPEG_SOURCES} )

  IF(FREEIMAGE_BUILD_STATIC)
    INSTALL(TARGETS 
	LibJPEG_s
	LIBRARY DESTINATION Dist
	ARCHIVE DESTINATION Dist
    )
    INSTALL(FILES jconfig.h jerror.h jmorecfg.h jpegint.h jpeglib.h DESTINATION Dist/LibJPEG)

  ENDIF()

ENDIF(FREEIMAGE_BUILD_JPEG_STATIC)

IF(FREEIMAGE_BUILD_JPEG_DLL)
  ADD_DEFINITIONS( -DLIBJPEG_BUILD_DLL )
  add_library ( FreeImageJPEG SHARED ${LIBJPEG_SOURCES} )

  INSTALL(TARGETS 
	FreeImageJPEG
	RUNTIME DESTINATION Dist
	LIBRARY DESTINATION Dist
	ARCHIVE DESTINATION Dist
  )
  INSTALL(FILES jconfig.h jerror.h jmorecfg.h jpegint.h jpeglib.h DESTINATION Dist/LibJPEG)

ENDIF(FREEIMAGE_BUILD_JPEG_DLL)
