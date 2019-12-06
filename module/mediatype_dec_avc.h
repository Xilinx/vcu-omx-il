#pragma once

#include "mediatype_dec_interface.h"

#include <vector>

struct DecMediatypeAVC final : DecMediatypeInterface
{
  DecMediatypeAVC(BufferContiguities bufferContiguities, BufferBytesAlignments bufferBytesAlignments, StrideAlignments strideAlignments);
  ~DecMediatypeAVC() override;

  ErrorType Get(std::string index, void* settings) const override;
  ErrorType Set(std::string index, void const* settings) override;
  void Reset() override;

  bool Check() override;

private:
  BufferContiguities bufferContiguities;
  BufferBytesAlignments bufferBytesAlignments;
  StrideAlignments strideAlignments;
  BufferHandles bufferHandles;

  std::vector<AVCProfileType> const profiles
  {
    AVCProfileType::AVC_PROFILE_BASELINE,
    AVCProfileType::AVC_PROFILE_MAIN,
    AVCProfileType::AVC_PROFILE_HIGH,
    AVCProfileType::AVC_PROFILE_HIGH_10,
    AVCProfileType::AVC_PROFILE_HIGH_422,
    AVCProfileType::AVC_PROFILE_CONSTRAINED_BASELINE,
    AVCProfileType::AVC_PROFILE_PROGRESSIVE_HIGH,
    AVCProfileType::AVC_PROFILE_CONSTRAINED_HIGH,
    AVCProfileType::AVC_PROFILE_HIGH_10_INTRA,
    AVCProfileType::AVC_PROFILE_HIGH_422_INTRA,
    AVCProfileType::XAVC_PROFILE_HIGH10_INTRA_CBG,
    AVCProfileType::XAVC_PROFILE_HIGH10_INTRA_VBR,
    AVCProfileType::XAVC_PROFILE_HIGH_422_INTRA_CBG,
    AVCProfileType::XAVC_PROFILE_HIGH_422_INTRA_VBR,
    AVCProfileType::XAVC_PROFILE_LONG_GOP_MAIN_MP4,
    AVCProfileType::XAVC_PROFILE_LONG_GOP_HIGH_MP4,
    AVCProfileType::XAVC_PROFILE_LONG_GOP_HIGH_MXF,
    AVCProfileType::XAVC_PROFILE_LONG_GOP_HIGH_422_MXF,
  };

  std::vector<int> const levels
  {
    9,
    10,
    11,
    12,
    13,
    20,
    21,
    22,
    30,
    31,
    32,
    40,
    41,
    42,
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

  std::vector<SequencePictureModeType> const sequenceModes
  {
    SequencePictureModeType::SEQUENCE_PICTURE_MODE_UNKNOWN,
    SequencePictureModeType::SEQUENCE_PICTURE_MODE_FRAME,
  };

  std::map<Format, std::vector<Format>> supportedFormatsMap;
};
