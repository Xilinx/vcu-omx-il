#include "ROIMngr.h"
#include <cassert>

extern "C"
{
#include "lib_rtos/lib_rtos.h"
}

struct AL_TRoiNode
{
  AL_TRoiNode* pPrev;
  AL_TRoiNode* pNext;

  int iPosX;
  int iPosY;
  int iWidth;
  int iHeight;

  int8_t iDeltaQP;
};

/****************************************************************************/
static AL_INLINE int RoundUp(int iVal, int iRnd)
{
  return (iVal + iRnd - 1) & (~(iRnd - 1));
}

/***************************************************************************/
static AL_INLINE int Clip3(int iVal, int iMin, int iMax)
{
  return ((iVal) < (iMin)) ? (iMin) : ((iVal) > (iMax)) ? (iMax) : (iVal);
}

/****************************************************************************/
static int32_t extendSign(uint32_t value, int numBits)
{
  value &= 0xffffffff >> (32 - numBits);

  if(value >= (1u << (numBits - 1)))
    value -= 1 << numBits;
  return value;
}

/****************************************************************************/
static int8_t ToInt(AL_ERoiQuality eQuality)
{
  return extendSign(eQuality, 6);
}

/****************************************************************************/
static void PushBack(AL_TRoiMngrCtx* pCtx, AL_TRoiNode* pNode)
{
  if(pCtx->pFirstNode)
  {
    assert(pCtx->pLastNode);
    pNode->pPrev = pCtx->pLastNode;
    pNode->pNext = nullptr;
    pCtx->pLastNode->pNext = pNode;
    pCtx->pLastNode = pNode;
  }
  else
  {
    assert(!pCtx->pLastNode);
    pNode->pPrev = nullptr;
    pNode->pNext = nullptr;
    pCtx->pFirstNode = pCtx->pLastNode = pNode;
  }
}

/****************************************************************************/
static uint8_t GetNewDeltaQP(AL_ERoiQuality eQuality)
{

  if(eQuality == AL_ROI_QUALITY_INTRA)
    return MASK_FORCE_INTRA;

  return ToInt(eQuality) & MASK_QP;
}

/****************************************************************************/
static int8_t GetDQp(uint8_t iDeltaQP)
{
  return (int8_t)((iDeltaQP & MASK_QP) << 2) >> 2;
}

/****************************************************************************/
static bool ShouldInsertAfter(int8_t iCurrentQP, int8_t iQPToInsert)
{

  if(iQPToInsert & MASK_FORCE_INTRA)
    return true;

  return GetDQp(iCurrentQP) > GetDQp(iQPToInsert);
}

/****************************************************************************/
static void Insert(AL_TRoiMngrCtx* pCtx, AL_TRoiNode* pNode)
{
  AL_TRoiNode* pCur = pCtx->pFirstNode;

  while(pCur && ShouldInsertAfter(pCur->iDeltaQP, pNode->iDeltaQP))
    pCur = pCur->pNext;

  if(pCur)
  {
    pNode->pPrev = pCur->pPrev;
    pNode->pNext = pCur;
    pCur->pPrev = pNode;

    if(pCur == pCtx->pFirstNode)
      pCtx->pFirstNode = pNode;
  }
  else
    PushBack(pCtx, pNode);
}

/****************************************************************************/
static void MeanQuality(AL_TRoiMngrCtx* pCtx, uint8_t* pTargetQP, uint8_t iDQp1, uint8_t iDQp2, int iNumQPPerLCU)
{
  auto eMask = (*pTargetQP & MASK_FORCE);

  int8_t iQP = Clip3((GetDQp(iDQp1) + GetDQp(iDQp2)) / 2, pCtx->iMinQP, pCtx->iMaxQP) & MASK_QP;
  pTargetQP[0] = iQP | eMask;

  for(int i = 1; i < iNumQPPerLCU; ++i)
    pTargetQP[i] = pTargetQP[0];
}

