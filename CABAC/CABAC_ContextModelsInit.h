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
 
#pragma once
#include "CommonDef.h"
#include "ContextModel.h"
#include "assert.h"
#include "mex.h"
#include "matrix.h"
#if RWTH_TRACE_CABAC_STATES
#include <cstdio>

#endif

// this class containts all our context models for the encoder and decoder
class CABAC_ContextModels {

public:
  CABAC_ContextModels();
  ~CABAC_ContextModels();

  // this method initializes all contexts, given a matlab array in the form of A=[ctxIdx0,mps0,state0,ctxIdx1,mps1,state1,...]
  // 
  void initContextModelsByMpsState(int maxNumContextModels, const mxArray* ptr);
  void initContextModelsByP0Prob(int maxNumContextModels, const mxArray* ptr);

  // TODO: write a context model initialization function which maps a propability estimate to the respective state
  // TODO: error handling
  // TODO: dynamic allocation of context models
  
  // getter function for a context model used by encoder and decoder
  ContextModel* getContextModel(int ctxIdx) { return &m_contextModels[ctxIdx]; };
#if RWTH_TRACE_CABAC_STATES && RWTH_TRACE_CABAC_TO_FILE
  void writeCompleteTraces(FILE* TraceCabacStatFile);
#endif

private:
  // total number of contexts we use
  // TODO: use this for dynamic allocation of m_contextModels
  int m_maxNumContextModels;

  // ContextModel container
  ContextModel m_contextModels[RWTH_MAX_NUM_CONTEXTS];
  void xMapProbabilityToState(double p0, int& mps, int& state);

};