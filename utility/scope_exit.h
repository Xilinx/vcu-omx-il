// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

template<typename Lambda>
struct ScopeExitClass
{
  ScopeExitClass(Lambda fn) :
    m_fn{fn}
  {
  }

  ~ScopeExitClass()
  {
    m_fn();
  }

private:
  Lambda m_fn;
};

template<typename Lambda>
ScopeExitClass<Lambda> scopeExit(Lambda fn)
{
  return ScopeExitClass<Lambda> {
           fn
  };
}

