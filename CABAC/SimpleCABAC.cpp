/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.  
 *
 * Copyright (c) 2016-2017, Institut für Nachrichtentechnik, RWTH Aachen University
 *
 * Christian Feldmann
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
 
#include <iostream>
#include <fstream>
#include <list>
#include <math.h>
#include <assert.h>

#include "CABAC_ArithmeticEncoder.h"
#include "CABAC_ArithmeticDecoder.h"
#include "CABAC_BitstreamFile.h"
#include "ContextModel.h"
#include "CommonDef.h"

using namespace std;

void codeToFile()
{
  // Create and open the bitstream to write to
  CABAC_BitstreamFile outStream;
  if (!outStream.openOutputFile("str.bin"))
  {
    fprintf(stderr, "\nfailed to open bitstream file `str.bin' for writing\n");
    return;
  }
  printf("Opened 'str.bin' for writing.\n");
  // Create the arithmetic coder (and provide the bitstream it shall write to)
  CABAC_ArithmeticEncoder arithmeticEncoder(&outStream);
  arithmeticEncoder.start();
  
  // Create a context, initialize it and code 001011 to is
  ContextModel ctx0;
  ctx0.init(0,20);    // Optional: Initialize the context to something other than equal probability.
  arithmeticEncoder.encodeBin( 0, &ctx0 );
  arithmeticEncoder.encodeBin( 0, &ctx0 );
  arithmeticEncoder.encodeBin( 1, &ctx0 );
  arithmeticEncoder.encodeBin( 0, &ctx0 );
  arithmeticEncoder.encodeBin( 1, &ctx0 );
  arithmeticEncoder.encodeBin( 1, &ctx0 );
    
  // Encode 5 EP bits (10010)
  arithmeticEncoder.encodeBinEP(1);
  arithmeticEncoder.encodeBinEP(0);
  arithmeticEncoder.encodeBinEP(0);
  arithmeticEncoder.encodeBinEP(1);
  arithmeticEncoder.encodeBinEP(0);

  // Encode the same 5 bits (10010) using the encodeBinsEP function
  arithmeticEncoder.encodeBinsEP(18, 5);

  // Create another context and code 110111 to it
  ContextModel ctx1;
  arithmeticEncoder.encodeBin( 1, &ctx1 );
  arithmeticEncoder.encodeBin( 1, &ctx1 );
  arithmeticEncoder.encodeBin( 0, &ctx1 );
  arithmeticEncoder.encodeBin( 1, &ctx1 );
  arithmeticEncoder.encodeBin( 1, &ctx1 );
  arithmeticEncoder.encodeBin( 1, &ctx1 );

#if RWTH_CABAC_FIXED_PROBABILITY
  // Encode some bits with a fixed probabilty
  arithmeticEncoder.encodeBinProb(1, 10);
  arithmeticEncoder.encodeBinProb(0, 10);
  arithmeticEncoder.encodeBinProb(0, 10);

  arithmeticEncoder.encodeBinProb(1, 30);
  arithmeticEncoder.encodeBinProb(1, 30);
  arithmeticEncoder.encodeBinProb(0, 30);
#endif

  // Finish coding
  arithmeticEncoder.finish();
  outStream.closeFile();
    
#if RWTH_TRACE_CABAC_STATES && RWTH_TRACE_CABAC_TO_FILE
  // Coding is complete. Write CABAC stats to file
  // Open the output file to write the CABAC state counters to
  FILE *m_cTraceCabacStatFile = fopen("CabacStats.log", "w");
  // Trace all ctx to the file
  ctx0.traceStatesToFile(m_cTraceCabacStatFile);
  ctx1.traceStatesToFile(m_cTraceCabacStatFile);
  // Close the CABAC stat file
  fclose(m_cTraceCabacStatFile);
#endif
}

void decodeFromFile()
{
  // Create and open the input bitstream
  CABAC_BitstreamFile inStream;
  if (!inStream.openInputFile("str.bin"))
  {
    fprintf(stderr, "\nfailed to open bitstream file `str.bin' for reading\n");
    return;
  }
  printf("Opened 'str.bin' for reading.\n");
  
  CABAC_ArithmeticDecoder arithmeticDecoder(&inStream);
  arithmeticDecoder.start();
  unsigned int uiBit;

  // Create a context, initialize it, and decode 6 bits from it. (Should be 001011)
  ContextModel ctx0;
  ctx0.init(0,20);    // Optional: Initialize the context to something other than equal probability.
  arithmeticDecoder.decodeBin(uiBit, &ctx0); assert(uiBit == 0);
  arithmeticDecoder.decodeBin(uiBit, &ctx0); assert(uiBit == 0);
  arithmeticDecoder.decodeBin(uiBit, &ctx0); assert(uiBit == 1);
  arithmeticDecoder.decodeBin(uiBit, &ctx0); assert(uiBit == 0);
  arithmeticDecoder.decodeBin(uiBit, &ctx0); assert(uiBit == 1);
  arithmeticDecoder.decodeBin(uiBit, &ctx0); assert(uiBit == 1);

  // Decode 4 EP bits (should be 10010)
  arithmeticDecoder.decodeBinEP(uiBit); assert(uiBit == 1);
  arithmeticDecoder.decodeBinEP(uiBit); assert(uiBit == 0);
  arithmeticDecoder.decodeBinEP(uiBit); assert(uiBit == 0);
  arithmeticDecoder.decodeBinEP(uiBit); assert(uiBit == 1);
  arithmeticDecoder.decodeBinEP(uiBit); assert(uiBit == 0);

  // Decode the same 5 bits (10010) using the decodeBinsEP function
  unsigned int uiVal;
  arithmeticDecoder.decodeBinsEP(uiVal, 5);
  assert(uiVal==18);

  // Create another context and decode six bits (should be 110111)
  ContextModel ctx1;
  arithmeticDecoder.decodeBin(uiBit, &ctx1); assert(uiBit == 1);
  arithmeticDecoder.decodeBin(uiBit, &ctx1); assert(uiBit == 1);
  arithmeticDecoder.decodeBin(uiBit, &ctx1); assert(uiBit == 0);
  arithmeticDecoder.decodeBin(uiBit, &ctx1); assert(uiBit == 1);
  arithmeticDecoder.decodeBin(uiBit, &ctx1); assert(uiBit == 1);
  arithmeticDecoder.decodeBin(uiBit, &ctx1); assert(uiBit == 1);

#if RWTH_CABAC_FIXED_PROBABILITY
  // Encode some bits with a fixed probabilty
  arithmeticDecoder.decodeBinProb(uiBit, 10); assert(uiBit==1);
  arithmeticDecoder.decodeBinProb(uiBit, 10); assert(uiBit==0);
  arithmeticDecoder.decodeBinProb(uiBit, 10); assert(uiBit==0);

  arithmeticDecoder.decodeBinProb(uiBit, 30); assert(uiBit==1);
  arithmeticDecoder.decodeBinProb(uiBit, 30); assert(uiBit==1);
  arithmeticDecoder.decodeBinProb(uiBit, 30); assert(uiBit==0);
#endif

  arithmeticDecoder.finish();
  inStream.closeFile();
}

int main(int argc, char* argv[])
{
  printf("CABAC test environement.\n");
  
  // encode something
  codeToFile();
  
  // decode it again
  decodeFromFile();

  return 0;
}