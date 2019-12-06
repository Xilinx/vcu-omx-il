#pragma once

#include "mediatype_enc_interface.h"

#include <vector>

struct EncMediatypeHEVC final : EncMediatypeInterface
{
  EncMediatypeHEVC(BufferContiguities bufferContiguities, BufferBytesAlignments bufferBytesAlignments, StrideAlignments strideAlignments, bool isSeparateConfigurationFromDataEnabled);
  ~EncMediatypeHEVC() override;

  ErrorType Get(std::string index, void* settings) const override;
  ErrorType Set(std::string index, void const* settings) override;
  void Reset() override;

  bool Check() override;

private:
  BufferContiguities bufferContiguities;
  BufferBytesAlignments bufferBytesAlignments;
  StrideAlignments strideAlignments;
  bool isSeparateConfigurationFromDataEnabled;
  BufferHandles bufferHandles;
  std::string sTwoPassLogFile;

  std::vector<HEVCProfileType> const profiles
  {
    HEVCProfileType::HEVC_PROFILE_MAIN,
    HEVCProfileType::HEVC_PROFILE_MAIN_10,
    HEVCProfileType::HEVC_PROFILE_MAIN_STILL,
    HEVCProfileType::HEVC_PROFILE_MONOCHROME,
    HEVCProfileType::HEVC_PROFILE_MONOCHROME_10,
    HEVCProfileType::HEVC_PROFILE_MAIN_422,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_10,
    HEVCProfileType::HEVC_PROFILE_MAIN_INTRA,
    HEVCProfileType::HEVC_PROFILE_MAIN_10_INTRA,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_INTRA,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_10_INTRA,
    HEVCProfileType::HEVC_PROFILE_MAIN_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_10_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_STILL_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MONOCHROME_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MONOCHROME_10_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_10_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_INTRA_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_10_INTRA_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_INTRA_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_10_INTRA_HIGH_TIER,
  };

  std::vector<int> const levels
  {
    10,
    20,
    21,
    30,
    31,
    40,
    41,
    50,
    51,
    52,
    60,
    61,
    62,
  };

  std::vector<ColorType> const colors
  {
    ColorType::COLOR_400,
    ColorType::COLOR_420,
    ColorType::COLOR_422,
  };

  std::vector<int> const bitdepths
  {
    8,
    10,
  };

  std::vector<VideoModeType> const videoModes
  {
    VideoModeType::VIDEO_MODE_PROGRESSIVE,
    VideoModeType::VIDEO_MODE_ALTERNATE_TOP_BOTTOM_FIELD,
    VideoModeType::VIDEO_MODE_ALTERNATE_BOTTOM_TOP_FIELD,
  };

  std::map<Format, std::vector<Format>> supportedFormatsMap;
};
