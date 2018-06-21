/******************************************************************************
*
* Copyright (C) 2017 Allegro DVT2.  All rights reserved.
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

#include "omx_settings_enc_hevc.h"
#include "omx_settings_enc_common.h"
#include "base/omx_mediatype/omx_mediatype_common_hevc.h"
#include "base/omx_mediatype/omx_mediatype_common.h"

#include "omx_settings_checks.h"
#include "omx_settings_utils.h"

#include "base/omx_utils/round.h"

extern "C"
{
#include <lib_common_enc/EncBuffers.h>
}

using namespace std;

EncSettingsHEVC::EncSettingsHEVC()
{
  Reset();
}

EncSettingsHEVC::~EncSettingsHEVC()
{
}

ErrorSettingsType EncSettingsHEVC::Reset()
{
  alignments.stride = 32;
  alignments.sliceHeight = 8;

  bufferHandles.input = BufferHandleType::BUFFER_HANDLE_CHAR_PTR;
  bufferHandles.output = BufferHandleType::BUFFER_HANDLE_CHAR_PTR;
  bufferBytesAlignments.input = 0;
  bufferBytesAlignments.output = 0;
  bufferContiguities.input = false;
  bufferContiguities.output = false;

  AL_Settings_SetDefaults(&settings);
  auto& channel = settings.tChParam[0];
  channel.eProfile = AL_PROFILE_HEVC_MAIN;
  AL_Settings_SetDefaultParam(&settings);
  channel.uLevel = 10;
  channel.uWidth = 176;
  channel.uHeight = 144;
  channel.ePicFormat = AL_420_8BITS;
  auto& rateCtrl = channel.tRCParam;
  rateCtrl.eRCMode = AL_RC_CBR;
  rateCtrl.eOptions = AL_RC_OPT_SCN_CHG_RES;
  rateCtrl.uMaxBitRate = rateCtrl.uTargetBitRate = 64000;
  rateCtrl.uFrameRate = 15;

  settings.bEnableAUD = false;
  settings.iPrefetchLevel2 = 0;
  return ERROR_SETTINGS_NONE;
}

ErrorSettingsType EncSettingsHEVC::Set(string index, void const* settings)
{
  if(!settings)
    return ERROR_SETTINGS_BAD_PARAMETER;

  if(index == SETTINGS_INDEX_RESOLUTION)
  {
    if(!UpdateResolution(this->settings, this->alignments, *(static_cast<Resolution const*>(settings))))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == SETTINGS_INDEX_FORMAT)
  {
    if(!UpdateFormat(this->settings, *(static_cast<Format const*>(settings))))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == SETTINGS_INDEX_PROFILE_LEVEL)
    return ERROR_SETTINGS_NOT_IMPLEMENTED;

  return ERROR_SETTINGS_BAD_INDEX;
}

ErrorSettingsType EncSettingsHEVC::Get(string index, void* settings)
{
  if(!settings)
    return ERROR_SETTINGS_BAD_PARAMETER;

  if(index == SETTINGS_INDEX_RESOLUTION)
  {
    *(static_cast<Resolution*>(settings)) = CreateResolution(this->settings, this->alignments);
    return ERROR_SETTINGS_NONE;
  }

  if(index == SETTINGS_INDEX_FORMAT)
  {
    *(static_cast<Format*>(settings)) = CreateFormat(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == SETTINGS_INDEX_FORMATS_SUPPORTED)
  {
    *(static_cast<vector<Format>*>(settings)) = CreateFormatsSupported(this->colors, this->bitdepths);
    return ERROR_SETTINGS_NONE;
  }

  if(index == SETTINGS_INDEX_PROFILE_LEVEL)
  {
    *(static_cast<ProfileLevelType*>(settings)) = this->settings.tChParam[0].uTier ? CreateHEVCHighTierProfileLevel(this->settings.tChParam[0].eProfile, this->settings.tChParam[0].uLevel) : CreateHEVCMainTierProfileLevel(this->settings.tChParam[0].eProfile, this->settings.tChParam[0].uLevel);
    return ERROR_SETTINGS_NONE;
  }

  if(index == SETTINGS_INDEX_BUFFER_MODE)
  {
    *(static_cast<BufferModeType*>(settings)) = CreateBufferMode(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == SETTINGS_INDEX_BUFFER_HANDLES)
  {
    *(static_cast<BufferHandles*>(settings)) = this->bufferHandles;
    return ERROR_SETTINGS_NONE;
  }

  if(index == SETTINGS_INDEX_BUFFER_SIZES)
  {
    *(static_cast<BufferSizes*>(settings)) = CreateBufferSizes(this->settings, this->alignments);
    return ERROR_SETTINGS_NONE;
  }

  if(index == SETTINGS_INDEX_BUFFER_BYTES_ALIGNMENTS)
  {
    *(static_cast<BufferBytesAlignments*>(settings)) = this->bufferBytesAlignments;
    return ERROR_SETTINGS_NONE;
  }

  if(index == SETTINGS_INDEX_BUFFER_CONTIGUITIES)
  {
    *(static_cast<BufferContiguities*>(settings)) = this->bufferContiguities;
    return ERROR_SETTINGS_NONE;
  }

  return ERROR_SETTINGS_BAD_INDEX;
}