/****************************************************************************/
static void UpdateTransitionHorz(AL_TRoiMngrCtx* pCtx, uint8_t* pLcu1, uint8_t* pLcu2, int iNumQPPerLCU, int iNumBytesPerLCU, int iLcuWidth, int iPosX, int iWidth, int8_t iQP)
{
  // left corner
  if(iPosX > 1)
    MeanQuality(pCtx, &pLcu1[-iNumBytesPerLCU], pLcu2[-2 * iNumBytesPerLCU], iQP, iNumQPPerLCU);
  else if(iPosX > 0)
    MeanQuality(pCtx, &pLcu1[-iNumBytesPerLCU], pLcu2[-iNumBytesPerLCU], iQP, iNumQPPerLCU);

  // width
  for(int w = 0; w < iWidth; ++w)
    MeanQuality(pCtx, &pLcu1[w * iNumBytesPerLCU], pLcu2[w * iNumBytesPerLCU], iQP, iNumQPPerLCU);

  // right corner
  if(iPosX + iWidth + 2 < iLcuWidth)
    MeanQuality(pCtx, &pLcu1[iWidth * iNumBytesPerLCU], pLcu2[(iWidth + 1) * iNumBytesPerLCU], iQP, iNumQPPerLCU);
  else if(iPosX + iWidth + 1 < iLcuWidth)
    MeanQuality(pCtx, &pLcu1[iWidth * iNumBytesPerLCU], pLcu2[iWidth * iNumBytesPerLCU], iQP, iNumQPPerLCU);
}

/****************************************************************************/
static void UpdateTransitionVert(AL_TRoiMngrCtx* pCtx, uint8_t* pLcu1, uint8_t* pLcu2, int iNumQPPerLCU, int iNumBytesPerLCU, int iLcuWidth, int iHeight, int8_t iQP)
{
  for(int h = 0; h < iHeight; ++h)
  {
    MeanQuality(pCtx, pLcu1, *pLcu2, iQP, iNumQPPerLCU);
    pLcu1 += (iLcuWidth * iNumBytesPerLCU);
    pLcu2 += (iLcuWidth * iNumBytesPerLCU);
  }
}

/****************************************************************************/
static uint32_t GetNodePosInBuf(AL_TRoiMngrCtx* pCtx, uint32_t uLcuX, uint32_t uLcuY, int iNumBytesPerLCU)
{
  uint32_t uLcuNum = uLcuY * pCtx->iLcuWidth + uLcuX;
  return uLcuNum * iNumBytesPerLCU;
}

/****************************************************************************/
static inline void SetLCUQuality(uint8_t* pLCUQP, uint8_t uROIQP)
{

  if(uROIQP & MASK_FORCE_INTRA)
    *pLCUQP = (*pLCUQP & MASK_QP) | MASK_FORCE_INTRA;
  else if((*pLCUQP & MASK_FORCE_INTRA) && !(uROIQP & MASK_FORCE_MV0))
  {
    *pLCUQP = uROIQP | MASK_FORCE_INTRA;
  }
  else
  {
    *pLCUQP = uROIQP;
  }
}

