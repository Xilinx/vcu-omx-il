// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <omx_header/OMX_Types.h>
#include <omx_header/OMX_CoreAlg.h>
OMX_PTR CreateDecComponentPrivate(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_STRING cComponentName, OMX_IN OMX_STRING cRole, OMX_IN OMX_ALG_COREINDEXTYPE nCoreParamIndex, OMX_IN OMX_PTR pSettings);
void DestroyDecComponentPrivate(OMX_IN OMX_PTR pComponentPrivate);

