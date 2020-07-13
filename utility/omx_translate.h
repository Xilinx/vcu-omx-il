/******************************************************************************
*
* Copyright (C) 2016-2020 Allegro DVT2.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX OR ALLEGRO DVT2 BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of  Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
*
* Except as contained in this notice, the name of Allegro DVT2 shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Allegro DVT2.
*
******************************************************************************/

#pragma once

#include <OMX_CoreExt.h>
#include <OMX_IndexExt.h>
#include <map>
#include <string>
#include <stdexcept>

static std::map<OMX_ERRORTYPE, std::string> OMXErrorInStringMap =
{
  { OMX_ErrorNone, "OMX_ErrorNone" },
  { OMX_ErrorInsufficientResources, "OMX_ErrorInsufficientResources" },
  { OMX_ErrorUndefined, "OMX_ErrorUndefined" },
  { OMX_ErrorInvalidComponentName, "OMX_ErrorInvalidComponentName" },
  { OMX_ErrorComponentNotFound, "OMX_ErrorComponentNotFound" },
  { OMX_ErrorInvalidComponent, "OMX_ErrorInvalidComponent" },
  { OMX_ErrorBadParameter, "OMX_ErrorBadParameter" },
  { OMX_ErrorNotImplemented, "OMX_ErrorNotImplemented" },
  { OMX_ErrorUnderflow, "OMX_ErrorUnderflow" },
  { OMX_ErrorOverflow, "OMX_ErrorOverflow" },
  { OMX_ErrorHardware, "OMX_ErrorHardware" },
  { OMX_ErrorInvalidState, "OMX_ErrorInvalidState" },
  { OMX_ErrorStreamCorrupt, "OMX_ErrorStreamCorrupt" },
  { OMX_ErrorPortsNotCompatible, "OMX_ErrorPortsNotCompatible" },
  { OMX_ErrorResourcesLost, "OMX_ErrorResourcesLost" },
  { OMX_ErrorNoMore, "OMX_ErrorNoMore" },
  { OMX_ErrorVersionMismatch, "OMX_ErrorVersionMismatch" },
  { OMX_ErrorNotReady, "OMX_ErrorNotReady" },
  { OMX_ErrorTimeout, "OMX_ErrorTimeout" },
  { OMX_ErrorSameState, "OMX_ErrorSameState" },
  { OMX_ErrorResourcesPreempted, "OMX_ErrorResourcesPreempted" },
  { OMX_ErrorPortUnresponsiveDuringAllocation, "OMX_ErrorPortUnresponsiveDuringAllocation" },
  { OMX_ErrorPortUnresponsiveDuringDeallocation, "OMX_ErrorPortUnresponsiveDuringDeallocation" },
  { OMX_ErrorPortUnresponsiveDuringStop, "OMX_ErrorPortUnresponsiveDuringStop" },
  { OMX_ErrorIncorrectStateTransition, "OMX_ErrorIncorrectStateTransition" },
  { OMX_ErrorIncorrectStateOperation, "OMX_ErrorIncorrectStateOperation" },
  { OMX_ErrorUnsupportedSetting, "OMX_ErrorUnsupportedSetting" },
  { OMX_ErrorUnsupportedIndex, "OMX_ErrorUnsupportedIndex" },
  { OMX_ErrorBadPortIndex, "OMX_ErrorBadPortIndex" },
  { OMX_ErrorPortUnpopulated, "OMX_ErrorPortUnpopulated" },
  { OMX_ErrorComponentSuspended, "OMX_ErrorComponentSuspended" },
  { OMX_ErrorDynamicResourcesUnavailable, "OMX_ErrorDynamicResourcesUnavailable" },
  { OMX_ErrorMbErrorsInFrame, "OMX_ErrorMbErrorsInFrame" },
  { OMX_ErrorFormatNotDetected, "OMX_ErrorFormatNotDetected" },
  { OMX_ErrorContentPipeOpenFailed, "OMX_ErrorContentPipeOpenFailed" },
  { OMX_ErrorContentPipeCreationFailed, "OMX_ErrorContentPipeCreationFailed" },
  { OMX_ErrorSeperateTablesUsed, "OMX_ErrorSeperateTablesUsed" },
  { OMX_ErrorTunnelingUnsupported, "OMX_ErrorTunnelingUnsupported" },

  { OMX_ErrorKhronosExtensions, "OMX_ErrorKhronosExtensions" },
  { static_cast<OMX_ERRORTYPE>(OMX_ErrorInvalidMode), "OMX_ErrorInvalidMode" },

  { OMX_ErrorVendorStartUnused, "OMX_ErrorVendorStartUnused" },
  { static_cast<OMX_ERRORTYPE>(OMX_ALG_ErrorNoChannelLeft), "OMX_ALG_ErrorNoChannelLeft" },
  { static_cast<OMX_ERRORTYPE>(OMX_ALG_ErrorChannelResourceUnavailable), "OMX_ALG_ErrorChannelResourceUnavailable" },
  { static_cast<OMX_ERRORTYPE>(OMX_ALG_ErrorChannelResourceFragmented), "OMX_ALG_ErrorChannelResourceFragmented" },

  { OMX_ErrorMax, "OMX_ErrorMax" },
};