/****************************************************************************/
static void ComputeROI(AL_TRoiMngrCtx* pCtx, int iNumQPPerLCU, int iNumBytesPerLCU, uint8_t* pBuf, AL_TRoiNode* pNode)
{
  auto* pLCU = pBuf + GetNodePosInBuf(pCtx, pNode->iPosX, pNode->iPosY, iNumBytesPerLCU);

  // Fill Roi
  for(int h = 0; h < pNode->iHeight; ++h)
  {
    for(int w = 0; w < pNode->iWidth; ++w)
    {
      for(int i = 0; i < iNumQPPerLCU; ++i)
        SetLCUQuality(&pLCU[w * iNumBytesPerLCU + i], pNode->iDeltaQP);
    }

    pLCU += iNumBytesPerLCU * pCtx->iLcuWidth;
  }

  if(!(pNode->iDeltaQP & MASK_FORCE))
  {
    // Update above transition
    if(pNode->iPosY)
    {
      uint8_t* pLcuTop1 = pBuf + GetNodePosInBuf(pCtx, pNode->iPosX, pNode->iPosY - 1, iNumBytesPerLCU);
      uint8_t* pLcuTop2 = pLcuTop1;

      if(pNode->iPosY > 1)
        pLcuTop2 = pBuf + GetNodePosInBuf(pCtx, pNode->iPosX, pNode->iPosY - 2, iNumBytesPerLCU);

      UpdateTransitionHorz(pCtx, pLcuTop1, pLcuTop2, iNumQPPerLCU, iNumBytesPerLCU, pCtx->iLcuWidth, pNode->iPosX, pNode->iWidth, pNode->iDeltaQP);
    }

    // update below transition
    if(pNode->iPosY + pNode->iHeight + 1 < pCtx->iLcuHeight)
    {
      uint8_t* pLcuBot1 = pBuf + GetNodePosInBuf(pCtx, pNode->iPosX, pNode->iPosY + pNode->iHeight, iNumBytesPerLCU);
      uint8_t* pLcuBot2 = pLcuBot1;

      if(pNode->iPosY + pNode->iHeight + 2 < pCtx->iLcuHeight)
        pLcuBot2 = pBuf + GetNodePosInBuf(pCtx, pNode->iPosX, pNode->iPosY + pNode->iHeight + 1, iNumBytesPerLCU);

      UpdateTransitionHorz(pCtx, pLcuBot1, pLcuBot2, iNumQPPerLCU, iNumBytesPerLCU, pCtx->iLcuWidth, pNode->iPosX, pNode->iWidth, pNode->iDeltaQP);
    }

    // update left transition
    if(pNode->iPosX)
    {
      uint8_t* pLcuLeft1 = pBuf + GetNodePosInBuf(pCtx, pNode->iPosX - 1, pNode->iPosY, iNumBytesPerLCU);
      uint8_t* pLcuLeft2 = pLcuLeft1;

      if(pNode->iPosX > 1)
        pLcuLeft2 = pBuf + GetNodePosInBuf(pCtx, pNode->iPosX - 2, pNode->iPosY, iNumBytesPerLCU);

      UpdateTransitionVert(pCtx, pLcuLeft1, pLcuLeft2, iNumQPPerLCU, iNumBytesPerLCU, pCtx->iLcuWidth, pNode->iHeight, pNode->iDeltaQP);
    }

    // update right transition
    if(pNode->iPosX + pNode->iWidth + 1 < pCtx->iLcuWidth)
    {
      uint8_t* pLcuRight1 = pBuf + GetNodePosInBuf(pCtx, pNode->iPosX + pNode->iWidth, pNode->iPosY, iNumBytesPerLCU);
      uint8_t* pLcuRight2 = pLcuRight1;

      if(pNode->iPosX + pNode->iWidth + 2 < pCtx->iLcuWidth)
        pLcuRight2 = pBuf + GetNodePosInBuf(pCtx, pNode->iPosX + pNode->iWidth + 1, pNode->iPosY, iNumBytesPerLCU);

      UpdateTransitionVert(pCtx, pLcuRight1, pLcuRight2, iNumQPPerLCU, iNumBytesPerLCU, pCtx->iLcuWidth, pNode->iHeight, pNode->iDeltaQP);
    }
  }
}

