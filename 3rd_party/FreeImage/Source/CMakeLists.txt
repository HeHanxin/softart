ADD_SUBDIRECTORY(LibJPEG)
ADD_SUBDIRECTORY(LibMNG)
ADD_SUBDIRECTORY(LibOpenJPEG)
ADD_SUBDIRECTORY(LibPNG)
ADD_SUBDIRECTORY(LibRawLite)
ADD_SUBDIRECTORY(LibTIFF4)
ADD_SUBDIRECTORY(ZLib)
ADD_SUBDIRECTORY(OpenEXR)

INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${OPENEXR_INCLUDE_DIRS})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/Source/OpenEXR/IlmImf 
	${CMAKE_SOURCE_DIR}/Source/OpenEXR/Imath 
	${CMAKE_SOURCE_DIR}/Source/OpenEXR/Iex 
	${CMAKE_SOURCE_DIR}/Source/OpenEXR/Half)

set( FREEIMAGE_SOURCES
	FreeImage.h

	DeprecationManager/Deprecated.cpp 
	DeprecationManager/DeprecationMgr.cpp DeprecationManager/DeprecationMgr.h 

	FreeImage/BitmapAccess.cpp FreeImage/CacheFile.cpp 
	FreeImage/ColorLookup.cpp FreeImage/Conversion.cpp 	
	FreeImage/Conversion16_555.cpp FreeImage/Conversion16_565.cpp 
	FreeImage/Conversion24.cpp FreeImage/Conversion32.cpp 
	FreeImage/Conversion4.cpp FreeImage/Conversion8.cpp
	FreeImage/ConversionFloat.cpp 
	FreeImage/ConversionRGBF.cpp FreeImage/ConversionRGB16.cpp FreeImage/ConversionType.cpp 
	FreeImage/ConversionUINT16.cpp
	FreeImage/FreeImage.cpp FreeImage/FreeImageIO.cpp FreeImage/GetType.cpp 
	FreeImage/Halftoning.cpp FreeImage/J2KHelper.cpp FreeImage/MemoryIO.cpp
	FreeImage/MNGHelper.cpp
	FreeImage/MultiPage.cpp FreeImage/NNQuantizer.cpp 
	FreeImage/PixelAccess.cpp FreeImage/Plugin.cpp FreeImage/PluginBMP.cpp 
	FreeImage/PluginCUT.cpp FreeImage/PluginDDS.cpp FreeImage/PluginEXR.cpp
	FreeImage/PluginG3.cpp FreeImage/PluginGIF.cpp FreeImage/PluginHDR.cpp
	FreeImage/PluginICO.cpp FreeImage/PluginIFF.cpp FreeImage/PluginJ2K.cpp
	FreeImage/PluginJNG.cpp
	FreeImage/PluginJP2.cpp FreeImage/PluginJPEG.cpp 
	FreeImage/PluginKOALA.cpp FreeImage/PluginMNG.cpp 
	FreeImage/PluginPCD.cpp FreeImage/PluginPCX.cpp FreeImage/PluginPFM.cpp 
	FreeImage/PluginPICT.cpp FreeImage/PluginPNG.cpp FreeImage/PluginPNM.cpp
	FreeImage/PluginPSD.cpp FreeImage/PluginRAS.cpp FreeImage/PluginRAW.cpp
	FreeImage/PluginSGI.cpp FreeImage/PluginTARGA.cpp 
	FreeImage/PluginTIFF.cpp FreeImage/PluginWBMP.cpp 
	FreeImage/PluginXBM.cpp FreeImage/PluginXPM.cpp 
	FreeImage/PSDParser.cpp FreeImage/TIFFLogLuv.cpp 
	FreeImage/tmoColorConvert.cpp FreeImage/tmoDrago03.cpp 
	FreeImage/tmoFattal02.cpp FreeImage/tmoReinhard05.cpp 
	FreeImage/ToneMapping.cpp 
	FreeImage/WuQuantizer.cpp 
	FreeImage/ZLibInterface.cpp FreeImage/PSDParser.h 

	FreeImageToolkit/Background.cpp FreeImageToolkit/BSplineRotate.cpp 
	FreeImageToolkit/Channels.cpp FreeImageToolkit/ClassicRotate.cpp 
	FreeImageToolkit/Colors.cpp FreeImageToolkit/CopyPaste.cpp 
	FreeImageToolkit/Display.cpp 
	FreeImageToolkit/Flip.cpp FreeImageToolkit/JPEGTransform.cpp 
	FreeImageToolkit/MultigridPoissonSolver.cpp 
	FreeImageToolkit/Rescale.cpp FreeImageToolkit/Resize.cpp 
	FreeImageToolkit/Filters.h FreeImageToolkit/Resize.h 

	Metadata/Exif.cpp Metadata/FIRational.cpp Metadata/FreeImageTag.cpp 
	Metadata/IPTC.cpp Metadata/TagConversion.cpp Metadata/TagLib.cpp 
	Metadata/XTIFF.cpp Metadata/FIRational.h
	)

IF(FREEIMAGE_BUILD_STATIC)
  ADD_DEFINITIONS(-DFREEIMAGE_LIB -DLIBRAW_NODLL -DOPJ_STATIC)
  add_library (FreeImage_s STATIC ${FREEIMAGE_SOURCES} )

  INSTALL(TARGETS 
	FreeImage_s
	RUNTIME DESTINATION Dist 
	LIBRARY DESTINATION Dist
	ARCHIVE DESTINATION Dist
  )

ENDIF()

IF(FREEIMAGE_BUILD_DLL)
  ADD_DEFINITIONS(-DFREEIMAGE_EXPORTS -DLIBRAW_NODLL -DOPJ_STATIC)
  add_library (FreeImage SHARED ${FREEIMAGE_SOURCES} )
  target_link_libraries(FreeImage OpenEXR_s LibMNG_s LibOpenJPEG_s LibRawLite_s)

  IF(FREEIMAGE_BUILD_ZLIB_DLL)
    target_link_libraries(FreeImage FreeImageZlib)
  ELSE(FREEIMAGE_BUILD_ZLIB_DLL)
    target_link_libraries(FreeImage Zlib_s)
  ENDIF(FREEIMAGE_BUILD_ZLIB_DLL)
 
  IF(FREEIMAGE_BUILD_JPEG_DLL)
    target_link_libraries(FreeImage FreeImageJPEG)
  ELSE(FREEIMAGE_BUILD_JPEG_DLL)
   target_link_libraries(FreeImage LibJPEG_s)
  ENDIF(FREEIMAGE_BUILD_JPEG_DLL)

  IF(FREEIMAGE_BUILD_PNG_DLL)
    target_link_libraries(FreeImage FreeImagePNG)
  ELSE(FREEIMAGE_BUILD_PNG_DLL)
    target_link_libraries(FreeImage LibPNG_s)
  ENDIF(FREEIMAGE_BUILD_PNG_DLL)

  IF(FREEIMAGE_BUILD_TIFF_DLL)
    target_link_libraries(FreeImage FreeImageTIFF)
  ELSE(FREEIMAGE_BUILD_TIFF_DLL)
    target_link_libraries(FreeImage LibTIFF_s)
  ENDIF(FREEIMAGE_BUILD_TIFF_DLL)

INSTALL(TARGETS 
	FreeImage
	RUNTIME DESTINATION Dist 
	LIBRARY DESTINATION Dist
	ARCHIVE DESTINATION Dist
)
ENDIF()


INSTALL(FILES FreeImage.h DESTINATION Dist)
