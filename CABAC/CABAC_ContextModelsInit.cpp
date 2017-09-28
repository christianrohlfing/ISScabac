/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.  
 *
 * Copyright (c) 2016-2017, Institut für Nachrichtentechnik, RWTH Aachen University
 *
 * Christian Feldmann, Christian Rohlfing, Yingbo Gao, Max Bläser
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
  
#include "CABAC_ContextModelsInit.h"
#include <cmath>
#include <algorithm>

template <typename T> T clip(const T& n, const T& lower, const T& upper) 
{
  return std::max(lower, std::min(n, upper));
}

CABAC_ContextModels::CABAC_ContextModels()
{
}

CABAC_ContextModels::~CABAC_ContextModels()
{
}

void CABAC_ContextModels::initContextModelsByMpsState(int maxNumContextModels, const mxArray * ptr)
{
  assert(maxNumContextModels < RWTH_MAX_NUM_CONTEXTS);
  m_maxNumContextModels = maxNumContextModels;

  if (!mxIsDouble(ptr))
    assert(mxIsDouble(ptr));

  int numrows = mxGetM(ptr);
  int numcols = mxGetN(ptr);
  int numelements = mxGetNumberOfElements(ptr);
  int numbytesperelement = mxGetElementSize(ptr);

#if RWTH_CABAC_DEBUG_OUTPUT
  mexPrintf("Status: context initializing, size: %i x %i, total: %i, %i bytes per element\n", numrows, numcols, numelements, numbytesperelement);
#endif

  double * data = mxGetPr(ptr);

  for (int ctxIdx = 0; ctxIdx < m_maxNumContextModels; ctxIdx++)
  {
    double * mpsdata = data + (ctxIdx * 3 + 1);
    double * statedata = data + (ctxIdx * 3 + 2);
    m_contextModels[ctxIdx].init(static_cast<unsigned int>(*(mpsdata)), static_cast<unsigned int>(*(statedata)));
#if RWTH_CABAC_DEBUG_OUTPUT
    mexPrintf("Status: context: %i, mps: %i, state: %i\n", ctxIdx, static_cast<unsigned int>(*(mpsdata)), static_cast<unsigned int>(*(statedata)));
#endif
  }

}

void CABAC_ContextModels::initContextModelsByP0Prob(int maxNumContextModels, const mxArray * ptr)
{
  assert(maxNumContextModels < RWTH_MAX_NUM_CONTEXTS);
  m_maxNumContextModels = maxNumContextModels;

  if (!mxIsDouble(ptr))
    assert(mxIsDouble(ptr));

  int numrows = mxGetM(ptr);
  int numcols = mxGetN(ptr);
  int numelements = mxGetNumberOfElements(ptr);
  int numbytesperelement = mxGetElementSize(ptr);

#if RWTH_CABAC_DEBUG_OUTPUT
  mexPrintf("Status: context initialized, size: %i x %i, total: %i, %i bytes per element\n", numrows, numcols, numelements, numbytesperelement);
#endif

  double * data = mxGetPr(ptr);
  int mps = -1;
  int state = -1;

  for (int ctxIdx = 0; ctxIdx < m_maxNumContextModels; ctxIdx++)
  {
    double * p0probs = data + ctxIdx;
    xMapProbabilityToState(*(p0probs), mps, state);
    m_contextModels[ctxIdx].init(mps, state);
#if RWTH_CABAC_DEBUG_OUTPUT
    mexPrintf("Status: context: %i, p0: %.2f, mps: %i, state: %i\n", ctxIdx, *(p0probs), mps, state);
#endif
  }
}

#if RWTH_TRACE_CABAC_STATES && RWTH_TRACE_CABAC_TO_FILE
void CABAC_ContextModels::writeCompleteTraces(FILE * TraceCabacStatFile)
{
  for (int k = 0; k < m_maxNumContextModels; k++)
  {
    m_contextModels[k].traceStatesToFile(TraceCabacStatFile);
  }
}
#endif

void CABAC_ContextModels::xMapProbabilityToState(double p0, int& mps, int& state)
{
  if (p0 > 1.0 || p0 < 0.0)
    assert(p0 <= 1.0 && p0 >= 0.0);

  double pLPS = 0.0;
  if (p0 >= 0.5) // 0 is MPS
  {
    pLPS = 1.0 - p0;
    mps = 0;
  }
  else // 1 is MPS
  {
    pLPS = p0;
    mps = 1;
  }
  
  if (pLPS < 0.01875)
  {
    pLPS = 0.01875;
  }

  // turn the LPS probability into the closest matching state
  state = clip<int>((int)std::round(62 * std::log10(2.0 * pLPS) / std::log10(2.0 * 0.01875)), 0, 62);
}
