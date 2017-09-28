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
 
#include "CABAC_ArithmeticDecoder.h"
#include <assert.h>

CABAC_ArithmeticDecoder::CABAC_ArithmeticDecoder(CABAC_BitstreamFile* ptCabacBitstream)
{
  m_ptBitstream = ptCabacBitstream;
#if RWTH_TRACE_CABAC_TO_FILE
  c = fopen("CABAC_DEC_TRACE.log", "w");
#endif
}

CABAC_ArithmeticDecoder::~CABAC_ArithmeticDecoder()
{
}

void CABAC_ArithmeticDecoder::setBitstream( CABAC_BitstreamFile* ptCabacBitstream )
{
  m_ptBitstream = ptCabacBitstream;
}

void CABAC_ArithmeticDecoder::start()
{
  assert( m_ptBitstream->getNumBitsUntilByteAligned() == 0 );
  m_uiRange    = 510;
  m_bitsNeeded = -8;
  m_uiValue    = (m_ptBitstream->readByte() << 8);
  m_uiValue   |= m_ptBitstream->readByte();
#if RWTH_TRACE_CABAC
  m_uiLow = 0;
  m_bitsLeft = 23;
#endif

  DTRACE_CABAC_T( "CABAC start: L=" );
  DTRACE_CABAC_V(m_uiLow);
  DTRACE_CABAC_T(" R=");
  DTRACE_CABAC_V(m_uiRange);
  DTRACE_CABAC_N;
}

void CABAC_ArithmeticDecoder::finish()
{
  // Decode terminating bit and assert it is 1
  unsigned int uiBit;
  decodeBinTrm(uiBit);
  assert(uiBit);

  // Has the bitstream been terminated properly?
  assert( ((m_ptBitstream->getLastByteRead() << (8 + m_bitsNeeded)) & 0xff) == 0x80 );
#if RWTH_TRACE_CABAC_TO_FILE
   fclose(c);
#endif
}

