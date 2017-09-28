/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.  
 *
 * Copyright (c) 2010-2017, ITU/ISO/IEC
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
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     ContextModel.h
    \brief    context model class (header)
*/

#ifndef __CONTEXT_MODEL__
#define __CONTEXT_MODEL__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CommonDef.h"

#if RWTH_TRACE_CABAC_STATES
#include <cstdio>
#include <vector>
#include <stdint.h>

using namespace std;
struct _CABACStep
{
  uint8_t codedBin;
  uint8_t state_p;
  uint8_t mps_p;
  uint8_t state_a;
  uint8_t mps_a;
  _CABACStep(uint8_t c, uint8_t sp, uint8_t mp, uint8_t sa, uint8_t ma) :
    codedBin(c), state_p(sp), mps_p(mp), state_a(sa), mps_a(ma)
  { }
};
typedef struct _CABACStep CABACStep;
#endif

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// context model class
class ContextModel
{
public:
  ContextModel  ();
  ~ContextModel ();
  
  unsigned char getState  ()                { return ( m_ucState >> 1 ); }                    ///< get current state
  unsigned char getMps    ()                { return ( m_ucState  & 1 ); }                    ///< get curret MPS
  void  setStateAndMps( unsigned char ucState, unsigned char ucMPS) { m_ucState = (ucState << 1) + ucMPS; } ///< set state and MPS
  
  void init ( unsigned int uiMps, unsigned int uiState );   ///< initialize state with initial probability
  unsigned char getInitState() { return ( m_ucInitState >> 1); }
  unsigned char getInitMps  () { return ( m_ucInitState  & 1); }
  int  m_ucInitState;
  
  void updateLPS ();
  void updateMPS ();
  
  int getEntropyBits(short val) { return m_entropyBits[m_ucState ^ val]; }
  
  unsigned int getBinsCoded()           { return m_binsCoded;   }

#if RWTH_TRACE_CABAC_STATES
  void addCabacStep(uint8_t cbin, uint8_t state_p, uint8_t mps_p, uint8_t state_a, uint8_t mps_a);
  std::vector<CABACStep>* getCabacSteps() { return &m_CABACSteps; };
  vector<vector<unsigned int>>* getCabactTransitions() { return &m_uiTraceCabacTransitions; };
#endif
  
private:
  unsigned char m_ucState; ///< internal state variable
  static const unsigned char m_aucNextStateMPS[ 128 ];
  static const unsigned char m_aucNextStateLPS[ 128 ];
  static const int m_entropyBits[ 128 ];
  unsigned int m_binsCoded;

  static unsigned int m_uiInstanceCounter;
  unsigned int m_uiCtxIdx;

#if RWTH_TRACE_CABAC_STATES
  // Save which how many bins were coded in each state
  uint64_t m_uiTraceCabacState[RWTH_TRACE_CABAC_STATES_NUM_STATES];
  void updateTraceState();
  vector<CABACStep> m_CABACSteps;
  vector<vector<unsigned int>> m_uiTraceCabacTransitions;
public:
#if RWTH_TRACE_CABAC_TO_FILE
  void traceStatesToFile(FILE *pOutFile);
#endif
#endif
};

//! \}

#endif

