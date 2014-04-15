cmake_minimum_required(VERSION 2.6.1)

INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

set( LIBMNG_SOURCES 
	libmng_callback_xs.c libmng_chunk_descr.c libmng_chunk_io.c 
	libmng_chunk_prc.c libmng_chunk_xs.c libmng_cms.c 
	libmng_display.c libmng_dither.c libmng_error.c libmng_filter.c 
	libmng_hlapi.c libmng_jpeg.c libmng_object_prc.c 	
	libmng_pixels.c libmng_prop_xs.c libmng_read.c libmng_trace.c 
	libmng_write.c libmng_zlib.c libmng.h libmng_chunk_descr.h 
	libmng_chunk_io.h libmng_chunk_prc.h libmng_chunks.h 
	libmng_cms.h libmng_conf.h libmng_data.h libmng_display.h 
	libmng_dither.h libmng_error.h libmng_filter.h libmng_jpeg.h 
	libmng_memory.h libmng_object_prc.h libmng_objects.h libmng_pixels.h 
	libmng_read.h libmng_trace.h libmng_types.h libmng_write.h 
	libmng_zlib.h 
)

add_library ( LibMNG_s STATIC ${LIBMNG_SOURCES} )

  IF(FREEIMAGE_BUILD_STATIC)
    INSTALL(TARGETS 
	LibMNG_s
	LIBRARY DESTINATION Dist
	ARCHIVE DESTINATION Dist
    )
  ENDIF()  
