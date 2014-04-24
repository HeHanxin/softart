cmake_minimum_required(VERSION 2.6.1)

ADD_DEFINITIONS(-DOPJ_STATIC -D_CRT_SECURE_NO_DEPRECATE)

set( LIBOPENJPEG_SOURCES 
	bio.c cidx_manager.c cio.c dwt.c event.c 
	image.c
	j2k.c j2k_lib.c jp2.c jpt.c
	mct.c mqc.c openjpeg.c phix_manager.c
	pi.c ppix_manager.c
	raw.c t1.c t1_generate_luts.c t2.c 
	tcd.c tgt.c thix_manager.c tpix_manager.c
	
	bio.h cidx_manager.h cio.h dwt.h event.h 
	fix.h image.h indexbox_manager.h int.h
	j2k.h j2k_lib.h jp2.h 
	mct.h mqc.h openjpeg.h opj_config.h
	opj_includes.h
	opj_malloc.h
	pi.h raw.h t1.h t1_luts.h 
	t2.h tcd.h tgt.h 
)

#ADD_DEFINITIONS(-DPNG_BUILD_DLL)

add_library ( LibOpenJPEG_s STATIC ${LIBOPENJPEG_SOURCES} )

  IF(FREEIMAGE_BUILD_STATIC)
    INSTALL(TARGETS 
	LibOpenJPEG_s
	LIBRARY DESTINATION Dist
	ARCHIVE DESTINATION Dist
    )
  ENDIF()  
