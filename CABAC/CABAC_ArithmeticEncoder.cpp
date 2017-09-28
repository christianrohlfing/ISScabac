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
 
#include "CABAC_ArithmeticEncoder.h"
#include <assert.h>

CABAC_ArithmeticEncoder::CABAC_ArithmeticEncoder(CABAC_BitstreamFile* ptCabacBitstream)
{
  m_ptBitstream = ptCabacBitstream;
#if RWTH_TRACE_CABAC_TO_FILE
  c = fopen("CABAC_ENC_TRACE.log", "w");
#endif
}

CABAC_ArithmeticEncoder::~CABAC_ArithmeticEncoder()
{
}

void CABAC_ArithmeticEncoder::setBitstream( CABAC_BitstreamFile* ptCabacBitstream )
{
  m_ptBitstream = ptCabacBitstream;
}

void CABAC_ArithmeticEncoder::start()
{
  m_uiLow            = 0;
  m_uiRange          = 510;
  m_bitsLeft         = 23;
  m_numBufferedBytes = 0;
  m_bufferedByte     = 0xff;
  m_uiBinsCoded      = 0;

  DTRACE_CABAC_T( "CABAC start: L=" );
  DTRACE_CABAC_V(m_uiLow);
  DTRACE_CABAC_T(" R=");
  DTRACE_CABAC_V(m_uiRange);
  DTRACE_CABAC_N;
}

void CABAC_ArithmeticEncoder::finish()
{
  // Encode the terminating bit
  encodeBinTrm(1);
  
  // write out remaining bits
  if ( m_uiLow >> ( 32 - m_bitsLeft ) )
  {
    //assert( m_numBufferedBytes > 0 );
    //assert( m_bufferedByte != 0xff );
    m_ptBitstream->write( m_bufferedByte + 1, 8 );
    while ( m_numBufferedBytes > 1 )
    {
      m_ptBitstream->write( 0x00, 8 );
      m_numBufferedBytes--;
    }
    m_uiLow -= 1 << ( 32 - m_bitsLeft );
  }
  else
  {
    if ( m_numBufferedBytes > 0 )
    {
      m_ptBitstream->write( m_bufferedByte, 8 );
    }
    while ( m_numBufferedBytes > 1 )
    {
      m_ptBitstream->write( 0xff, 8 );
      m_numBufferedBytes--;
    }    
  }
  m_ptBitstream->write( m_uiLow >> 8, 24 - m_bitsLeft );

  // Terminate the bitstream
  m_ptBitstream->write(1, 1);
  m_ptBitstream->writeAlignZero();
}

/**
 * \brief Encode bin
 *
 * \param binValue   bin value
 * \param rcCtxModel context model
 */
void CABAC_ArithmeticEncoder::encodeBin( unsigned int binValue, ContextModel *rcCtxModel )
{
#if RWTH_TRACE_CABAC
  DTRACE_CABAC_T("EncodeBinSymbol=");
  DTRACE_CABAC_V(binValue);
  DTRACE_CABAC_T(", StateTrans=(");
  DTRACE_CABAC_V(rcCtxModel->getMps());
  DTRACE_CABAC_T(",");
  DTRACE_CABAC_V(rcCtxModel->getState());
  DTRACE_CABAC_T(")->(");
#endif
  
  m_uiBinsCoded++;
  
  unsigned int  uiLPS   = sm_aucLPSTable[ rcCtxModel->getState() ][ ( m_uiRange >> 6 ) & 3 ];
  m_uiRange    -= uiLPS;
  
  if( binValue != rcCtxModel->getMps() )
  {
    // Coding a LPS
    // Write out numBits
    int numBits = sm_aucRenormTable[ uiLPS >> 3 ];
    m_uiLow     = ( m_uiLow + m_uiRange ) << numBits;
    m_uiRange   = uiLPS << numBits;
    rcCtxModel->updateLPS();
    
    m_bitsLeft -= numBits;
  }
  else
  {
    // Coding a MPS
    rcCtxModel->updateMPS();
    if ( m_uiRange >= 256 )
    {
      // No renormalization required
      DTRACE_CABAC_V(rcCtxModel->getMps());
      DTRACE_CABAC_T(",");
      DTRACE_CABAC_V(rcCtxModel->getState());
      DTRACE_CABAC_T("), L=");
      DTRACE_CABAC_V(m_uiLow);
      DTRACE_CABAC_T(", R=");
      DTRACE_CABAC_V(m_uiRange);
      DTRACE_CABAC_T(", LPS=");
      DTRACE_CABAC_V(uiLPS);
      DTRACE_CABAC_N;
      return;
    }
    
    // Write out 1 bit
    m_uiLow <<= 1;
    m_uiRange <<= 1;
    m_bitsLeft--;
  }

  DTRACE_CABAC_V(rcCtxModel->getMps());
  DTRACE_CABAC_T(",");
  DTRACE_CABAC_V(rcCtxModel->getState());
  DTRACE_CABAC_T("), L=");
  DTRACE_CABAC_V(m_uiLow);
  DTRACE_CABAC_T(", R=");
  DTRACE_CABAC_V(m_uiRange);
  DTRACE_CABAC_T(", LPS=");
  DTRACE_CABAC_V(uiLPS);
  DTRACE_CABAC_N;
  testAndWriteOut();
}