/****************************************************************************/
AL_TRoiMngrCtx* AL_RoiMngr_Create(int iPicWidth, int iPicHeight, AL_EProfile eProf, AL_ERoiQuality eBkgQuality, AL_ERoiOrder eOrder)
{
  AL_TRoiMngrCtx* pCtx = (AL_TRoiMngrCtx*)Rtos_Malloc(sizeof(AL_TRoiMngrCtx));

  if(!pCtx)
    return nullptr;
  pCtx->iMinQP = AL_IS_AVC(eProf) || AL_IS_HEVC(eProf) ? -32 : -128;
  pCtx->iMaxQP = AL_IS_AVC(eProf) || AL_IS_HEVC(eProf) ? 31 : 127;
  pCtx->iPicWidth = iPicWidth; // TODO convert Pixel to LCU
  pCtx->iPicHeight = iPicHeight; // TODO convert Pixel to LCU
  pCtx->uLcuSize = AL_IS_AVC(eProf) ? 4 : AL_IS_HEVC(eProf) ? 5 : 6;

  pCtx->eBkgQuality = eBkgQuality;
  pCtx->eOrder = eOrder;
  pCtx->pFirstNode = nullptr;
  pCtx->pLastNode = nullptr;

  pCtx->iLcuWidth = RoundUp(pCtx->iPicWidth, 1 << pCtx->uLcuSize) >> pCtx->uLcuSize;
  pCtx->iLcuHeight = RoundUp(pCtx->iPicHeight, 1 << pCtx->uLcuSize) >> pCtx->uLcuSize;
  pCtx->iNumLCUs = pCtx->iLcuWidth * pCtx->iLcuHeight;

  return pCtx;
}

/****************************************************************************/
void AL_RoiMngr_Destroy(AL_TRoiMngrCtx* pCtx)
{
  AL_RoiMngr_Clear(pCtx);
  Rtos_Free(pCtx);
}

/****************************************************************************/
void AL_RoiMngr_Clear(AL_TRoiMngrCtx* pCtx)
{
  AL_TRoiNode* pCur = pCtx->pFirstNode;

  while(pCur)
  {
    AL_TRoiNode* pNext = pCur->pNext;
    Rtos_Free(pCur);
    pCur = pNext;
  }

  pCtx->pFirstNode = nullptr;
  pCtx->pLastNode = nullptr;
}

/****************************************************************************/
bool AL_RoiMngr_AddROI(AL_TRoiMngrCtx* pCtx, int iPosX, int iPosY, int iWidth, int iHeight, AL_ERoiQuality eQuality)
{
  if(iPosX >= pCtx->iPicWidth || iPosY >= pCtx->iPicHeight)
    return false;

  iPosX = iPosX >> pCtx->uLcuSize;
  iPosY = iPosY >> pCtx->uLcuSize;
  iWidth = RoundUp(iWidth, 1 << pCtx->uLcuSize) >> pCtx->uLcuSize;
  iHeight = RoundUp(iHeight, 1 << pCtx->uLcuSize) >> pCtx->uLcuSize;

  AL_TRoiNode* pNode = (AL_TRoiNode*)Rtos_Malloc(sizeof(AL_TRoiNode));

  if(!pNode)
    return false;

  pNode->iPosX = iPosX;
  pNode->iPosY = iPosY;
  pNode->iWidth = ((iPosX + iWidth) > pCtx->iLcuWidth) ? (pCtx->iLcuWidth - iPosX) : iWidth;
  pNode->iHeight = ((iPosY + iHeight) > pCtx->iLcuHeight) ? (pCtx->iLcuHeight - iPosY) : iHeight;

  pNode->iDeltaQP = GetNewDeltaQP(eQuality);
  pNode->pNext = nullptr;
  pNode->pPrev = nullptr;

  if(pCtx->eOrder == AL_ROI_QUALITY_ORDER)
    Insert(pCtx, pNode);
  else
    PushBack(pCtx, pNode);

  return true;
}

/****************************************************************************/
void AL_RoiMngr_FillBuff(AL_TRoiMngrCtx* pCtx, int iNumQPPerLCU, int iNumBytesPerLCU, uint8_t* pBuf)
{
  assert(pBuf);

  // Fill background
  for(int iLCU = 0; iLCU < pCtx->iNumLCUs; iLCU++)
  {
    int iFirst = iLCU * iNumBytesPerLCU;

    for(int iQP = 0; iQP < iNumQPPerLCU; ++iQP)
      pBuf[iFirst + iQP] = GetNewDeltaQP(pCtx->eBkgQuality);
  }

  // Fill ROIs
  AL_TRoiNode* pCur = pCtx->pFirstNode;

  while(pCur)
  {
    ComputeROI(pCtx, iNumQPPerLCU, iNumBytesPerLCU, pBuf, pCur);
    pCur = pCur->pNext;
  }
}
