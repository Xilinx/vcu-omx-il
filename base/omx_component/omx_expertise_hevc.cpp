/******************************************************************************
*
* Copyright (C) 2015-2023 Allegro DVT2
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
******************************************************************************/

#include "omx_expertise_hevc.h"
#include "omx_convert_omx_media.h"

using namespace std;

ExpertiseHEVC::~ExpertiseHEVC() = default;

static OMX_ERRORTYPE SetMediaProfileLevel(OMX_ALG_VIDEO_HEVCPROFILETYPE const& profile, OMX_ALG_VIDEO_HEVCLEVELTYPE const& level, std::shared_ptr<SettingsInterface> media)
{
  ProfileLevel p;
  p.profile.hevc = ConvertOMXToMediaHEVCProfileLevel(profile, level).profile.hevc;
  p.level = ConvertOMXToMediaHEVCProfileLevel(profile, level).level;

  auto ret = media->Set(SETTINGS_INDEX_PROFILE_LEVEL, &p);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

static OMX_ERRORTYPE SetMediaGop(OMX_U32 bFrames, OMX_U32 pFrames, std::shared_ptr<SettingsInterface> media)
{
  Gop gop;
  auto ret = media->Get(SETTINGS_INDEX_GROUP_OF_PICTURES, &gop);

  if(ret == SettingsInterface::BAD_INDEX)
    return OMX_ErrorUnsupportedIndex;

  gop.b = ConvertOMXToMediaBFrames(bFrames, pFrames);
  gop.length = ConvertOMXToMediaGopLength(bFrames, pFrames);

  ret = media->Set(SETTINGS_INDEX_GROUP_OF_PICTURES, &gop);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

static OMX_ERRORTYPE SetMediaConstrainedIntraPrediction(OMX_BOOL constrainedIntraPrediction, std::shared_ptr<SettingsInterface> media)
{
  bool b = ConvertOMXToMediaBool(constrainedIntraPrediction);

  auto ret = media->Set(SETTINGS_INDEX_CONSTRAINED_INTRA_PREDICTION, &b);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

static OMX_ERRORTYPE SetMediaLoopFilter(OMX_ALG_VIDEO_HEVCLOOPFILTERTYPE const& loopFilter, std::shared_ptr<SettingsInterface> media)
{
  LoopFilterType l = ConvertOMXToMediaHEVCLoopFilter(loopFilter);

  auto ret = media->Set(SETTINGS_INDEX_LOOP_FILTER, &l);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ExpertiseHEVC::GetProfileLevelSupported(OMX_PTR param, std::shared_ptr<SettingsInterface> media)
{
  vector<ProfileLevel> supported;
  auto ret = media->Get(SETTINGS_INDEX_PROFILES_LEVELS_SUPPORTED, &supported);
  OMX_CHECK_MEDIA_GET(ret);
  auto& pl = *(OMX_VIDEO_PARAM_PROFILELEVELTYPE*)param;

  if(pl.nProfileIndex >= supported.size())
    return OMX_ErrorNoMore;

  pl.eProfile = ConvertMediaToOMXHEVCProfile(supported[pl.nProfileIndex]);
  pl.eLevel = ConvertMediaToOMXHEVCLevel(supported[pl.nProfileIndex]);

  return OMX_ErrorNone;
}

OMX_ERRORTYPE ExpertiseHEVC::GetProfileLevel(OMX_PTR param, Port const& port, std::shared_ptr<SettingsInterface> media)
{
  ProfileLevel profileLevel;
  auto ret = media->Get(SETTINGS_INDEX_PROFILE_LEVEL, &profileLevel);
  OMX_CHECK_MEDIA_GET(ret);
  auto& pl = *(OMX_VIDEO_PARAM_PROFILELEVELTYPE*)param;
  pl.nPortIndex = port.index;
  pl.eProfile = ConvertMediaToOMXHEVCProfile(profileLevel);
  pl.eLevel = ConvertMediaToOMXHEVCLevel(profileLevel);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ExpertiseHEVC::SetProfileLevel(OMX_PTR param, Port const& port, std::shared_ptr<SettingsInterface> media)
{
  OMX_VIDEO_PARAM_PROFILELEVELTYPE rollback;
  GetProfileLevel(&rollback, port, media);
  auto pl = *(OMX_VIDEO_PARAM_PROFILELEVELTYPE*)param;
  auto profile = static_cast<OMX_ALG_VIDEO_HEVCPROFILETYPE>(pl.eProfile);
  auto level = static_cast<OMX_ALG_VIDEO_HEVCLEVELTYPE>(pl.eLevel);

  auto ret = SetMediaProfileLevel(profile, level, media);

  if(ret != OMX_ErrorNone)
  {
    SetProfileLevel(&rollback, port, media);
    throw ret;
  }

  return OMX_ErrorNone;
}

OMX_ERRORTYPE ExpertiseHEVC::GetExpertise(OMX_PTR param, Port const& port, std::shared_ptr<SettingsInterface> media)
{
  ProfileLevel profileLevel;
  Gop gop;
  bool isConstrainedIntraPrediction;
  LoopFilterType loopFilter;
  auto ret = media->Get(SETTINGS_INDEX_PROFILE_LEVEL, &profileLevel);
  OMX_CHECK_MEDIA_GET(ret);
  bool bGop = (media->Get(SETTINGS_INDEX_GROUP_OF_PICTURES, &gop) == SettingsInterface::SUCCESS);
  bool bIntraPred = (media->Get(SETTINGS_INDEX_CONSTRAINED_INTRA_PREDICTION, &isConstrainedIntraPrediction) == SettingsInterface::SUCCESS);
  bool bLoopFilter = (media->Get(SETTINGS_INDEX_LOOP_FILTER, &loopFilter) == SettingsInterface::SUCCESS);
  auto& hevc = *(OMX_ALG_VIDEO_PARAM_HEVCTYPE*)param;
  hevc.nPortIndex = port.index;
  hevc.nBFrames = bGop ? ConvertMediaToOMXBFrames(gop) : 0;
  hevc.nPFrames = bGop ? ConvertMediaToOMXPFrames(gop) : 0;
  hevc.eProfile = ConvertMediaToOMXHEVCProfile(profileLevel);
  hevc.eLevel = ConvertMediaToOMXHEVCLevel(profileLevel);
  hevc.bConstIpred = bIntraPred ? ConvertMediaToOMXBool(isConstrainedIntraPrediction) : OMX_FALSE;
  hevc.eLoopFilterMode = bLoopFilter ? ConvertMediaToOMXHEVCLoopFilter(loopFilter) : OMX_ALG_VIDEO_HEVCLoopFilterMaxEnum;
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ExpertiseHEVC::SetExpertise(OMX_PTR param, Port const& port, std::shared_ptr<SettingsInterface> media)
{
  OMX_ALG_VIDEO_PARAM_HEVCTYPE rollback;
  GetExpertise(&rollback, port, media);
  auto hevc = *(OMX_ALG_VIDEO_PARAM_HEVCTYPE*)param;

  auto ret = SetMediaProfileLevel(hevc.eProfile, hevc.eLevel, media);

  if(ret != OMX_ErrorNone)
  {
    SetExpertise(&rollback, port, media);
    throw ret;
  }

  // only encoder

  ret = SetMediaGop(hevc.nBFrames, hevc.nPFrames, media);

  if(ret == OMX_ErrorBadParameter)
  {
    SetExpertise(&rollback, port, media);
    throw ret;
  }

  ret = SetMediaConstrainedIntraPrediction(hevc.bConstIpred, media);

  if(ret == OMX_ErrorBadParameter)
  {
    SetExpertise(&rollback, port, media);
    throw ret;
  }

  ret = SetMediaLoopFilter(hevc.eLoopFilterMode, media);

  if(ret == OMX_ErrorBadParameter)
  {
    SetExpertise(&rollback, port, media);
    throw ret;
  }

  return OMX_ErrorNone;
}