static inline std::string ToStringOMXError(OMX_ERRORTYPE error)
{
  try
  {
    return OMXErrorInStringMap.at(error);
  }
  catch(std::out_of_range const &)
  {
    return std::string {
             "Out-of-bound error"
    };
  }
}

static std::map<OMX_STATETYPE, std::string> OMXStateInStringMap =
{
  { OMX_StateInvalid, "OMX_StateInvalid" },
  { OMX_StateLoaded, "OMX_StateLoaded" },
  { OMX_StateIdle, "OMX_StateIdle" },
  { OMX_StateExecuting, "OMX_StateExecuting" },
  { OMX_StatePause, "OMX_StatePause" },
  { OMX_StateWaitForResources, "OMX_StateWaitForResources" },
};

static inline std::string ToStringOMXState(OMX_STATETYPE state)
{
  try
  {
    return OMXStateInStringMap.at(state);
  }
  catch(std::out_of_range const &)
  {
    return std::string {
             "Out-of-bound state"
    };
  }
}

static std::map<OMX_COMMANDTYPE, std::string> OMXCommandInStringMap =
{
  { OMX_CommandStateSet, "OMX_CommandStateSet" },
  { OMX_CommandFlush, "OMX_CommandFlush" },
  { OMX_CommandPortDisable, "OMX_CommandPortDisable" },
  { OMX_CommandPortEnable, "OMX_CommandPortEnable" },
  { OMX_CommandMarkBuffer, "OMX_CommandMarkBuffer" },
  { OMX_CommandKhronosExtensions, "OMX_CommandKhronosExtensions" },
  { OMX_CommandVendorStartUnused, "OMX_CommandVendorStartUnused" },
  { OMX_CommandMax, "OMX_CommandMax" },
};

static inline std::string ToStringOMXCommand(OMX_COMMANDTYPE command)
{
  try
  {
    return OMXCommandInStringMap.at(command);
  }
  catch(std::out_of_range const &)
  {
    return std::string {
             "Out-of-bound command"
    };
  }
}

static std::map<OMX_EVENTTYPE, std::string> OMXEventInStringMap =
{
  { OMX_EventCmdComplete, "OMX_EventCmdComplete" },
  { OMX_EventError, "OMX_EventError" },
  { OMX_EventMark, "OMX_EventMark" },
  { OMX_EventPortSettingsChanged, "OMX_EventPortSettingsChanged" },
  { OMX_EventBufferFlag, "OMX_EventBufferFlag" },
  { OMX_EventResourcesAcquired, "OMX_EventResourcesAcquired" },
  { OMX_EventComponentResumed, "OMX_EventComponentResumed" },
  { OMX_EventDynamicResourcesAvailable, "OMX_EventDynamicResourcesAvailable" },
  { OMX_EventPortFormatDetected, "OMX_EventPortFormatDetected" },

  { OMX_EventKhronosExtensions, "OMX_EventKhronosExtensions" },
  { static_cast<OMX_EVENTTYPE>(OMX_EventIndexSettingChanged), "OMX_EventIndexSettingChanged" },

  { OMX_EventVendorStartUnused, "OMX_EventVendorStartUnused" },
  { static_cast<OMX_EVENTTYPE>(OMX_ALG_EventSEIPrefixParsed), "OMX_ALG_EventSEIPrefixParsed" },
  { static_cast<OMX_EVENTTYPE>(OMX_ALG_EventSEISuffixParsed), "OMX_ALG_EventSEISuffixParsed" },
  { static_cast<OMX_EVENTTYPE>(OMX_ALG_EventResolutionChanged), "OMX_ALG_EventResolutionChanged" },

  { OMX_EventMax, "OMX_EventMax" },
};

static inline std::string ToStringOMXEvent(OMX_EVENTTYPE event)
{
  try
  {
    return OMXEventInStringMap.at(event);
  }
  catch(std::out_of_range const &)
  {
    return std::string {
             "Out-of-bound event"
    };
  }
}

