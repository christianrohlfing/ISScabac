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
 
#include "CABAC_BitstreamFile.h"
#include <assert.h>

CABAC_BitstreamFile::CABAC_BitstreamFile()
{
  m_num_held_bits = 0;
  m_held_bits = 0;
  m_num_bits_written = 0;
  bFileOpened = false;
  bInputFile = false;
}

CABAC_BitstreamFile::~CABAC_BitstreamFile()
{
}

bool CABAC_BitstreamFile::openOutputFile(const char *cOtuputFileName)
{
  assert(!bFileOpened);
  // Open output file for writing
  bitstreamFile.open(cOtuputFileName, fstream::binary | fstream::out);
  if (!bitstreamFile)
  {
    return false;
  }
  else
  {
    bFileOpened = true;
    bInputFile = false;
    return true;
  }
}

bool CABAC_BitstreamFile::openInputFile(const char *cInputFileName)
{
  bitstreamFile.open(cInputFileName, ifstream::in | ifstream::binary);
  if (!bitstreamFile)
  {
    return false;
  }
  else
  {
    bFileOpened = true;
    bInputFile = true;
    return true;
  }
}

void CABAC_BitstreamFile::closeFile()
{
  bitstreamFile.close();
  bFileOpened = false;
}

void CABAC_BitstreamFile::writeByteAlignment()
{
  write( 1, 1);
  writeAlignZero();
}

  // ################## Arithmetic encoding ######################
void CABAC_BitstreamFile::write   ( unsigned int uiBits, unsigned int uiNumberOfBits )
{
  assert(bFileOpened && !bInputFile);
  assert( uiNumberOfBits <= 32 );
  assert( uiNumberOfBits == 32 || (uiBits & (~0 << uiNumberOfBits)) == 0 );

  /* any modulo 8 remainder of num_total_bits cannot be written this time,
   * and will be held until next time. */
  unsigned int num_total_bits = uiNumberOfBits + m_num_held_bits;
  unsigned int next_num_held_bits = num_total_bits % 8;

  /* form a byte aligned word (write_bits), by concatenating any held bits
   * with the new bits, discarding the bits that will form the next_held_bits.
   * eg: H = held bits, V = n new bits        /---- next_held_bits
   * len(H)=7, len(V)=1: ... ---- HHHH HHHV . 0000 0000, next_num_held_bits=0
   * len(H)=7, len(V)=2: ... ---- HHHH HHHV . V000 0000, next_num_held_bits=1
   * if total_bits < 8, the value of v_ is not used */
  unsigned char next_held_bits = uiBits << (8 - next_num_held_bits);

  if (!(num_total_bits >> 3))
  {
    /* insufficient bits accumulated to write out, append new_held_bits to
     * current held_bits */
    /* NB, this requires that v only contains 0 in bit positions {31..n} */
    m_held_bits |= next_held_bits;
    m_num_held_bits = next_num_held_bits;
    return;
  }

  /* topword serves to justify held_bits to align with the msb of uiBits */
  unsigned int topword = (uiNumberOfBits - next_num_held_bits) & ~((1 << 3) -1);
  unsigned int write_bits = (m_held_bits << topword) | (uiBits >> next_num_held_bits);

  switch (num_total_bits >> 3)
  {
    case 4: bitstreamFile << (char)(write_bits >> 24); m_num_bits_written += 8;
    case 3: bitstreamFile << (char)(write_bits >> 16); m_num_bits_written += 8;
    case 2: bitstreamFile << (char)(write_bits >> 8); m_num_bits_written += 8;
    case 1: bitstreamFile << (char)(write_bits); m_num_bits_written += 8;
  }

  m_held_bits = next_held_bits;
  m_num_held_bits = next_num_held_bits;
}

void CABAC_BitstreamFile::writeAlignZero()
{
  assert(bFileOpened && !bInputFile);
  if (0 == m_num_held_bits)
  {
    return;
  }
  bitstreamFile << (char)(m_held_bits);
  m_held_bits = 0;
  m_num_held_bits = 0;
  m_num_bits_written += 8;
}

unsigned int CABAC_BitstreamFile::readByte()
{
  unsigned char cRead = bitstreamFile.get();
  m_cLastCharRead = cRead;
  return cRead;
}