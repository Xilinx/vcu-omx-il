/*
 * Copyright (C) 2016-2020 Allegro DVT2. All rights reserved.
 * Copyright (c) 2016 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/** OMX_CoreAlg.h - OpenMax IL version 1.1.2
 * The OMX_CoreAlg header file contains extensions to the definitions used
 * by both the application and the component to access common items.
 */

#ifndef OMX_CoreAlg_h
#define OMX_CoreAlg_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Each OMX header shall include all required header files to allow the
 * header to compile without errors.  The includes below are required
 * for this header file to compile successfully
 */
#include <OMX_Core.h>

// This buffer already exist in OpenMax IL version 1.2 (3.7.3.7.1)
// Keep there names and values
#define OMX_BUFFERFLAG_TIMESTAMPINVALID 0x00000100
#define OMX_BUFFERFLAG_READONLY 0x00000200
#define OMX_BUFFERFLAG_ENDOFSUBFRAME 0x00000400
#define OMX_BUFFERFLAG_SKIPFRAME 0x00000800
// INTERLACED FLAG
#define OMX_ALG_BUFFERFLAG_TOP_FIELD 0x00001000
#define OMX_ALG_BUFFERFLAG_BOT_FIELD 0x00002000

/**
 * Vendor standard extension indices.
 * This enum lists the current AllegroDVT2 extension indices to OpenMAX IL.
 */

typedef enum OMX_ALG_ERRORTYPE
{
  OMX_ALG_ErrorVendorStartUnused = (OMX_S32)OMX_ErrorVendorStartUnused,

  OMX_ALG_ErrorNoChannelLeft = (OMX_S32)OMX_ErrorVendorStartUnused + 0x1000,
  OMX_ALG_ErrorChannelResourceUnavailable = (OMX_S32)OMX_ErrorVendorStartUnused + 0x1001,
  OMX_ALG_ErrorChannelLoadDistribution = (OMX_S32)OMX_ErrorVendorStartUnused + 0x1002,
  OMX_ALG_ErrorChannelHardwareCapacityExceeded = (OMX_S32)OMX_ErrorVendorStartUnused + 0x1003,

  OMX_ALG_ErrorMax = (OMX_S32) 0x9000FFFF,
}OMX_ALG_ERRORTYPE;

typedef enum OMX_ALG_EVENTTYPE
{
  OMX_ALG_EventVendorStartUnused = OMX_EventVendorStartUnused,

  /** OMX_ALG_EventSEIPrefixParsed should be called when a decoder parsed a prefix SEI
   * nData1: payload type
   * nData2: payload size
   * pEventData: payload buffer (OMX_U8*)
   */
  OMX_ALG_EventSEIPrefixParsed,

  /** OMX_ALG_EventSEISuffixParsed should be called when a decoder parsed a suffix SEI
   * nData1: payload type
   * nData2: payload size
   * pEventData: payload buffer (OMX_U8*)
   */
  OMX_ALG_EventSEISuffixParsed,

  /** OMX_ALG_EventResolutionChanged should be fired when a video codec detect a resolution changes
   * Stride and nSliceHeight shall not be modified and reallocation shall not be done. Component keeps the same buffers
   * nData1: width
   * nData2: height
   * pEventData: NULL
   */
  OMX_ALG_EventResolutionChanged,

  OMX_ALG_EventMax = OMX_EventMax,
}OMX_ALG_EVENTTYPE;

/**
 * Core device parameters
 *
 * STRUCT MEMBERS:
 *  nSize    : Size of the structure in bytes
 *  nVersion : OMX specification version information
 *  cDevice  : Device name
 */
typedef struct OMX_ALG_CORE_DEVICE
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_STRING cDevice;
}OMX_ALG_CORE_DEVICE;

typedef enum OMX_ALG_COREINDEXTYPE
{
  OMX_ALG_CoreIndexStartUnused = 0x01000000,
  OMX_ALG_CoreIndexUnused, /**< reference: NULL */
  OMX_ALG_CoreIndexDevice, /**< reference: OMX_ALG_CORE_DEVICE */
  OMX_ALG_CoreIndexExtensions = 0x6F000000, /**< Reserved region for introducing ALG Standard Extensions */

  /* Customer specific area */
  OMX_ALG_CoreIndexCustomerStartUnused = 0x7F000000,

  /* Customer specific structures should be in the range of 0x7F000000
     to 0x7FFFFFFE.  This range is not broken out by customer, so
     private indexes are not guaranteed unique and therefore should
     only be sent to the appropriate component. */
  OMX_ALG_CoreIndexMax = 0x7FFFFFFF
}OMX_ALG_COREINDEXTYPE;

/** The OMX_ALG_GetHandle method will locate the component specified by the
    component name given, load that component with specified settings into memory and then invoke
    the component's methods to create an instance of the component.

    The core should return from this call within 20 msec.

    @param [out] pHandle
        pointer to an OMX_HANDLETYPE pointer to be filled in by this method.
    @param [in] cComponentName
        pointer to a null terminated string with the component name.  The
        names of the components are strings less than 127 bytes in length
        plus the trailing null for a maximum size of 128 bytes.  An example
        of a valid component name is "OMX.TI.AUDIO.DSP.MIXER\0".  Names are
        assigned by the vendor, but shall start with "OMX." and then have
        the Vendor designation next.
    @param [in] pAppData
        pointer to an application defined value that will be returned
        during callbacks so that the application can identify the source
        of the callback.
    @param [in] pCallBacks
        pointer to a OMX_CALLBACKTYPE structure that will be passed to the
        component to initialize it with.
    @param [in] nCoreParamIndex
        Index of the core structure to be sent. This value is from the
        OMX_ALG_COREINDEXTYPE enumeration.
    @param [in] pSettings
        pointer to a specified strucutre defined by nCoreSettingsIndex
    @return OMX_ERRORTYPE
        If the command successfully executes, the return code will be
        OMX_ErrorNone.  Otherwise the appropriate OMX error will be returned.
    @ingroup core
 */
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_ALG_GetHandle(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN OMX_STRING cComponentName, OMX_IN OMX_PTR pAppData, OMX_IN OMX_CALLBACKTYPE* pCallBacks, OMX_IN OMX_ALG_COREINDEXTYPE nCoreParamIndex, OMX_IN OMX_PTR pSettings);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OMX_CoreAlg_h */
/* File EOF */

