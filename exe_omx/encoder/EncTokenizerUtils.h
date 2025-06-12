// SPDX-FileCopyrightText: © 2025 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <list>
#include <iostream>
#include <string>

struct TBounds
{
  int32_t min;
  int32_t max;
};

/****************************************************************************/
struct CCmdTokenizer
{
  explicit CCmdTokenizer(std::string& sLine)
    : m_sLine(sLine),
    m_zBeg(0),
    m_zEnd(0),
    m_zNext(0),
    m_sVal("")
  {}

  bool GetNext(void)
  {
    m_sVal = "";

    if(m_zNext == std::string::npos)
      return false;

    m_zBeg = m_sLine.find_first_not_of(m_Separators, m_zNext);

    if(m_zBeg == std::string::npos)
      return false;

    m_zEnd = m_sLine.find_first_of(m_Separators, m_zBeg);

    if(m_zEnd != std::string::npos && m_sLine[m_zEnd] == '=')
    {
      size_t zPos1 = m_sLine.find_first_not_of(m_Separators, m_zEnd + 1);

      if(zPos1 == std::string::npos)
        return false;

      size_t zPos2 = m_sLine.find_first_of(m_Separators, zPos1 + 1);

      m_sVal = m_sLine.substr(zPos1, zPos2 - zPos1);

      m_zNext = zPos2;
    }
    else
      m_zNext = m_zEnd;

    return true;
  }

  double GetValue() { return atof(m_sVal.c_str()); }

  TBounds GetValueBounds(void)
  {
    TBounds tBounds = { 0, 0 };

    size_t sLowerPos = m_sVal.find('[');
    size_t sSplitPos = m_sVal.find(';');
    size_t sUpperPos = m_sVal.find(']');

    if(sLowerPos != 0 || sSplitPos == std::string::npos || sSplitPos <= sLowerPos || sUpperPos == std::string::npos || sUpperPos <= sSplitPos)
      throw std::runtime_error("bad range syntax for line " + m_sLine);

    tBounds.min = atof(m_sVal.substr(1, sSplitPos - 1).c_str());
    tBounds.max = atof(m_sVal.substr(sSplitPos + 1, sUpperPos - (sSplitPos + 1)).c_str());

    return tBounds;
  }

  std::list<std::string> GetValueList(void)
  {
    size_t sLowerPos = m_sVal.find('(');
    size_t sUpperPos = m_sVal.find(')');
    size_t last_element_detection = m_sVal.find_last_of(';');

    std::list<std::string> listElement;

    size_t lower = sLowerPos + 1;

    if((sLowerPos == std::string::npos) || (sUpperPos == std::string::npos))
      throw std::runtime_error("missing parentheses in dynamic commands");

    if(last_element_detection != std::string::npos)
    {
      for(size_t i = sLowerPos; i < sUpperPos; i++)
      {
        if(m_sVal.substr(i).find(';') == 0)
        {
          if(i != last_element_detection)
          {
            // retrieve element from beginning to n-2
            listElement.push_back(m_sVal.substr(lower, i - lower));
            lower = i + 1;
          }
          else
          {
            // retrieve before last element
            listElement.push_back(m_sVal.substr(lower, last_element_detection - lower));
            // retrieve last element
            listElement.push_back(m_sVal.substr(last_element_detection + 1, sUpperPos - last_element_detection - 1));
          }
        }
      }
    }
    else
    {
      // singleton
      listElement.push_back(m_sVal.substr(1, sUpperPos - 1));
    }

    return listElement;
  }

  bool operator == (char const* const S) const { return m_sLine.substr(m_zBeg, m_zEnd - m_zBeg) == S; }
  int32_t atoi(void)
  {
    return ::atoi(m_sLine.substr(m_zBeg, m_zEnd - m_zBeg).c_str());
  }

private:
  char const* const m_Separators = ":,= \t\r";

  std::string const& m_sLine;
  size_t m_zBeg;
  size_t m_zEnd;
  size_t m_zNext;
  std::string m_sVal;
};
