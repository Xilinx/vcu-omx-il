/******************************************************************************
*
* Copyright (C) 2019 Allegro DVT2.  All rights reserved.
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

#include "omx_expertise_avc.h"

using namespace std;

static OMX_ERRORTYPE SetMediaProfileLevel(OMX_VIDEO_AVCPROFILETYPE const& profile, OMX_VIDEO_AVCLEVELTYPE const& level, std::shared_ptr<MediatypeInterface> media)
{
  ProfileLevel p;
  p.profile.avc = ConvertOMXToMediaAVCProfileLevel(profile, level).profile.avc;
  p.level = ConvertOMXToMediaAVCProfileLevel(profile, level).level;

  auto ret = media->Set(SETTINGS_INDEX_PROFILE_LEVEL, &p);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

static OMX_ERRORTYPE SetMediaGop(OMX_U32 bFrames, OMX_U32 pFrames, std::shared_ptr<MediatypeInterface> media)
{
  Gop gop;
  auto ret = media->Get(SETTINGS_INDEX_GROUP_OF_PICTURES, &gop);

  if(ret == MediatypeInterface::BAD_INDEX)
    return OMX_ErrorUnsupportedIndex;

  gop.b = ConvertOMXToMediaBFrames(bFrames, pFrames);
  gop.length = ConvertOMXToMediaGopLength(bFrames, pFrames);

  ret = media->Set(SETTINGS_INDEX_GROUP_OF_PICTURES, &gop);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

static OMX_ERRORTYPE SetMediaEntropyCoding(OMX_BOOL entropyCoding, std::shared_ptr<MediatypeInterface> media)
{
  EntropyCodingType e = ConvertOMXToMediaEntropyCoding(entropyCoding);

  auto ret = media->Set(SETTINGS_INDEX_ENTROPY_CODING, &e);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

static OMX_ERRORTYPE SetMediaConstrainedIntraPrediction(OMX_BOOL constrainedIntraPrediction, std::shared_ptr<MediatypeInterface> media)
{
  bool b = ConvertOMXToMediaBool(constrainedIntraPrediction);

  auto ret = media->Set(SETTINGS_INDEX_CONSTRAINED_INTRA_PREDICTION, &b);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

static OMX_ERRORTYPE SetMediaLoopFilter(OMX_VIDEO_AVCLOOPFILTERTYPE const& loopFilter, std::shared_ptr<MediatypeInterface> media)
{
  LoopFilterType l = ConvertOMXToMediaAVCLoopFilter(loopFilter);

  auto ret = media->Set(SETTINGS_INDEX_LOOP_FILTER, &l);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ExpertiseAVC::GetProfileLevelSupported(OMX_PTR param, std::shared_ptr<MediatypeInterface> media)
{
  vector<ProfileLevel> supported;
  auto ret = media->Get(SETTINGS_INDEX_PROFILES_LEVELS_SUPPORTED, &supported);
  OMX_CHECK_MEDIA_GET(ret);
  auto& pl = *(OMX_VIDEO_PARAM_PROFILELEVELTYPE*)param;

  if(pl.nProfileIndex >= supported.size())
    return OMX_ErrorNoMore;

  pl.eProfile = ConvertMediaToOMXAVCProfile(supported[pl.nProfileIndex]);
  pl.eLevel = ConvertMediaToOMXAVCLevel(supported[pl.nProfileIndex]);

  return OMX_ErrorNone;
}

OMX_ERRORTYPE ExpertiseAVC::GetProfileLevel(OMX_PTR param, Port const& port, std::shared_ptr<MediatypeInterface> media)
{
  ProfileLevel profileLevel;
  auto ret = media->Get(SETTINGS_INDEX_PROFILE_LEVEL, &profileLevel);
  OMX_CHECK_MEDIA_GET(ret);
  auto& pl = *(OMX_VIDEO_PARAM_PROFILELEVELTYPE*)param;
  pl.nPortIndex = port.index;
  pl.eProfile = ConvertMediaToOMXAVCProfile(profileLevel);
  pl.eLevel = ConvertMediaToOMXAVCLevel(profileLevel);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ExpertiseAVC::SetProfileLevel(OMX_PTR param, Port const& port, std::shared_ptr<MediatypeInterface> media)
{
  OMX_VIDEO_PARAM_PROFILELEVELTYPE rollback;
  GetProfileLevel(&rollback, port, media);
  auto pl = *(OMX_VIDEO_PARAM_PROFILELEVELTYPE*)param;
  auto profile = static_cast<OMX_VIDEO_AVCPROFILETYPE>(pl.eProfile);
  auto level = static_cast<OMX_VIDEO_AVCLEVELTYPE>(pl.eLevel);

  auto ret = SetMediaProfileLevel(profile, level, media);

  if(ret != OMX_ErrorNone)
  {
    SetProfileLevel(&rollback, port, media);
    throw ret;
  }

  return OMX_ErrorNone;
}

OMX_ERRORTYPE ExpertiseAVC::GetExpertise(OMX_PTR param, Port const& port, std::shared_ptr<MediatypeInterface> media)
{
  ProfileLevel profileLevel;
  Gop gop;
  EntropyCodingType entropyCoding;
  bool isConstrainedIntraPrediction;
  LoopFilterType loopFilter;
  auto ret = media->Get(SETTINGS_INDEX_PROFILE_LEVEL, &profileLevel);
  OMX_CHECK_MEDIA_GET(ret);
  bool bGop = (media->Get(SETTINGS_INDEX_GROUP_OF_PICTURES, &gop) == MediatypeInterface::SUCCESS);
  bool bEntropyCoding = (media->Get(SETTINGS_INDEX_ENTROPY_CODING, &entropyCoding) == MediatypeInterface::SUCCESS);
  bool bIntraPred = (media->Get(SETTINGS_INDEX_CONSTRAINED_INTRA_PREDICTION, &isConstrainedIntraPrediction) == MediatypeInterface::SUCCESS);
  bool bLoopFilter = (media->Get(SETTINGS_INDEX_LOOP_FILTER, &loopFilter) == MediatypeInterface::SUCCESS);
  auto& avc = *(OMX_VIDEO_PARAM_AVCTYPE*)param;
  avc.nPortIndex = port.index;
  avc.nBFrames = bGop ? ConvertMediaToOMXBFrames(gop) : 0;
  avc.nPFrames = bGop ? ConvertMediaToOMXPFrames(gop) : 0;
  avc.bUseHadamard = OMX_TRUE; // XXX
  avc.nRefFrames = 1; // XXX
  avc.nRefIdx10ActiveMinus1 = 0; // XXX
  avc.nRefIdx11ActiveMinus1 = 0; // XXX
  avc.bEnableUEP = OMX_FALSE; // XXX
  avc.bEnableFMO = OMX_FALSE; // XXX
  avc.bEnableASO = OMX_FALSE; // XXX
  avc.bEnableRS = OMX_FALSE; // XXX
  avc.eProfile = ConvertMediaToOMXAVCProfile(profileLevel);
  avc.eLevel = ConvertMediaToOMXAVCLevel(profileLevel);
  avc.nAllowedPictureTypes = OMX_VIDEO_PictureTypeI | OMX_VIDEO_PictureTypeP | OMX_VIDEO_PictureTypeB; // XXX
  avc.bFrameMBsOnly = OMX_TRUE; // XXX
  avc.bMBAFF = OMX_FALSE; // XXX
  avc.bEntropyCodingCABAC = bEntropyCoding ? ConvertMediaToOMXEntropyCoding(entropyCoding) : OMX_FALSE;
  avc.bWeightedPPrediction = OMX_FALSE; // XXX
  avc.nWeightedBipredicitonMode = OMX_FALSE; // XXX
  avc.bconstIpred = bIntraPred ? ConvertMediaToOMXBool(isConstrainedIntraPrediction) : OMX_FALSE;
  avc.bDirect8x8Inference = OMX_TRUE; // XXX
  avc.bDirectSpatialTemporal = OMX_TRUE; // XXX
  avc.nCabacInitIdc = 0; // XXX
  avc.eLoopFilterMode = bLoopFilter ? ConvertMediaToOMXAVCLoopFilter(loopFilter) : OMX_VIDEO_AVCLoopFilterMax;
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ExpertiseAVC::SetExpertise(OMX_PTR param, Port const& port, std::shared_ptr<MediatypeInterface> media)
{
  OMX_VIDEO_PARAM_AVCTYPE rollback;
  GetExpertise(&rollback, port, media);
  auto avc = *(OMX_VIDEO_PARAM_AVCTYPE*)param;

  auto ret = SetMediaProfileLevel(avc.eProfile, avc.eLevel, media);

  if(ret != OMX_ErrorNone)
  {
    SetExpertise(&rollback, port, media);
    throw ret;
  }

  // only encoder

  ret = SetMediaGop(avc.nBFrames, avc.nPFrames, media);

  if(ret == OMX_ErrorBadParameter)
  {
    SetExpertise(&rollback, port, media);
    throw ret;
  }

  ret = SetMediaEntropyCoding(avc.bEntropyCodingCABAC, media);

  if(ret == OMX_ErrorBadParameter)
  {
    SetExpertise(&rollback, port, media);
    throw ret;
  }

  ret = SetMediaConstrainedIntraPrediction(avc.bconstIpred, media);

  if(ret == OMX_ErrorBadParameter)
  {
    SetExpertise(&rollback, port, media);
    throw ret;
  }

  ret = SetMediaLoopFilter(avc.eLoopFilterMode, media);

  if(ret == OMX_ErrorBadParameter)
  {
    SetExpertise(&rollback, port, media);
    throw ret;
  }

  return OMX_ErrorNone;
}