#if RWTH_CABAC_FIXED_PROBABILITY
/**
 * Encode a bit with a certain probability. This function requires no context update operations.
 * \param binValue bin Vale
 * \param uiProbability The probability of the bit to encode being one in percent. (1...99).
 */
void CABAC_ArithmeticEncoder::encodeBinProb(unsigned int  binValue, unsigned int uiProbability)
{
  if (uiProbability == 50)
  {
    encodeBinEP(binValue);
    return;
  }
  
  DTRACE_CABAC_T("CABAC encodeBin fixed symbol=");
  DTRACE_CABAC_V(binValue);
  DTRACE_CABAC_T(" p=");
  DTRACE_CABAC_V(uiProbability);
  assert(uiProbability > 0 && uiProbability < 100);
  
  m_uiBinsCoded++;
  unsigned int uiMPS = (uiProbability > 50) ? 1 : 0;
  // The probability of the bit being the MPS where: 0 -> 50%, 49 -> 99%
  unsigned int uiProbMPS = (uiProbability < 50) ? 50 - uiProbability : uiProbability - 50;
    
  unsigned int  uiLPS = sm_aucLPSTProbTable[uiProbMPS-1][(m_uiRange >> 6) & 3];
  m_uiRange -= uiLPS;

  if (binValue != uiMPS)
  {
    // least probable symbol being send
    int numBits = sm_aucRenormTable[uiLPS >> 3];
    m_uiLow = (m_uiLow + m_uiRange) << numBits;
    m_uiRange = uiLPS << numBits;

    m_bitsLeft -= numBits;
  }
  else
  {
    if (m_uiRange >= 256)
    {
      DTRACE_CABAC_T(" L=");
      DTRACE_CABAC_V(m_uiLow);
      DTRACE_CABAC_T(" R=");
      DTRACE_CABAC_V(m_uiRange);
      DTRACE_CABAC_T(" LPS=");
      DTRACE_CABAC_V(uiLPS);
      DTRACE_CABAC_N;
      return;
    }
    m_uiLow <<= 1;
    m_uiRange <<= 1;
    m_bitsLeft--;
  }
  DTRACE_CABAC_T(" L=");
  DTRACE_CABAC_V(m_uiLow);
  DTRACE_CABAC_T(" R=");
  DTRACE_CABAC_V(m_uiRange);
  DTRACE_CABAC_T(" LPS=");
  DTRACE_CABAC_V(uiLPS);
  DTRACE_CABAC_N;
  testAndWriteOut();
}
#endif

/**
 * \brief Encode equiprobable bin
 *
 * \param binValue bin value
 */
void CABAC_ArithmeticEncoder::encodeBinEP( unsigned int binValue )
{
  m_uiBinsCoded++;
  m_uiLow <<= 1;
  if( binValue )
  {
    m_uiLow += m_uiRange;
  }
  m_bitsLeft--;

  DTRACE_CABAC_T("CABAC encodeEPBin symbol=");
  DTRACE_CABAC_V( binValue );
  DTRACE_CABAC_T( " EP" );
  DTRACE_CABAC_T(" L=");
  DTRACE_CABAC_V(m_uiLow);
  DTRACE_CABAC_T(" R=");
  DTRACE_CABAC_V(m_uiRange);
  DTRACE_CABAC_N;

  testAndWriteOut();
}

/**
 * \brief Encode equiprobable bins
 *
 * \param binValues bin values
 * \param numBins number of bins
 */