void CABAC_ArithmeticDecoder::decodeBin( unsigned int& ruiBin, ContextModel *rcCtxModel )
{
#if RWTH_TRACE_CABAC
  unsigned int uiMpsBefore = rcCtxModel->getMps();
  unsigned int uiStateBefore = rcCtxModel->getState();
#endif

  unsigned int uiLPS = sm_aucLPSTable[ rcCtxModel->getState() ][ ( m_uiRange >> 6 ) - 4 ];
  m_uiRange -= uiLPS;
  unsigned int scaledRange = m_uiRange << 7;
  
  if( m_uiValue < scaledRange )
  {
    // MPS path
    ruiBin = rcCtxModel->getMps();
#if TRACE_STATISTICS_BITRATE
    // MPS recieved. Add frac bits before updating the context
    m_fracBits += rcCtxModel.getEntropyBits( ruiBin );
#endif
    rcCtxModel->updateMPS();
    
    if ( scaledRange >= ( 256 << 7 ) )
    {
      DTRACE_CABAC_T("DecodeBinSymbol=");
      DTRACE_CABAC_V( ruiBin );
      DTRACE_CABAC_T( ", StateTrans=(" );
      DTRACE_CABAC_V(uiMpsBefore);
      DTRACE_CABAC_T( "," );
      DTRACE_CABAC_V(uiStateBefore);
      DTRACE_CABAC_T( ")->(" ); 
      DTRACE_CABAC_V(rcCtxModel->getMps());
      DTRACE_CABAC_T( "," );
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
    
    m_uiRange = scaledRange >> 6;
    m_uiValue += m_uiValue;
#if RWTH_TRACE_CABAC
    m_uiLow <<= 1;
    m_bitsLeft--;
#endif
    
    if ( ++m_bitsNeeded == 0 )
    {
      m_bitsNeeded = -8;
      m_uiValue += m_ptBitstream->readByte();      
    }
  }
  else
  {
    // LPS path
    int numBits = sm_aucRenormTable[ uiLPS >> 3 ];
    m_uiValue   = ( m_uiValue - scaledRange ) << numBits;
#if RWTH_TRACE_CABAC
    m_uiLow = (m_uiLow + m_uiRange) << numBits;
    m_bitsLeft -= numBits;
#endif
    m_uiRange   = uiLPS << numBits;
    ruiBin      = 1 - rcCtxModel->getMps();
    rcCtxModel->updateLPS();
    
    m_bitsNeeded += numBits;
        
    if ( m_bitsNeeded >= 0 )
    {
      m_uiValue += m_ptBitstream->readByte() << m_bitsNeeded;
      m_bitsNeeded -= 8;
    }
  }

    DTRACE_CABAC_T("DecodeBinSymbol=");
    DTRACE_CABAC_V(ruiBin);
    DTRACE_CABAC_T(", StateTrans=(");
    DTRACE_CABAC_V(uiMpsBefore);
    DTRACE_CABAC_T(",");
    DTRACE_CABAC_V(uiStateBefore);
    DTRACE_CABAC_T(")->(");
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
#if RWTH_TRACE_CABAC
  // Test and write out
  if (m_bitsLeft < 12)
  {
    m_bitsLeft += 8;
    m_uiLow &= 0xffffffffu >> m_bitsLeft;
  }
#endif
}

#if RWTH_CABAC_FIXED_PROBABILITY
void CABAC_ArithmeticDecoder::decodeBinProb(unsigned int& ruiBin, unsigned int uiProbability)
{
  assert(uiProbability > 0 && uiProbability < 100);
  if (uiProbability == 50)
  {
    decodeBinEP(ruiBin);
    return;
  }
  
  unsigned int uiMPS = (uiProbability > 50) ? 1 : 0;
  // The probability of the bit being the MPS where: 0 -> 50%, 49 -> 99%
  unsigned int uiProbMPS = (uiProbability < 50) ? 50 - uiProbability : uiProbability - 50;

  unsigned int  uiLPS = sm_aucLPSTProbTable[uiProbMPS-1][(m_uiRange >> 6) & 3];
  m_uiRange -= uiLPS;
  unsigned int scaledRange = m_uiRange << 7;

  if (m_uiValue < scaledRange)
  {
    // MPS path
    ruiBin = uiMPS;

    if (scaledRange >= (256 << 7))
    {
      DTRACE_CABAC_T("CABAC decodeBin fixed symbol=");
      DTRACE_CABAC_V( ruiBin );
      DTRACE_CABAC_T(" p=");
      DTRACE_CABAC_V(uiProbability);
      DTRACE_CABAC_T(" L=");
      DTRACE_CABAC_V(m_uiLow);
      DTRACE_CABAC_T(" R=");
      DTRACE_CABAC_V(m_uiRange);
      DTRACE_CABAC_T(" LPS=");
      DTRACE_CABAC_V(uiLPS);
      DTRACE_CABAC_N;
      return;
    }

    m_uiRange = scaledRange >> 6;
    m_uiValue += m_uiValue;
#if RWTH_TRACE_CABAC
    m_uiLow <<= 1;
    m_bitsLeft--;
#endif

    if (++m_bitsNeeded == 0)
    {
      m_bitsNeeded = -8;
      m_uiValue += m_ptBitstream->readByte();
    }
  }
  else
  {
    // LPS path
    int numBits = sm_aucRenormTable[uiLPS >> 3];
    m_uiValue = (m_uiValue - scaledRange) << numBits;
#if RWTH_TRACE_CABAC
    m_uiLow = (m_uiLow + m_uiRange) << numBits;
    m_bitsLeft -= numBits;
#endif
    m_uiRange = uiLPS << numBits;
    ruiBin = 1 - uiMPS;

    m_bitsNeeded += numBits;

    if (m_bitsNeeded >= 0)
    {
      m_uiValue += m_ptBitstream->readByte() << m_bitsNeeded;
      m_bitsNeeded -= 8;
    }
  }

  DTRACE_CABAC_T("CABAC decodeBin fixed symbol=");
  DTRACE_CABAC_V( ruiBin );
  DTRACE_CABAC_T(" p=");
  DTRACE_CABAC_V(uiProbability);
  DTRACE_CABAC_T(" L=");
  DTRACE_CABAC_V(m_uiLow);
  DTRACE_CABAC_T(" R=");
  DTRACE_CABAC_V(m_uiRange);
  DTRACE_CABAC_T(" LPS=");
  DTRACE_CABAC_V(uiLPS);
  DTRACE_CABAC_N;

#if RWTH_TRACE_CABAC
  // Test and write out
  if (m_bitsLeft < 12)
  {
    m_bitsLeft += 8;
    m_uiLow &= 0xffffffffu >> m_bitsLeft;
  }
#endif
}
#endif

void CABAC_ArithmeticDecoder::decodeBinEP( unsigned int& ruiBin )
{
  m_uiValue += m_uiValue;
#if TRACE_STATISTICS_BITRATE
  m_fracBits += 32768;
#endif

  if ( ++m_bitsNeeded >= 0 )
  {
    m_bitsNeeded = -8;
    m_uiValue += m_ptBitstream->readByte();
  }
  
  ruiBin = 0;
  unsigned int scaledRange = m_uiRange << 7;
  if ( m_uiValue >= scaledRange )
  {
    ruiBin = 1;
    m_uiValue -= scaledRange;
  }

  DTRACE_CABAC_T("CABAC decodeEPBin symbol=");
  DTRACE_CABAC_V( ruiBin );
  DTRACE_CABAC_T( " EP" );
#if RWTH_TRACE_CABAC
  m_uiLow <<= 1;
  if (ruiBin)
  {
    m_uiLow += m_uiRange;
  }
  m_bitsLeft--;
  DTRACE_CABAC_T(" L=");
  DTRACE_CABAC_V(m_uiLow);
  DTRACE_CABAC_T(" R=");
  DTRACE_CABAC_V(m_uiRange);
  DTRACE_CABAC_N;
  // Test and write out
  if (m_bitsLeft < 12)
  {
    m_bitsLeft += 8;
    m_uiLow &= 0xffffffffu >> m_bitsLeft;
  }
#endif
}

void CABAC_ArithmeticDecoder::decodeBinsEP( unsigned int& ruiBin, int numBins )
{
  unsigned int bins = 0;

  while ( numBins > 8 )
  {
    m_uiValue = ( m_uiValue << 8 ) + ( m_ptBitstream->readByte() << ( 8 + m_bitsNeeded ) );
    
    unsigned int scaledRange = m_uiRange << 15;
    for ( int i = 0; i < 8; i++ )
    {
      bins += bins;
      scaledRange >>= 1;
      if ( m_uiValue >= scaledRange )
      {
        bins++;
        m_uiValue -= scaledRange;
      }
    }
    numBins -= 8;

    DTRACE_CABAC_T("CABAC encode EP 8_symbol=");
    DTRACE_CABAC_VB(bins & 255, 8);
    DTRACE_CABAC_T(" EP");
#if RWTH_TRACE_CABAC    
    m_uiLow <<= 8;
    unsigned int pattern = bins & 255;
    m_uiLow += m_uiRange * pattern;
    m_bitsLeft -= 8;
    DTRACE_CABAC_T(" L=");
    DTRACE_CABAC_V(m_uiLow);
    DTRACE_CABAC_T(" R=");
    DTRACE_CABAC_V(m_uiRange);
    // Test and write out
    if (m_bitsLeft < 12)
    {
      m_bitsLeft += 8;
      m_uiLow &= 0xffffffffu >> m_bitsLeft;
    }
#endif
    DTRACE_CABAC_N;
  }
  
  m_bitsNeeded += numBins;
  m_uiValue <<= numBins;
  
  if ( m_bitsNeeded >= 0 )
  {
    m_uiValue += m_ptBitstream->readByte() << m_bitsNeeded;
    m_bitsNeeded -= 8;
  }
  
  unsigned int scaledRange = m_uiRange << ( numBins + 7 );
  for ( int i = 0; i < numBins; i++ )
  {
    bins += bins;
    scaledRange >>= 1;
    if ( m_uiValue >= scaledRange )
    {
      bins++;
      m_uiValue -= scaledRange;
    }
  }

  DTRACE_CABAC_T("CABAC encode EP ");
  DTRACE_CABAC_V(numBins);
  DTRACE_CABAC_T("_symbol=");
  DTRACE_CABAC_VB(bins & ((1<<numBins)-1), numBins);
  DTRACE_CABAC_T(" EP");
#if RWTH_TRACE_CABAC
  m_uiLow <<= numBins;
  unsigned int pattern = bins & ((1 << numBins) - 1);
  m_uiLow += m_uiRange * pattern;
  m_bitsLeft -= numBins;
  DTRACE_CABAC_T(" L=");
  DTRACE_CABAC_V(m_uiLow);
  DTRACE_CABAC_T(" R=");
  DTRACE_CABAC_V(m_uiRange);
  // Test and write out
  if (m_bitsLeft < 12)
  {
    m_bitsLeft += 8;
    m_uiLow &= 0xffffffffu >> m_bitsLeft;
  }
#endif
  DTRACE_CABAC_N;

  ruiBin = bins;
}

void CABAC_ArithmeticDecoder::decodeBinTrm( unsigned int& ruiBin )
{
  m_uiRange -= 2;
  unsigned int scaledRange = m_uiRange << 7;
  if( m_uiValue >= scaledRange )
  {
    ruiBin = 1;
#if RWTH_TRACE_CABAC
    m_uiLow += m_uiRange;
    m_uiLow <<= 7;
    m_bitsLeft -= 7;
    m_uiRange = 2 << 7;
#endif
  }
  else
  {
    ruiBin = 0;
    if ( scaledRange < ( 256 << 7 ) )
    {
      m_uiRange = scaledRange >> 6;
      m_uiValue += m_uiValue;
      
      if ( ++m_bitsNeeded == 0 )
      {
        m_bitsNeeded = -8;
        m_uiValue += m_ptBitstream->readByte();      
      }
#if RWTH_TRACE_CABAC
      m_uiLow <<= 1;
      m_bitsLeft--;
#endif
    }
  }
  
  DTRACE_CABAC_T("CABAC decode Terminating Bin symbol=");
  DTRACE_CABAC_V( ruiBin );
  DTRACE_CABAC_T(" L=");
  DTRACE_CABAC_V(m_uiLow);
  DTRACE_CABAC_T(" R=");
  DTRACE_CABAC_V(m_uiRange);
  DTRACE_CABAC_N;
#if RWTH_TRACE_CABAC
  // Test and write out
  if (m_bitsLeft < 12)
  {
    m_bitsLeft += 8;
    m_uiLow &= 0xffffffffu >> m_bitsLeft;
  }
#endif
}

const unsigned char CABAC_ArithmeticDecoder::sm_aucLPSTable[64][4] =
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

const unsigned char CABAC_ArithmeticDecoder::sm_aucRenormTable[32] =
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
const unsigned char CABAC_ArithmeticDecoder::sm_aucLPSTProbTable[49][4] =
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