static std::map<OMX_INDEXTYPE, std::string> OMXIndexInStringMap =
{
  { OMX_IndexComponentStartUnused, "OMX_IndexComponentStartUnused" },
  { OMX_IndexParamPriorityMgmt, "OMX_IndexParamPriorityMgmt" },
  { OMX_IndexParamAudioInit, "OMX_IndexParamAudioInit" },
  { OMX_IndexParamImageInit, "OMX_IndexParamImageInit" },
  { OMX_IndexParamVideoInit, "OMX_IndexParamVideoInit" },
  { OMX_IndexParamOtherInit, "OMX_IndexParamOtherInit" },
  { OMX_IndexParamNumAvailableStreams, "OMX_IndexParamNumAvailableStreams" },
  { OMX_IndexParamActiveStream, "OMX_IndexParamActiveStream" },
  { OMX_IndexParamSuspensionPolicy, "OMX_IndexParamSuspensionPolicy" },
  { OMX_IndexParamComponentSuspended, "OMX_IndexParamComponentSuspended" },
  { OMX_IndexConfigCapturing, "OMX_IndexConfigCapturing" },
  { OMX_IndexConfigCaptureMode, "OMX_IndexConfigCaptureMode" },
  { OMX_IndexAutoPauseAfterCapture, "OMX_IndexAutoPauseAfterCapture" },
  { OMX_IndexParamContentURI, "OMX_IndexParamContentURI" },
  { OMX_IndexParamCustomContentPipe, "OMX_IndexParamCustomContentPipe" },
  { OMX_IndexParamDisableResourceConcealment, "OMX_IndexParamDisableResourceConcealment" },
  { OMX_IndexConfigMetadataItemCount, "OMX_IndexConfigMetadataItemCount" },
  { OMX_IndexConfigContainerNodeCount, "OMX_IndexConfigContainerNodeCount" },
  { OMX_IndexConfigMetadataItem, "OMX_IndexConfigMetadataItem" },
  { OMX_IndexConfigCounterNodeID, "OMX_IndexConfigCounterNodeID" },
  { OMX_IndexParamMetadataFilterType, "OMX_IndexParamMetadataFilterType" },
  { OMX_IndexParamMetadataKeyFilter, "OMX_IndexParamMetadataKeyFilter" },
  { OMX_IndexConfigPriorityMgmt, "OMX_IndexConfigPriorityMgmt" },
  { OMX_IndexParamStandardComponentRole, "OMX_IndexParamStandardComponentRole" },
  { OMX_IndexPortStartUnused, "OMX_IndexPortStartUnused" },
  { OMX_IndexParamPortDefinition, "OMX_IndexParamPortDefinition" },
  { OMX_IndexParamCompBufferSupplier, "OMX_IndexParamCompBufferSupplier" },
  { OMX_IndexReservedStartUnused, "OMX_IndexReservedStartUnused" },
  { OMX_IndexAudioStartUnused, "OMX_IndexAudioStartUnused" },
  { OMX_IndexParamAudioPortFormat, "OMX_IndexParamAudioPortFormat" },
  { OMX_IndexParamAudioPcm, "OMX_IndexParamAudioPcm" },
  { OMX_IndexParamAudioAac, "OMX_IndexParamAudioAac" },
  { OMX_IndexParamAudioRa, "OMX_IndexParamAudioRa" },
  { OMX_IndexParamAudioMp3, "OMX_IndexParamAudioMp3" },
  { OMX_IndexParamAudioAdpcm, "OMX_IndexParamAudioAdpcm" },
  { OMX_IndexParamAudioG723, "OMX_IndexParamAudioG723" },
  { OMX_IndexParamAudioG729, "OMX_IndexParamAudioG729" },
  { OMX_IndexParamAudioAmr, "OMX_IndexParamAudioAmr" },
  { OMX_IndexParamAudioWma, "OMX_IndexParamAudioWma" },
  { OMX_IndexParamAudioSbc, "OMX_IndexParamAudioSbc" },
  { OMX_IndexParamAudioMidi, "OMX_IndexParamAudioMidi" },
  { OMX_IndexParamAudioGsm_FR, "OMX_IndexParamAudioGsm_FR" },
  { OMX_IndexParamAudioMidiLoadUserSound, "OMX_IndexParamAudioMidiLoadUserSound" },
  { OMX_IndexParamAudioG726, "OMX_IndexParamAudioG726" },
  { OMX_IndexParamAudioGsm_EFR, "OMX_IndexParamAudioGsm_EFR" },
  { OMX_IndexParamAudioGsm_HR, "OMX_IndexParamAudioGsm_HR" },
  { OMX_IndexParamAudioPdc_FR, "OMX_IndexParamAudioPdc_FR" },
  { OMX_IndexParamAudioPdc_EFR, "OMX_IndexParamAudioPdc_EFR" },
  { OMX_IndexParamAudioPdc_HR, "OMX_IndexParamAudioPdc_HR" },
  { OMX_IndexParamAudioTdma_FR, "OMX_IndexParamAudioTdma_FR" },
  { OMX_IndexParamAudioTdma_EFR, "OMX_IndexParamAudioTdma_EFR" },
  { OMX_IndexParamAudioQcelp8, "OMX_IndexParamAudioQcelp8" },
  { OMX_IndexParamAudioQcelp13, "OMX_IndexParamAudioQcelp13" },
  { OMX_IndexParamAudioEvrc, "OMX_IndexParamAudioEvrc" },
  { OMX_IndexParamAudioSmv, "OMX_IndexParamAudioSmv" },
  { OMX_IndexParamAudioVorbis, "OMX_IndexParamAudioVorbis" },
  { OMX_IndexConfigAudioMidiImmediateEvent, "OMX_IndexConfigAudioMidiImmediateEvent" },
  { OMX_IndexConfigAudioMidiControl, "OMX_IndexConfigAudioMidiControl" },
  { OMX_IndexConfigAudioMidiSoundBankProgram, "OMX_IndexConfigAudioMidiSoundBankProgram" },
  { OMX_IndexConfigAudioMidiStatus, "OMX_IndexConfigAudioMidiStatus" },
  { OMX_IndexConfigAudioMidiMetaEvent, "OMX_IndexConfigAudioMidiMetaEvent" },
  { OMX_IndexConfigAudioMidiMetaEventData, "OMX_IndexConfigAudioMidiMetaEventData" },
  { OMX_IndexConfigAudioVolume, "OMX_IndexConfigAudioVolume" },
  { OMX_IndexConfigAudioBalance, "OMX_IndexConfigAudioBalance" },
  { OMX_IndexConfigAudioChannelMute, "OMX_IndexConfigAudioChannelMute" },
  { OMX_IndexConfigAudioMute, "OMX_IndexConfigAudioMute" },
  { OMX_IndexConfigAudioLoudness, "OMX_IndexConfigAudioLoudness" },
  { OMX_IndexConfigAudioEchoCancelation, "OMX_IndexConfigAudioEchoCancelation" },
  { OMX_IndexConfigAudioNoiseReduction, "OMX_IndexConfigAudioNoiseReduction" },
  { OMX_IndexConfigAudioBass, "OMX_IndexConfigAudioBass" },
  { OMX_IndexConfigAudioTreble, "OMX_IndexConfigAudioTreble" },
  { OMX_IndexConfigAudioStereoWidening, "OMX_IndexConfigAudioStereoWidening" },
  { OMX_IndexConfigAudioChorus, "OMX_IndexConfigAudioChorus" },
  { OMX_IndexConfigAudioEqualizer, "OMX_IndexConfigAudioEqualizer" },
  { OMX_IndexConfigAudioReverberation, "OMX_IndexConfigAudioReverberation" },
  { OMX_IndexConfigAudioChannelVolume, "OMX_IndexConfigAudioChannelVolume" },
  { OMX_IndexImageStartUnused, "OMX_IndexImageStartUnused" },
  { OMX_IndexParamImagePortFormat, "OMX_IndexParamImagePortFormat" },
  { OMX_IndexParamFlashControl, "OMX_IndexParamFlashControl" },
  { OMX_IndexConfigFocusControl, "OMX_IndexConfigFocusControl" },
  { OMX_IndexParamQFactor, "OMX_IndexParamQFactor" },
  { OMX_IndexParamQuantizationTable, "OMX_IndexParamQuantizationTable" },
  { OMX_IndexParamHuffmanTable, "OMX_IndexParamHuffmanTable" },
  { OMX_IndexConfigFlashControl, "OMX_IndexConfigFlashControl" },
  { OMX_IndexVideoStartUnused, "OMX_IndexVideoStartUnused" },
  { OMX_IndexParamVideoPortFormat, "OMX_IndexParamVideoPortFormat" },
  { OMX_IndexParamVideoQuantization, "OMX_IndexParamVideoQuantization" },
  { OMX_IndexParamVideoFastUpdate, "OMX_IndexParamVideoFastUpdate" },
  { OMX_IndexParamVideoBitrate, "OMX_IndexParamVideoBitrate" },
  { OMX_IndexParamVideoMotionVector, "OMX_IndexParamVideoMotionVector" },
  { OMX_IndexParamVideoIntraRefresh, "OMX_IndexParamVideoIntraRefresh" },
  { OMX_IndexParamVideoErrorCorrection, "OMX_IndexParamVideoErrorCorrection" },
  { OMX_IndexParamVideoVBSMC, "OMX_IndexParamVideoVBSMC" },
  { OMX_IndexParamVideoMpeg2, "OMX_IndexParamVideoMpeg2" },
  { OMX_IndexParamVideoMpeg4, "OMX_IndexParamVideoMpeg4" },
  { OMX_IndexParamVideoWmv, "OMX_IndexParamVideoWmv" },
  { OMX_IndexParamVideoRv, "OMX_IndexParamVideoRv" },
  { OMX_IndexParamVideoAvc, "OMX_IndexParamVideoAvc" },
  { OMX_IndexParamVideoH263, "OMX_IndexParamVideoH263" },
  { OMX_IndexParamVideoProfileLevelQuerySupported, "OMX_IndexParamVideoProfileLevelQuerySupported" },
  { OMX_IndexParamVideoProfileLevelCurrent, "OMX_IndexParamVideoProfileLevelCurrent" },
  { OMX_IndexConfigVideoBitrate, "OMX_IndexConfigVideoBitrate" },
  { OMX_IndexConfigVideoFramerate, "OMX_IndexConfigVideoFramerate" },
  { OMX_IndexConfigVideoIntraVOPRefresh, "OMX_IndexConfigVideoIntraVOPRefresh" },
  { OMX_IndexConfigVideoIntraMBRefresh, "OMX_IndexConfigVideoIntraMBRefresh" },
  { OMX_IndexConfigVideoMBErrorReporting, "OMX_IndexConfigVideoMBErrorReporting" },
  { OMX_IndexParamVideoMacroblocksPerFrame, "OMX_IndexParamVideoMacroblocksPerFrame" },
  { OMX_IndexConfigVideoMacroBlockErrorMap, "OMX_IndexConfigVideoMacroBlockErrorMap" },
  { OMX_IndexParamVideoSliceFMO, "OMX_IndexParamVideoSliceFMO" },
  { OMX_IndexConfigVideoAVCIntraPeriod, "OMX_IndexConfigVideoAVCIntraPeriod" },
  { OMX_IndexConfigVideoNalSize, "OMX_IndexConfigVideoNalSize" },
  { OMX_IndexCommonStartUnused, "OMX_IndexCommonStartUnused" },
  { OMX_IndexParamCommonDeblocking, "OMX_IndexParamCommonDeblocking" },
  { OMX_IndexParamCommonSensorMode, "OMX_IndexParamCommonSensorMode" },
  { OMX_IndexParamCommonInterleave, "OMX_IndexParamCommonInterleave" },
  { OMX_IndexConfigCommonColorFormatConversion, "OMX_IndexConfigCommonColorFormatConversion" },
  { OMX_IndexConfigCommonScale, "OMX_IndexConfigCommonScale" },
  { OMX_IndexConfigCommonImageFilter, "OMX_IndexConfigCommonImageFilter" },
  { OMX_IndexConfigCommonColorEnhancement, "OMX_IndexConfigCommonColorEnhancement" },
  { OMX_IndexConfigCommonColorKey, "OMX_IndexConfigCommonColorKey" },
  { OMX_IndexConfigCommonColorBlend, "OMX_IndexConfigCommonColorBlend" },
  { OMX_IndexConfigCommonFrameStabilisation, "OMX_IndexConfigCommonFrameStabilisation" },
  { OMX_IndexConfigCommonRotate, "OMX_IndexConfigCommonRotate" },
  { OMX_IndexConfigCommonMirror, "OMX_IndexConfigCommonMirror" },
  { OMX_IndexConfigCommonOutputPosition, "OMX_IndexConfigCommonOutputPosition" },
  { OMX_IndexConfigCommonInputCrop, "OMX_IndexConfigCommonInputCrop" },
  { OMX_IndexConfigCommonOutputCrop, "OMX_IndexConfigCommonOutputCrop" },
  { OMX_IndexConfigCommonDigitalZoom, "OMX_IndexConfigCommonDigitalZoom" },
  { OMX_IndexConfigCommonOpticalZoom, "OMX_IndexConfigCommonOpticalZoom" },
  { OMX_IndexConfigCommonWhiteBalance, "OMX_IndexConfigCommonWhiteBalance" },
  { OMX_IndexConfigCommonExposure, "OMX_IndexConfigCommonExposure" },
  { OMX_IndexConfigCommonContrast, "OMX_IndexConfigCommonContrast" },
  { OMX_IndexConfigCommonBrightness, "OMX_IndexConfigCommonBrightness" },
  { OMX_IndexConfigCommonBacklight, "OMX_IndexConfigCommonBacklight" },
  { OMX_IndexConfigCommonGamma, "OMX_IndexConfigCommonGamma" },
  { OMX_IndexConfigCommonSaturation, "OMX_IndexConfigCommonSaturation" },
  { OMX_IndexConfigCommonLightness, "OMX_IndexConfigCommonLightness" },
  { OMX_IndexConfigCommonExclusionRect, "OMX_IndexConfigCommonExclusionRect" },
  { OMX_IndexConfigCommonDithering, "OMX_IndexConfigCommonDithering" },
  { OMX_IndexConfigCommonPlaneBlend, "OMX_IndexConfigCommonPlaneBlend" },
  { OMX_IndexConfigCommonExposureValue, "OMX_IndexConfigCommonExposureValue" },
  { OMX_IndexConfigCommonOutputSize, "OMX_IndexConfigCommonOutputSize" },
  { OMX_IndexParamCommonExtraQuantData, "OMX_IndexParamCommonExtraQuantData" },
  { OMX_IndexConfigCommonFocusRegion, "OMX_IndexConfigCommonFocusRegion" },
  { OMX_IndexConfigCommonFocusStatus, "OMX_IndexConfigCommonFocusStatus" },
  { OMX_IndexConfigCommonTransitionEffect, "OMX_IndexConfigCommonTransitionEffect" },
  { OMX_IndexOtherStartUnused, "OMX_IndexOtherStartUnused" },
  { OMX_IndexParamOtherPortFormat, "OMX_IndexParamOtherPortFormat" },
  { OMX_IndexConfigOtherPower, "OMX_IndexConfigOtherPower" },
  { OMX_IndexConfigOtherStats, "OMX_IndexConfigOtherStats" },
  { OMX_IndexTimeStartUnused, "OMX_IndexTimeStartUnused" },
  { OMX_IndexConfigTimeScale, "OMX_IndexConfigTimeScale" },
  { OMX_IndexConfigTimeClockState, "OMX_IndexConfigTimeClockState" },
  { OMX_IndexConfigTimeActiveRefClock, "OMX_IndexConfigTimeActiveRefClock" },
  { OMX_IndexConfigTimeCurrentMediaTime, "OMX_IndexConfigTimeCurrentMediaTime" },
  { OMX_IndexConfigTimeCurrentWallTime, "OMX_IndexConfigTimeCurrentWallTime" },
  { OMX_IndexConfigTimeCurrentAudioReference, "OMX_IndexConfigTimeCurrentAudioReference" },
  { OMX_IndexConfigTimeCurrentVideoReference, "OMX_IndexConfigTimeCurrentVideoReference" },
  { OMX_IndexConfigTimeMediaTimeRequest, "OMX_IndexConfigTimeMediaTimeRequest" },
  { OMX_IndexConfigTimeClientStartTime, "OMX_IndexConfigTimeClientStartTime" },
  { OMX_IndexConfigTimePosition, "OMX_IndexConfigTimePosition" },
  { OMX_IndexConfigTimeSeekMode, "OMX_IndexConfigTimeSeekMode" },

  { OMX_IndexKhronosExtensions, "OMX_IndexKhronosExtensions" },
  { static_cast<OMX_INDEXTYPE>(OMX_IndexExtComponentStartUnused), "OMX_IndexExtComponentStartUnused" },
  { static_cast<OMX_INDEXTYPE>(OMX_IndexConfigCallbackRequest), "OMX_IndexConfigCallbackRequest" },
  { static_cast<OMX_INDEXTYPE>(OMX_IndexConfigCommitMode), "OMX_IndexConfigCommitMode" },
  { static_cast<OMX_INDEXTYPE>(OMX_IndexConfigCommit), "OMX_IndexConfigCommit" },
  { static_cast<OMX_INDEXTYPE>(OMX_IndexExtPortStartUnused), "OMX_IndexExtPortStartUnused" },
  { static_cast<OMX_INDEXTYPE>(OMX_IndexExtAudioStartUnused), "OMX_IndexExtAudioStartUnused" },
  { static_cast<OMX_INDEXTYPE>(OMX_IndexExtImageStartUnused), "OMX_IndexExtImageStartUnused" },
  { static_cast<OMX_INDEXTYPE>(OMX_IndexExtVideoStartUnused), "OMX_IndexExtVideoStartUnused" },
  { static_cast<OMX_INDEXTYPE>(OMX_IndexParamNalStreamFormatSupported), "OMX_IndexParamNalStreamFormatSupported" },
  { static_cast<OMX_INDEXTYPE>(OMX_IndexParamNalStreamFormat), "OMX_IndexParamNalStreamFormat" },
  { static_cast<OMX_INDEXTYPE>(OMX_IndexParamNalStreamFormatSelect), "OMX_IndexParamNalStreamFormatSelect" },
  { static_cast<OMX_INDEXTYPE>(OMX_IndexParamVideoVp8), "OMX_IndexParamVideoVp8" },
  { static_cast<OMX_INDEXTYPE>(OMX_IndexConfigVideoVp8ReferenceFrame), "OMX_IndexConfigVideoVp8ReferenceFrame" },
  { static_cast<OMX_INDEXTYPE>(OMX_IndexConfigVideoVp8ReferenceFrameType), "OMX_IndexConfigVideoVp8ReferenceFrameType" },
  { static_cast<OMX_INDEXTYPE>(OMX_IndexExtCommonStartUnused), "OMX_IndexExtCommonStartUnused" },
  { static_cast<OMX_INDEXTYPE>(OMX_IndexExtOtherStartUnused), "OMX_IndexExtOtherStartUnused" },
  { static_cast<OMX_INDEXTYPE>(OMX_IndexExtTimeStartUnused), "OMX_IndexExtTimeStartUnused" },

  { OMX_IndexVendorStartUnused, "OMX_IndexVendorStartUnused" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexVendorComponentStartUnused), "OMX_ALG_IndexVendorComponentStartUnused" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamReportedLatency), "OMX_ALG_IndexParamReportedLatency" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamPreallocation), "OMX_ALG_IndexParamPreallocation" },

  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexVendorPortStartUnused), "OMX_ALG_IndexVendorPortStartUnused" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexPortParamBufferMode), "OMX_ALG_IndexPortParamBufferMode" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexPortParamEarlyCallback), "OMX_ALG_IndexPortParamEarlyCallback" },

  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVendorVideoStartUnused), "OMX_ALG_IndexParamVendorVideoStartUnused" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoHevc), "OMX_ALG_IndexParamVideoHevc" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoVp9), "OMX_ALG_IndexParamVideoVp9" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoGopControl), "OMX_ALG_IndexParamVideoGopControl" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoSlices), "OMX_ALG_IndexParamVideoSlices" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoSceneChangeResilience), "OMX_ALG_IndexParamVideoSceneChangeResilience" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoPrefetchBuffer), "OMX_ALG_IndexParamVideoPrefetchBuffer" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoCodedPictureBuffer), "OMX_ALG_IndexParamVideoCodedPictureBuffer" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoQuantizationControl), "OMX_ALG_IndexParamVideoQuantizationControl" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoQuantizationExtension), "OMX_ALG_IndexParamVideoQuantizationExtension" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoScalingList), "OMX_ALG_IndexParamVideoScalingList" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoDecodedPictureBuffer), "OMX_ALG_IndexParamVideoDecodedPictureBuffer" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoInternalEntropyBuffers), "OMX_ALG_IndexParamVideoInternalEntropyBuffers" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoLowBandwidth), "OMX_ALG_IndexParamVideoLowBandwidth" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoAspectRatio), "OMX_ALG_IndexParamVideoAspectRatio" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoSubframe), "OMX_ALG_IndexParamVideoSubframe" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoInstantaneousDecodingRefresh), "OMX_ALG_IndexParamVideoInstantaneousDecodingRefresh" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoMaxBitrate), "OMX_ALG_IndexParamVideoMaxBitrate" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoFillerData), "OMX_ALG_IndexParamVideoFillerData" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoBufferMode), "OMX_ALG_IndexParamVideoBufferMode" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoInterlaceFormatCurrent), "OMX_ALG_IndexParamVideoInterlaceFormatCurrent" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoInterlaceFormatSupported), "OMX_ALG_IndexParamVideoInterlaceFormatSupported" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoLongTerm), "OMX_ALG_IndexParamVideoLongTerm" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoLookAhead), "OMX_ALG_IndexParamVideoLookAhead" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoTwoPass), "OMX_ALG_IndexParamVideoTwoPass" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoColorPrimaries), "OMX_ALG_IndexParamVideoColorPrimaries" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoTransferCharacteristics), "OMX_ALG_IndexParamVideoTransferCharacteristics" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoColorMatrix), "OMX_ALG_IndexParamVideoColorMatrix" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoInputParsed), "OMX_ALG_IndexParamVideoInputParsed" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoMaxPictureSize), "OMX_ALG_IndexParamVideoMaxPictureSize" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoMaxPictureSizes), "OMX_ALG_IndexParamVideoMaxPictureSizes" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoLoopFilterBeta), "OMX_ALG_IndexParamVideoLoopFilterBeta" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoLoopFilterTc), "OMX_ALG_IndexParamVideoLoopFilterTc" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoQuantizationTable), "OMX_ALG_IndexParamVideoQuantizationTable" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoAccessUnitDelimiter), "OMX_ALG_IndexParamVideoAccessUnitDelimiter" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoBufferingPeriodSEI), "OMX_ALG_IndexParamVideoBufferingPeriodSEI" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoPictureTimingSEI), "OMX_ALG_IndexParamVideoPictureTimingSEI" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoRecoveryPointSEI), "OMX_ALG_IndexParamVideoRecoveryPointSEI" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoMasteringDisplayColourVolumeSEI), "OMX_ALG_IndexParamVideoMasteringDisplayColourVolumeSEI" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoContentLightLevelSEI), "OMX_ALG_IndexParamVideoContentLightLevelSEI" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoST209410SEI), "OMX_ALG_IndexParamVideoST209410SEI" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoST209440SEI), "OMX_ALG_IndexParamVideoST209440SEI" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoRateControlPlugin), "OMX_ALG_IndexParamVideoRateControlPlugin" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoCrop), "OMX_ALG_IndexParamVideoCrop" },

  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVendorVideoStartUnused), "OMX_ALG_IndexConfigVendorVideoStartUnused" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoInsertInstantaneousDecodingRefresh), "OMX_ALG_IndexConfigVideoInsertInstantaneousDecodingRefresh" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoGroupOfPictures), "OMX_ALG_IndexConfigVideoGroupOfPictures" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoRegionOfInterest), "OMX_ALG_IndexConfigVideoRegionOfInterest" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoRegionOfInterestByValue), "OMX_ALG_IndexConfigVideoRegionOfInterestByValue" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoNotifySceneChange), "OMX_ALG_IndexConfigVideoNotifySceneChange" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoInsertLongTerm), "OMX_ALG_IndexConfigVideoInsertLongTerm" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoUseLongTerm), "OMX_ALG_IndexConfigVideoUseLongTerm" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoNotifyResolutionChange), "OMX_ALG_IndexConfigVideoNotifyResolutionChange" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoInsertPrefixSEI), "OMX_ALG_IndexConfigVideoInsertPrefixSEI" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoInsertSuffixSEI), "OMX_ALG_IndexConfigVideoInsertSuffixSEI" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoQuantizationParameterTable), "OMX_ALG_IndexConfigVideoQuantizationParameterTable" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoLoopFilterBeta), "OMX_ALG_IndexConfigVideoLoopFilterBeta" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoLoopFilterTc), "OMX_ALG_IndexConfigVideoLoopFilterTc" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoTransferCharacteristics), "OMX_ALG_IndexConfigVideoTransferCharacteristics" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoColorMatrix), "OMX_ALG_IndexConfigVideoColorMatrix" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoColorPrimaries), "OMX_ALG_IndexConfigVideoColorPrimaries" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoHighDynamicRangeSEI), "OMX_ALG_IndexConfigVideoHighDynamicRangeSEI" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoMaxResolutionChange), "OMX_ALG_IndexConfigVideoMaxResolutionChange" },

  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexVendorCommonStartUnused), "OMX_ALG_IndexVendorCommonStartUnused" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamCommonSequencePictureModeCurrent), "OMX_ALG_IndexParamCommonSequencePictureModeCurrent" },
  { static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamCommonSequencePictureModeQuerySupported), "OMX_ALG_IndexParamCommonSequencePictureModeQuerySupported" },

  { OMX_IndexMax, "OMX_IndexMax" },
};

static inline std::string ToStringOMXIndex(OMX_INDEXTYPE index)
{
  try
  {
    return OMXIndexInStringMap.at(index);
  }
  catch(std::out_of_range const &)
  {
    return std::string {
             "Out-of-bound index"
    };
  }
}