void CABAC_ArithmeticEncoder::encodeBinsEP( unsigned int binValues, int numBins )
{
  m_uiBinsCoded += numBins;
  
  while ( numBins > 8 )
  {
    numBins -= 8;
    unsigned int pattern = binValues >> numBins; 
    m_uiLow <<= 8;
    m_uiLow += m_uiRange * pattern;
    binValues -= pattern << numBins;
    m_bitsLeft -= 8;

    DTRACE_CABAC_T("CABAC encode EP 8_symbol=");
    DTRACE_CABAC_VB(pattern, 8);
    DTRACE_CABAC_T(" EP");
    DTRACE_CABAC_T(" L=");
    DTRACE_CABAC_V(m_uiLow);
    DTRACE_CABAC_T(" R=");
    DTRACE_CABAC_V(m_uiRange);
    DTRACE_CABAC_N;

    testAndWriteOut();
  }
  
  m_uiLow <<= numBins;
  m_uiLow += m_uiRange * binValues;
  m_bitsLeft -= numBins;

  DTRACE_CABAC_T("CABAC encode EP ");
  DTRACE_CABAC_V(numBins);
  DTRACE_CABAC_T("_symbol=");
  DTRACE_CABAC_VB(binValues, numBins);
  DTRACE_CABAC_T(" EP");
  DTRACE_CABAC_T(" L=");
  DTRACE_CABAC_V(m_uiLow);
  DTRACE_CABAC_T(" R=");
  DTRACE_CABAC_V(m_uiRange);
  DTRACE_CABAC_N;

  testAndWriteOut();
}

/**
 * \brief Encode terminating bin
 *
 * \param binValue bin value
 */
void CABAC_ArithmeticEncoder::encodeBinTrm( unsigned int binValue )
{
  DTRACE_CABAC_T("CABAC code Terminating Bin symbol=");
  DTRACE_CABAC_V( binValue );

  m_uiBinsCoded += 1;
  m_uiRange -= 2;
  if( binValue )
  {
    // Terminating bit 1
    // This will produce 7 output bits
    m_uiLow  += m_uiRange;
    m_uiLow <<= 7;
    m_uiRange = 2 << 7;
    m_bitsLeft -= 7;
  }
  else if ( m_uiRange >= 256 )
  {
    // Terminating bit 0
    // Not normalization required
    DTRACE_CABAC_T(" L=");
    DTRACE_CABAC_V(m_uiLow);
    DTRACE_CABAC_T(" R=");
    DTRACE_CABAC_V(m_uiRange);
    DTRACE_CABAC_N;
    return;
  }
  else
  {
    // Terminating bit 0
    // Normalization required
    m_uiLow   <<= 1;
    m_uiRange <<= 1;
    m_bitsLeft--;    
  }
  DTRACE_CABAC_T(" L=");
  DTRACE_CABAC_V(m_uiLow);
  DTRACE_CABAC_T(" R=");
  DTRACE_CABAC_V(m_uiRange);
  DTRACE_CABAC_N;
  testAndWriteOut();
}

void CABAC_ArithmeticEncoder::testAndWriteOut()
{
  if ( m_bitsLeft < 12 )
  {
    writeOut();
  }
}

/**
 * \brief Move bits from register into bitstream
 */
void CABAC_ArithmeticEncoder::writeOut()
{
  unsigned int leadByte = m_uiLow >> (24 - m_bitsLeft);
  m_bitsLeft += 8;
  m_uiLow &= 0xffffffffu >> m_bitsLeft;
  
  if ( leadByte == 0xff )
  {
    m_numBufferedBytes++;
  }
  else
  {
    if ( m_numBufferedBytes > 0 )
    {
      unsigned int carry = leadByte >> 8;
      unsigned int byte = m_bufferedByte + carry;
      m_bufferedByte = leadByte & 0xff;
      m_ptBitstream->write( byte, 8 );
      
      byte = ( 0xff + carry ) & 0xff;
      while ( m_numBufferedBytes > 1 )
      {
        m_ptBitstream->write( byte, 8 );
        m_numBufferedBytes--;
      }
    }
    else
    {
      m_numBufferedBytes = 1;
      m_bufferedByte = leadByte;
    }      
  }    
}

