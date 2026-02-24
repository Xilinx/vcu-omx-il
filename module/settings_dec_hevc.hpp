// SPDX-FileCopyrightText: © 2026 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "module/module_enums.hpp"
#include "settings_dec_interface.hpp"

#include <vector>

struct DecSettingsHEVC final : DecSettingsInterface
{
  DecSettingsHEVC(BufferContiguities bufferContiguities, BufferBytesAlignments bufferBytesAlignments, StrideAlignments strideAlignments);
  ~DecSettingsHEVC() override;

  ErrorType Get(std::string index, void* settings) const override;
  ErrorType Set(std::string index, void const* settings) override;
  void Reset() override;

  bool Check() override;

private:
  BufferContiguities bufferContiguities;
  BufferBytesAlignments bufferBytesAlignments;
  StrideAlignments strideAlignments;
  BufferHandles bufferHandles;
  int tier;

  std::vector<HEVCProfileType> const profiles
  {
    HEVCProfileType::HEVC_PROFILE_MAIN,
    HEVCProfileType::HEVC_PROFILE_MAIN_10,
    HEVCProfileType::HEVC_PROFILE_MAIN_12,
    HEVCProfileType::HEVC_PROFILE_MAIN_STILL,
    HEVCProfileType::HEVC_PROFILE_MONOCHROME,
    HEVCProfileType::HEVC_PROFILE_MONOCHROME_10,
    HEVCProfileType::HEVC_PROFILE_MONOCHROME_12,
    HEVCProfileType::HEVC_PROFILE_MAIN_422,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_10,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_12,
    HEVCProfileType::HEVC_PROFILE_MAIN_444,
    HEVCProfileType::HEVC_PROFILE_MAIN_444_10,
    HEVCProfileType::HEVC_PROFILE_MAIN_444_12,
    HEVCProfileType::HEVC_PROFILE_MAIN_INTRA,
    HEVCProfileType::HEVC_PROFILE_MAIN_10_INTRA,
    HEVCProfileType::HEVC_PROFILE_MAIN_12_INTRA,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_INTRA,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_10_INTRA,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_12_INTRA,
    HEVCProfileType::HEVC_PROFILE_MAIN_444_INTRA,
    HEVCProfileType::HEVC_PROFILE_MAIN_444_10_INTRA,
    HEVCProfileType::HEVC_PROFILE_MAIN_444_12_INTRA,
    HEVCProfileType::HEVC_PROFILE_MAIN_444_STILL,
    HEVCProfileType::HEVC_PROFILE_MAIN_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_10_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_12_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_STILL_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MONOCHROME_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MONOCHROME_10_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MONOCHROME_12_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_10_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_12_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_444_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_444_10_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_444_12_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_INTRA_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_10_INTRA_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_12_INTRA_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_INTRA_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_10_INTRA_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_12_INTRA_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_444_INTRA_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_444_10_INTRA_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_444_12_INTRA_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_444_STILL_HIGH_TIER,
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
    63,
    70,
    71,
    72,
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

  std::vector<StorageType> const storages
  {
    StorageType::STORAGE_RASTER,
  };

  std::vector<SequencePictureModeType> const sequenceModes
  {
    SequencePictureModeType::SEQUENCE_PICTURE_MODE_UNKNOWN,
    SequencePictureModeType::SEQUENCE_PICTURE_MODE_FRAME,
    SequencePictureModeType::SEQUENCE_PICTURE_MODE_FIELD,
  };
};
