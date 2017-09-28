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
 
// Common definitions for the simplified CABAC
#ifndef __COMMONDEF__
#define __COMMONDEF__

// Trace the cabac states (how often is which context coded?)
#ifdef _WIN32
#define RWTH_TRACE_CABAC_STATES 1
#else
#define RWTH_TRACE_CABAC_STATES 0
#endif
#define RWTH_TRACE_CABAC 0
#define RWTH_TRACE_CABAC_TO_FILE 0
#define RWTH_TRACE_CABAC_STATES_NUM_STATES 128 /// The number of states


// Enables coding of bins with a fixed probability
#define RWTH_CABAC_FIXED_PROBABILITY 0

// Enable Debug Output for MEX
#define RWTH_CABAC_DEBUG_OUTPUT 0

// Maximum Number of Contexts
#define RWTH_MAX_NUM_CONTEXTS 1000

/** clip a, such that minVal <= a <= maxVal */
template <typename T> inline T Clip3( T minVal, T maxVal, T a) { return std::min<T> (std::max<T> (minVal, a) , maxVal); }  ///< general min/max clip

// RWTH_TRACE_CABAC
#define RWTH_TRACE_CABAC 0
#if RWTH_TRACE_CABAC
#if RWTH_TRACE_CABAC_TO_FILE
#define DTRACE_CABAC_N fprintf(c,"\n");
#define DTRACE_CABAC_V(x) fprintf(c,"%i", x);
#define DTRACE_CABAC_T(x) fprintf(c,"%s", x);
#define DTRACE_CABAC_VB(x,numBits) fprintf(c,"%i,%i",x,numBits);
#else
#define DTRACE_CABAC_N printf("\n");
#define DTRACE_CABAC_V(x) printf("%i", x);
#define DTRACE_CABAC_T(x) printf("%s", x);
#define DTRACE_CABAC_VB(x,numBits) printf("%i,%i",x,numBits);
#endif
#else
#define DTRACE_CABAC_N 
#define DTRACE_CABAC_V(x)
#define DTRACE_CABAC_T(x)
#define DTRACE_CABAC_VB(x,numBits)
#endif

#endif