const unsigned char CABAC_ArithmeticEncoder::sm_aucLPSTable[64][4] =
{
  { 128, 176, 208, 240},
  { 128, 167, 197, 227},
  { 128, 158, 187, 216},
  { 123, 150, 178, 205},
  { 116, 142, 169, 195},
  { 111, 135, 160, 185},
  { 105, 128, 152, 175},
  { 100, 122, 144, 166},
  {  95, 116, 137, 158},
  {  90, 110, 130, 150},
  {  85, 104, 123, 142},
  {  81,  99, 117, 135},
  {  77,  94, 111, 128},
  {  73,  89, 105, 122},
  {  69,  85, 100, 116},
  {  66,  80,  95, 110},
  {  62,  76,  90, 104},
  {  59,  72,  86,  99},
  {  56,  69,  81,  94},
  {  53,  65,  77,  89},
  {  51,  62,  73,  85},
  {  48,  59,  69,  80},
  {  46,  56,  66,  76},
  {  43,  53,  63,  72},
  {  41,  50,  59,  69},
  {  39,  48,  56,  65},
  {  37,  45,  54,  62},
  {  35,  43,  51,  59},
  {  33,  41,  48,  56},
  {  32,  39,  46,  53},
  {  30,  37,  43,  50},
  {  29,  35,  41,  48},
  {  27,  33,  39,  45},
  {  26,  31,  37,  43},
  {  24,  30,  35,  41},
  {  23,  28,  33,  39},
  {  22,  27,  32,  37},
  {  21,  26,  30,  35},
  {  20,  24,  29,  33},
  {  19,  23,  27,  31},
  {  18,  22,  26,  30},
  {  17,  21,  25,  28},
  {  16,  20,  23,  27},
  {  15,  19,  22,  25},
  {  14,  18,  21,  24},
  {  14,  17,  20,  23},
  {  13,  16,  19,  22},
  {  12,  15,  18,  21},
  {  12,  14,  17,  20},
  {  11,  14,  16,  19},
  {  11,  13,  15,  18},
  {  10,  12,  15,  17},
  {  10,  12,  14,  16},
  {   9,  11,  13,  15},
  {   9,  11,  12,  14},
  {   8,  10,  12,  14},
  {   8,   9,  11,  13},
  {   7,   9,  11,  12},
  {   7,   9,  10,  12},
  {   7,   8,  10,  11},
  {   6,   8,   9,  11},
  {   6,   7,   9,  10},
  {   6,   7,   8,   9},
  {   2,   2,   2,   2}
};

const unsigned char CABAC_ArithmeticEncoder::sm_aucRenormTable[32] =
{
  6,  5,  4,  4,
  3,  3,  3,  3,
  2,  2,  2,  2,
  2,  2,  2,  2,
  1,  1,  1,  1,
  1,  1,  1,  1,
  1,  1,  1,  1,
  1,  1,  1,  1
};

#if RWTH_CABAC_FIXED_PROBABILITY
const unsigned char CABAC_ArithmeticEncoder::sm_aucLPSTProbTable[49][4] =
{
  { 139, 171, 202, 234 },
  { 136, 167, 198, 229 },
  { 134, 164, 194, 224 },
  { 131, 160, 190, 220 },
  { 128, 157, 186, 215 },
  { 125, 153, 182, 210 },
  { 122, 150, 178, 205 },
  { 119, 146, 174, 201 },
  { 116, 143, 169, 196 },
  { 114, 139, 165, 191 },
  { 111, 136, 161, 186 },
  { 108, 132, 157, 181 },
  { 105, 129, 153, 177 },
  { 102, 126, 149, 172 },
  { 99, 122, 145, 167 },
  { 97, 119, 140, 162 },
  { 94, 115, 136, 158 },
  { 91, 112, 132, 153 },
  { 88, 108, 128, 148 },
  { 85, 105, 124, 143 },
  { 82, 101, 120, 138 },
  { 80, 98, 116, 134 },
  { 77, 94, 112, 129 },
  { 74, 91, 107, 124 },
  { 71, 87, 103, 119 },
  { 68, 84, 99, 115 },
  { 65, 80, 95, 110 },
  { 62, 77, 91, 105 },
  { 60, 73, 87, 100 },
  { 57, 70, 83, 95 },
  { 54, 66, 78, 91 },
  { 51, 63, 74, 86 },
  { 48, 59, 70, 81 },
  { 45, 56, 66, 76 },
  { 43, 52, 62, 72 },
  { 40, 49, 58, 67 },
  { 37, 45, 54, 62 },
  { 34, 42, 50, 57 },
  { 31, 38, 45, 53 },
  { 28, 35, 41, 48 },
  { 26, 31, 37, 43 },
  { 23, 28, 33, 38 },
  { 20, 24, 29, 33 },
  { 17, 21, 25, 29 },
  { 14, 17, 21, 24 },
  { 11, 14, 17, 19 },
  { 9, 10, 12, 14 },
  { 6, 7, 8, 10 },
  { 4, 4, 4, 5 }
};
#endif