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

#include "omx_enc_param.h"

int EncodingParameters::getCPBSize()
{
  return cpbSize;
}

void EncodingParameters::setCPBSize(int size)
{
  this->cpbSize = size;
}

int EncodingParameters::getInitialRemovalDelay()
{
  return initialRemovalDelay;
}

void EncodingParameters::setInitialRemovalDelay(int delay)
{
  this->initialRemovalDelay = delay;
}

int EncodingParameters::getL2CacheSize()
{
  return l2CacheSize;
}

void EncodingParameters::setL2CacheSize(int size)
{
  this->l2CacheSize = size;
}

int EncodingParameters::getBitrate()
{
  return bitrate;
}

void EncodingParameters::setBitrate(int bitrate)
{
  this->bitrate = bitrate;
}

int EncodingParameters::getMaxBitrate()
{
  return maxBitrate;
}

void EncodingParameters::setMaxBitrate(int maxBitrate)
{
  this->maxBitrate = maxBitrate;
}

int EncodingParameters::getNumSlices()
{
  return numSlices;
}

void EncodingParameters::setLevel(int level)
{
  this->level = level;
}

int EncodingParameters::getLevel()
{
  return level;
}

void EncodingParameters::setNumSlices(int number)
{
  numSlices = number;
}

void EncodingParameters::setProfile(AL_EProfile profile)
{
  this->profile = profile;
}

AL_EProfile EncodingParameters::getProfile()
{
  return profile;
}

void EncodingParameters::setRCMode(AL_ERateCtrlMode mode)
{
  rc = mode;
}

AL_ERateCtrlMode EncodingParameters::getRCMode()
{
  return rc;
}

void EncodingParameters::setRCOptions(AL_ERateCtrlOption opt)
{
  rcOptions = opt;
}

AL_ERateCtrlOption EncodingParameters::getRCOptions()
{
  return rcOptions;
}

Quantization EncodingParameters::getQuantization()
{
  return quant;
}

void EncodingParameters::setQuantization(Quantization const quant)
{
  this->quant = quant;
}

QuantizationExt EncodingParameters::getQuantizationExt()
{
  return quantExt;
}

void EncodingParameters::setQuantizationExtension(QuantizationExt const quantExt)
{
  this->quantExt = quantExt;
}

void EncodingParameters::setGopMode(OMX_ALG_EGopCtrlMode const mode)
{
  this->gop = mode;
}

OMX_ALG_EGopCtrlMode EncodingParameters::getGopMode()
{
  return gop;
}

void EncodingParameters::setQpMode(OMX_ALG_EQpCtrlMode const mode)
{
  this->qpMode = mode;
}

OMX_ALG_EQpCtrlMode EncodingParameters::getQpMode()
{
  return qpMode;
}

void EncodingParameters::setGdrMode(OMX_ALG_EGdrMode const mode)
{
  this->gdr = mode;
}

OMX_ALG_EGdrMode EncodingParameters::getGdrMode()
{
  return gdr;
}

void EncodingParameters::setScalingListMode(OMX_ALG_EScalingList const mode)
{
  this->sclist = mode;
}

OMX_ALG_EScalingList EncodingParameters::getScalingListMode()
{
  return sclist;
}

void EncodingParameters::setDependentSlices(bool const isDependent)
{
  this->dependentSlices = isDependent;
}

bool EncodingParameters::getDependentSlices()
{
  return dependentSlices;
}

void EncodingParameters::setSlicesSize(int const size)
{
  this->slicesSize = size;
}

int EncodingParameters::getSlicesSize()
{
  return this->slicesSize;
}

void EncodingParameters::setAspectRatio(OMX_ALG_EAspectRatio const aspect)
{
  this->aspectRatio = aspect;
}

OMX_ALG_EAspectRatio EncodingParameters::getAspectRatio()
{
  return aspectRatio;
}

void EncodingParameters::setIDRFreq(uint32_t const freq)
{
  this->idrFreq = freq;
}

uint32_t EncodingParameters::getIDRFreq()
{
  return idrFreq;
}

