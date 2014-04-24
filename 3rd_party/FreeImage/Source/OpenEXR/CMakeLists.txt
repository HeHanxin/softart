
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(Half)
INCLUDE_DIRECTORIES(Iex)
INCLUDE_DIRECTORIES(IlmImf)
INCLUDE_DIRECTORIES(IlmThread)
INCLUDE_DIRECTORIES(Imath)

set( OPENEXR_SOURCES 

Half/half.cpp Half/half.h Half/halfFunction.h Half/halfLimits.h 

Iex/IexBaseExc.cpp Iex/IexThrowErrnoExc.cpp Iex/Iex.h 
Iex/IexBaseExc.h Iex/IexErrnoExc.h Iex/IexMacros.h 
Iex/IexMathExc.h Iex/IexThrowErrnoExc.h 

IlmImf/ImfAcesFile.cpp
IlmImf/ImfAttribute.cpp IlmImf/ImfB44Compressor.cpp IlmImf/ImfBoxAttribute.cpp 
IlmImf/ImfChannelList.cpp IlmImf/ImfChannelListAttribute.cpp 
IlmImf/ImfChromaticities.cpp IlmImf/ImfChromaticitiesAttribute.cpp 
IlmImf/ImfCompressionAttribute.cpp IlmImf/ImfCompressor.cpp 
IlmImf/ImfConvert.cpp IlmImf/ImfCRgbaFile.cpp IlmImf/ImfDoubleAttribute.cpp 
IlmImf/ImfEnvmap.cpp IlmImf/ImfEnvmapAttribute.cpp IlmImf/ImfFloatAttribute.cpp 
IlmImf/ImfFrameBuffer.cpp IlmImf/ImfFramesPerSecond.cpp IlmImf/ImfHeader.cpp 
IlmImf/ImfHuf.cpp IlmImf/ImfInputFile.cpp IlmImf/ImfIntAttribute.cpp 
IlmImf/ImfIO.cpp IlmImf/ImfKeyCode.cpp IlmImf/ImfKeyCodeAttribute.cpp 
IlmImf/ImfLineOrderAttribute.cpp IlmImf/ImfLut.cpp IlmImf/ImfMatrixAttribute.cpp
IlmImf/ImfMisc.cpp IlmImf/ImfMultiView.cpp
IlmImf/ImfOpaqueAttribute.cpp IlmImf/ImfOutputFile.cpp 
IlmImf/ImfPizCompressor.cpp IlmImf/ImfPreviewImage.cpp 
IlmImf/ImfPreviewImageAttribute.cpp IlmImf/ImfPxr24Compressor.cpp 
IlmImf/ImfRational.cpp IlmImf/ImfRationalAttribute.cpp IlmImf/ImfRgbaFile.cpp 
IlmImf/ImfRgbaYca.cpp IlmImf/ImfRleCompressor.cpp 
IlmImf/ImfScanLineInputFile.cpp IlmImf/ImfStandardAttributes.cpp 
IlmImf/ImfStdIO.cpp IlmImf/ImfStringAttribute.cpp IlmImf/ImfStringVectorAttribute.cpp
IlmImf/ImfTestFile.cpp 
IlmImf/ImfThreading.cpp IlmImf/ImfTileDescriptionAttribute.cpp 
IlmImf/ImfTiledInputFile.cpp IlmImf/ImfTiledMisc.cpp 
IlmImf/ImfTiledOutputFile.cpp IlmImf/ImfTiledRgbaFile.cpp 
IlmImf/ImfTileOffsets.cpp IlmImf/ImfTimeCode.cpp IlmImf/ImfTimeCodeAttribute.cpp 
IlmImf/ImfVecAttribute.cpp IlmImf/ImfVersion.cpp IlmImf/ImfWav.cpp 
IlmImf/ImfZipCompressor.cpp 
IlmImf/b44ExpLogTable.h 
IlmImf/ImfAcesFile.h
IlmImf/ImfArray.h IlmImf/ImfAttribute.h 
IlmImf/ImfAutoArray.h IlmImf/ImfB44Compressor.h IlmImf/ImfBoxAttribute.h 
IlmImf/ImfChannelList.h IlmImf/ImfChannelListAttribute.h 
IlmImf/ImfCheckedArithmetic.h
IlmImf/ImfChromaticities.h IlmImf/ImfChromaticitiesAttribute.h 
IlmImf/ImfCompression.h IlmImf/ImfCompressionAttribute.h IlmImf/ImfCompressor.h
IlmImf/ImfConvert.h IlmImf/ImfCRgbaFile.h IlmImf/ImfDoubleAttribute.h 
IlmImf/ImfEnvmap.h IlmImf/ImfEnvmapAttribute.h IlmImf/ImfFloatAttribute.h 
IlmImf/ImfFrameBuffer.h IlmImf/ImfFramesPerSecond.h IlmImf/ImfHeader.h 
IlmImf/ImfHuf.h IlmImf/ImfInputFile.h IlmImf/ImfInt64.h IlmImf/ImfIntAttribute.h 
IlmImf/ImfIO.h IlmImf/ImfKeyCode.h IlmImf/ImfKeyCodeAttribute.h 
IlmImf/ImfLineOrder.h IlmImf/ImfLineOrderAttribute.h IlmImf/ImfLut.h 
IlmImf/ImfMatrixAttribute.h IlmImf/ImfMisc.h IlmImf/ImfMultiView.h IlmImf/ImfName.h 
IlmImf/ImfOpaqueAttribute.h IlmImf/ImfOutputFile.h IlmImf/ImfPixelType.h 
IlmImf/ImfPizCompressor.h IlmImf/ImfPreviewImage.h 
IlmImf/ImfPreviewImageAttribute.h IlmImf/ImfPxr24Compressor.h 
IlmImf/ImfRational.h IlmImf/ImfRationalAttribute.h IlmImf/ImfRgba.h 
IlmImf/ImfRgbaFile.h IlmImf/ImfRgbaYca.h IlmImf/ImfRleCompressor.h 
IlmImf/ImfScanLineInputFile.h IlmImf/ImfStandardAttributes.h 
IlmImf/ImfStdIO.h IlmImf/ImfStringAttribute.h IlmImf/ImfStringVectorAttribute.h
IlmImf/ImfTestFile.h 
IlmImf/ImfThreading.h IlmImf/ImfTileDescription.h 
IlmImf/ImfTileDescriptionAttribute.h IlmImf/ImfTiledInputFile.h 
IlmImf/ImfTiledMisc.h IlmImf/ImfTiledOutputFile.h IlmImf/ImfTiledRgbaFile.h 
IlmImf/ImfTileOffsets.h IlmImf/ImfTimeCode.h 
IlmImf/ImfTimeCodeAttribute.h IlmImf/ImfVecAttribute.h IlmImf/ImfVersion.h 
IlmImf/ImfWav.h IlmImf/ImfXdr.h IlmImf/ImfZipCompressor.h 

IlmThread/IlmThread.cpp IlmThread/IlmThreadMutex.cpp
IlmThread/IlmThreadPool.cpp IlmThread/IlmThreadSemaphore.cpp 
IlmThread/IlmThread.h IlmThread/IlmThreadMutex.h IlmThread/IlmThreadPool.h 
IlmThread/IlmThreadSemaphore.h 

Imath/ImathBox.cpp Imath/ImathColorAlgo.cpp Imath/ImathFun.cpp 
Imath/ImathMatrixAlgo.cpp Imath/ImathRandom.cpp Imath/ImathShear.cpp 
Imath/ImathVec.cpp Imath/ImathBox.h Imath/ImathBoxAlgo.h Imath/ImathColor.h 
Imath/ImathColorAlgo.h Imath/ImathEuler.h Imath/ImathExc.h Imath/ImathFrame.h 
Imath/ImathFrustum.h Imath/ImathFun.h Imath/ImathGL.h Imath/ImathGLU.h 
Imath/ImathHalfLimits.h Imath/ImathInt64.h Imath/ImathInterval.h 
Imath/ImathLimits.h Imath/ImathLine.h Imath/ImathLineAlgo.h Imath/ImathMath.h 
Imath/ImathMatrix.h Imath/ImathMatrixAlgo.h Imath/ImathPlane.h 
Imath/ImathPlatform.h Imath/ImathQuat.h Imath/ImathRandom.h 
Imath/ImathRoots.h Imath/ImathShear.h Imath/ImathSphere.h 
Imath/ImathVec.h Imath/ImathVecAlgo.h 
)

#ADD_DEFINITIONS(-DPNG_BUILD_DLL)

add_library ( OpenEXR_s STATIC ${OPENEXR_SOURCES} )

  IF(FREEIMAGE_BUILD_STATIC)
    INSTALL(TARGETS 
	OpenEXR_s
	LIBRARY DESTINATION Dist
	ARCHIVE DESTINATION Dist
    )
  ENDIF()  
