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
 
#include "mex.h"
#include "matrix.h"

#include <iostream>
#include <stdint.h>
#include <fstream>
#include <list>
#include <math.h>
#include <assert.h>
#include <vector>
#include <bitset>

#include "CABAC_ArithmeticEncoder.h"
#include "CABAC_ArithmeticDecoder.h"
#include "CABAC_BitstreamFile.h"
#include "ContextModel.h"
#include "CABAC_ContextModelsInit.h"

#include "CABAC_ArithmeticEncoder.cpp"
#include "CABAC_ArithmeticDecoder.cpp"
#include "CABAC_BitstreamFile.cpp"
#include "CABAC_ContextModelsInit.cpp"
#include "ContextModel.cpp"


using namespace std;

void _main();

// this is the CABAC base class, which contains the encoder and decoder and all models used by them
// coding can be done using a specific contexts
// this class can be easily modified if more contexts are needed
// output bits are written into or input bits are read from a CABAC_BitstreamFile

// TODO: make the CABAC class a singleton implementation
class CABAC {
public:
  CABAC() {};
  ~CABAC() {};
  char* fn;
  bool bFileNameIsSet;
  CABAC_BitstreamFile bitStream;
  CABAC_ContextModels encoderModels;
  CABAC_ContextModels decoderModels;
  CABAC_ArithmeticEncoder encoder;
  CABAC_ArithmeticDecoder decoder;
};


// function to retrieve the CABAC instance
static CABAC* getPointer(const mxArray *prhs[])
{
  uintptr_t c_value = *mxGetPr(prhs[1]);
  CABAC *c_pointer =  (CABAC *)c_value;
  if (c_pointer==NULL)
  {
    mexErrMsgTxt("Error: No initialized CABAC instance provided \n");
    assert(c_pointer);
  }
#if RWTH_CABAC_DEBUG_OUTPUT && 0
  mexPrintf("Status: CABAC pointer retrived in CPP, Pointer address: %p\n", c_pointer );
#endif
  return c_pointer;
};

