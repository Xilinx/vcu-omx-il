// SPDX-FileCopyrightText: © 2025 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "module/module_enums.hpp"
#include "settings_enc_interface.hpp"

#include <vector>
#include <memory>

extern "C"
{
#include <lib_common/Allocator.h>
}

struct EncSettingsMJPEG final : EncSettingsInterface
{
  EncSettingsMJPEG(BufferContiguities bufferContiguities, BufferBytesAlignments bufferBytesAlignments, StrideAlignments strideAlignments, bool isSeparateConfigurationFromDataEnabled, std::shared_ptr<AL_TAllocator> const& allocator);
  ~EncSettingsMJPEG() override;

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
  std::shared_ptr<AL_TAllocator> allocator;

  std::vector<JPEGProfileType> const profiles =
  {
    JPEGProfileType::JPEG_PROFILE,
    JPEGProfileType::JPEG_PROFILE_EXT_HUFF,
    JPEGProfileType::JPEG_PROFILE_LOSSLESS
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
  };

  std::vector<StorageType> const storages
  {
    StorageType::STORAGE_RASTER,
  };

  std::vector<VideoModeType> const videoModes
  {
    VideoModeType::VIDEO_MODE_PROGRESSIVE,
  };
};