// the MEX interface function
void mexFunction(
  int               nlhs, 		// Number of expected output mxArrays
  mxArray 		    *plhs[],  	// Array of pointers to the expected output mxArrays
  int       		nrhs,		// Number of input mxArrays
  const mxArray 	*prhs[]		// Array of pointers to the input mxArrays
)
{
  CABAC *c; // pointer to (new) instance of the CABAC class 
  char* fn = 0;
  char cmd[64]; // temp char array to hold the command

  if (nrhs < 1 || !mxIsClass(prhs[0], "char") || mxGetString(prhs[0], cmd, sizeof(cmd))) 
  {
    mexErrMsgTxt("Error: input 0 must be a valid keyword - init, encodeStart, encodeBin, encodeFinish, decodeStart, decodeBin, decodeFinish\n");
  }

  // start parsing the input command

  string inputCmd(cmd);

  if (inputCmd == "initByState" || inputCmd == "initByProb")
  {
    // get the filename string
    if (nrhs<2) 
    { 
      mexErrMsgTxt("Error: please provide the filename string and the context initializations\n"); 
    }
    else if (nrhs>3) 
    { 
      mexErrMsgTxt("Error: too many parameters, provide the filename and the context initializations");
    }
    else 
    {
     if (!mxIsClass(prhs[1], "char")) 
     {
        mexErrMsgTxt("Error: invalid filename \n");
     }
     else
     {
       fn = mxArrayToString(prhs[1]);
       if (!mxIsClass(prhs[2], "double"))
       {
         mexErrMsgTxt("Error: invalid context initialization\n");
       }
       else
       {
         // all clear

         // set up an instance
         c = new CABAC;
#if RWTH_CABAC_DEBUG_OUTPUT
         mexPrintf("CABAC instance created. Pointer address c = %p\n", c);
#endif
         uintptr_t c_value = (uintptr_t)c;
         plhs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
         *mxGetPr(plhs[0]) = c_value; // return pointer to matlab environment

         // copy the filename
         //memcpy(c->fn, fn, sizeof(fn));
         c->fn = fn;
         
         if (inputCmd == "initByState")
         {
           //init the contexts
           int numberOfContextModels = mxGetNumberOfElements(prhs[2]) / 3;

           c->encoderModels.initContextModelsByMpsState(numberOfContextModels, prhs[2]);
           c->decoderModels.initContextModelsByMpsState(numberOfContextModels, prhs[2]);
         }
         else
         {
           //init the contexts
           int numberOfContextModels = mxGetNumberOfElements(prhs[2]);

           c->encoderModels.initContextModelsByP0Prob(numberOfContextModels, prhs[2]);
           c->decoderModels.initContextModelsByP0Prob(numberOfContextModels, prhs[2]);
         }

#if RWTH_CABAC_DEBUG_OUTPUT
         mexPrintf("Status: CABAC successfully initialized. Writing / Reading from %s \n",c->fn);
#endif
       }
     }
    }
  }
  else if (inputCmd == "encodeStart")
  {
    if (nrhs < 2) 
    {
      mexErrMsgTxt("Error: You need to provide the pointer to the initialized CABAC engine \n");
    }
    c = getPointer(prhs);
    assert(c);
    // open bitstream for writing
    if (!c->bitStream.openOutputFile(c->fn)) 
    {
      mexPrintf("Error: filename %s cannot be opened for writing\n", c->fn);
      mexErrMsgTxt("Error: bitstreamfile access error\n");
    }
#if RWTH_CABAC_DEBUG_OUTPUT
    mexPrintf("Status: filename: %s opened for writing\n", c->fn);
#endif
    // set bitstream to encoder
    c->encoder.setBitstream(&(c->bitStream));
    // start the encoder
    c->encoder.start();
    // initialize the context model
  
  }
  else if (inputCmd == "encodeBin")
  {
    if (nrhs < 2) 
    {
      mexErrMsgTxt("Error: You need to provide the pointer to the initialized CABAC engine \n");
    }
    c = getPointer(prhs);
    assert(c);
    unsigned int encodedBin;
    if (nrhs != 4) 
    { 
      mexErrMsgTxt("Error: invalid input, provide the bin and context index to be encoded with\n"); 
    }
    encodedBin = static_cast<unsigned int>((*mxGetPr(prhs[2])));
    int ctx_idx = static_cast<int>((*mxGetPr(prhs[3])));
    if (encodedBin != 0 && encodedBin != 1) 
    { 
      mexErrMsgTxt("Error: invalid input 3, bin to be encoded should either be 1 or 0\n"); 
    }
    else
    {
#if RWTH_TRACE_CABAC_STATES
      uint8_t bin = encodedBin;
      uint8_t mps_p = c->encoderModels.getContextModel(ctx_idx)->getMps();
      uint8_t state_p = c->encoderModels.getContextModel(ctx_idx)->getState();
#endif
      // encode bin
      c->encoder.encodeBin(encodedBin, c->encoderModels.getContextModel(ctx_idx));
#if RWTH_TRACE_CABAC_STATES
      uint8_t mps_a = c->encoderModels.getContextModel(ctx_idx)->getMps();
      uint8_t state_a = c->encoderModels.getContextModel(ctx_idx)->getState();
      c->encoderModels.getContextModel(ctx_idx)->addCabacStep(bin, state_p, mps_p, state_a, mps_a);
#endif
#if RWTH_CABAC_DEBUG_OUTPUT
      mexPrintf("Status: bin value %d encoded into context %d\n", encodedBin,ctx_idx);
#endif
    }
  }
  else if (inputCmd == "getNumBits")
  {
    if (nrhs < 2) 
    {
      mexErrMsgTxt("Error: You need to provide the pointer to the initialized CABAC engine \n");
    }
    c = getPointer(prhs);
    assert(c);
    unsigned int encodedBin;
    if (nrhs != 2) 
    { 
      mexErrMsgTxt("Error: invalid input\n"); 
    }
    unsigned int bits = c->bitStream.getNumberOfWrittenBits();
    plhs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
    *mxGetPr(plhs[0]) = bits;
  }
  else if (inputCmd == "encodeFinish")
  {
    if (nrhs < 2) 
    {
      mexErrMsgTxt("Error: You need to provide the pointer to the initialized CABAC engine \n");
    }
    c = getPointer(prhs);
    assert(c);
    c->encoder.finish();
    c->bitStream.closeFile();
#if RWTH_CABAC_DEBUG_OUTPUT
    mexPrintf("Status: encoding finished, outstream closed\n");
    mexPrintf("Status: number of written bits: %d\n", c->bitStream.getNumberOfWrittenBits());
#endif
  }
  else if (inputCmd == "decodeStart")
  {
    if (nrhs < 2) 
    {
      mexErrMsgTxt("Error: You need to provide the pointer to the initialized CABAC engine \n");
    }
    c = getPointer(prhs);
    // open bitstream for reading
    if (!c->bitStream.openInputFile(c->fn)) 
    {
      mexPrintf("Error: filename %s cannot be opened for reading\n", c->fn);
      mexErrMsgTxt("Error: bitstreamfile access error\n");
    }
#if RWTH_CABAC_DEBUG_OUTPUT
    mexPrintf("Status: filename %s opened for reading\n", c->fn);
#endif
    // set bitstream to decoder
    c->decoder.setBitstream(&(c->bitStream));
    // start the decoder
    c->decoder.start();
  }
  else if (inputCmd == "decodeBin")
  {
    if (nrhs < 2) 
    {
      mexErrMsgTxt("Error: You need to provide the pointer to the initialized CABAC engine \n");
    }
    if (nlhs != 1) 
    { 
      mexErrMsgTxt("Error: invalid command, provide a variable to store the decoded bin \n"); 
    }
    else
    {
      c = getPointer(prhs);
      assert(c);
      unsigned int decodedBin = 0;
      int ctx_idx = static_cast<int>((*mxGetPr(prhs[2])));
      // decode Bin
#if RWTH_TRACE_CABAC_STATES
      uint8_t bin = decodedBin;
      uint8_t mps_p = c->decoderModels.getContextModel(ctx_idx)->getMps();
      uint8_t state_p = c->decoderModels.getContextModel(ctx_idx)->getState();
#endif
      c->decoder.decodeBin(decodedBin, c->decoderModels.getContextModel(ctx_idx));
#if RWTH_TRACE_CABAC_STATES
      uint8_t mps_a = c->decoderModels.getContextModel(ctx_idx)->getMps();
      uint8_t state_a = c->decoderModels.getContextModel(ctx_idx)->getState();
      c->decoderModels.getContextModel(ctx_idx)->addCabacStep(bin, state_p, mps_p, state_a, mps_a);
#endif
#if RWTH_CABAC_DEBUG_OUTPUT
      mexPrintf("Status: bin value %d decoded from context %d\n", decodedBin,ctx_idx);
#endif
      plhs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
      *mxGetPr(plhs[0]) = decodedBin;
    }
  }
  else if (inputCmd == "decodeFinish")
  {
    if (nrhs < 2) 
    {
      mexErrMsgTxt("Error: You need to provide the pointer to the initialized CABAC engine \n");
    }
    c = getPointer(prhs);
    assert(c);
    c->decoder.finish();
    c->bitStream.closeFile();
#if RWTH_CABAC_DEBUG_OUTPUT
    mexPrintf("Status: decoding finished, instream closed\n");
#endif
#if RWTH_TRACE_CABAC_STATES && RWTH_TRACE_CABAC_TO_FILE
    FILE* traceFile = fopen("CABAC_DEC_STATS_TRACE.log", "w");
    c->decoderModels.writeCompleteTraces(traceFile);
    fclose(traceFile);
#endif
  }
#if RWTH_TRACE_CABAC_STATES
  else if (inputCmd == "getEncoderStats")
  {
    if (nrhs < 2) 
    {
      mexErrMsgTxt("Error: You need to provide the pointer to the initialized CABAC engine \n");
    }
    if (nlhs != 2) 
    { 
      mexErrMsgTxt("Error: invalid command, provide two variables to store the trace \n"); 
    }
    else
    {
        c = getPointer(prhs);
        int ctx_idx = static_cast<int>((*mxGetPr(prhs[2])));
        std::vector<CABACStep>* stepsTrace = c->encoderModels.getContextModel(ctx_idx)->getCabacSteps();
        std::vector<std::vector<unsigned int>>* transTrace = c->encoderModels.getContextModel(ctx_idx)->getCabactTransitions();

        //plhs[0] = mxCreateNumericMatrix(5, stepsTrace->size(), mxUINT8_CLASS, mxREAL);
        plhs[0] = mxCreateNumericMatrix(5, stepsTrace->size(), mxUINT8_CLASS, mxREAL);
        if (plhs[0])
        {
          uint8_t* out1Data = (uint8_t*)mxGetData(plhs[0]);
          for (int entry=0;entry<stepsTrace->size();entry++)
          { 
            CABACStep temp = stepsTrace->at(entry);
            out1Data[entry * 5 + 0] = temp.codedBin;
            out1Data[entry * 5 + 1] = temp.state_p;
            out1Data[entry * 5 + 2] = temp.mps_p;
            out1Data[entry * 5 + 3] = temp.state_a;
            out1Data[entry * 5 + 4] = temp.mps_a;
          }
        }
        else
        {
          printf("output0 array was not created\n");
        }

        plhs[1] = mxCreateNumericMatrix(RWTH_TRACE_CABAC_STATES_NUM_STATES, RWTH_TRACE_CABAC_STATES_NUM_STATES,mxUINT32_CLASS, mxREAL);
        if (plhs[1])
        {     
          unsigned int* out2Data = (unsigned int*)mxGetData(plhs[1]);
          for (int state_p = 0; state_p < RWTH_TRACE_CABAC_STATES_NUM_STATES; state_p++)
          {
            for (int state_a = 0; state_a < RWTH_TRACE_CABAC_STATES_NUM_STATES; state_a++)
            {
              unsigned int temp = (*transTrace)[state_p][state_a];
              out2Data[state_p * RWTH_TRACE_CABAC_STATES_NUM_STATES + state_a] = temp;
            }
          }
        }
        else
        {
          printf("output1 array was not created\n");
        }
    }
  }
  else if (inputCmd == "getDecoderStats")
  {
    if (nrhs < 2) 
    {
      mexErrMsgTxt("Error: You need to provide the pointer to the initialized CABAC engine \n");
    }
    if (nlhs != 2) 
    { 
      mexErrMsgTxt("Error: invalid command, provide two variables to store the trace \n"); 
    }
    else
    {
        c = getPointer(prhs);
        int ctx_idx = static_cast<int>((*mxGetPr(prhs[2])));
        std::vector<CABACStep>* stepsTrace = c->decoderModels.getContextModel(ctx_idx)->getCabacSteps();
        std::vector<std::vector<unsigned int>>* transTrace = c->decoderModels.getContextModel(ctx_idx)->getCabactTransitions();

        plhs[0] = mxCreateNumericMatrix(5,stepsTrace->size(), mxUINT8_CLASS, mxREAL);
        if (plhs[0])
        {
          uint8_t* out1Data = (uint8_t*)mxGetData(plhs[0]);
          for (int entry = 0; entry < stepsTrace->size(); entry++)
          {
            CABACStep temp = stepsTrace->at(entry);
            out1Data[entry * 5 + 0] = temp.codedBin;
            out1Data[entry * 5 + 1] = temp.state_p;
            out1Data[entry * 5 + 2] = temp.mps_p;
            out1Data[entry * 5 + 3] = temp.state_a;
            out1Data[entry * 5 + 4] = temp.mps_a;
          }
        }
        else
        {
          printf("output0 array was not created\n");
        }

        plhs[1] = mxCreateNumericMatrix(RWTH_TRACE_CABAC_STATES_NUM_STATES, RWTH_TRACE_CABAC_STATES_NUM_STATES, mxUINT32_CLASS, mxREAL);
        if (plhs[1])
        {
          unsigned int* out2Data = (unsigned int*)mxGetData(plhs[1]);
          for (int state_p = 0; state_p < RWTH_TRACE_CABAC_STATES_NUM_STATES; state_p++)
          {
            for (int state_a = 0; state_a < RWTH_TRACE_CABAC_STATES_NUM_STATES; state_a++)
            {
              unsigned int temp = (*transTrace)[state_p][state_a];
              out2Data[state_p * RWTH_TRACE_CABAC_STATES_NUM_STATES + state_a] = temp;
            }
          }
        }
        else
        {
          printf("output1 array was not created\n");
        }
    }
  }
#endif
  else
  {
    mexErrMsgTxt("Error: Invalid Command\n");
  } 
}
