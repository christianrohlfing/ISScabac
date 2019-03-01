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
 
#pragma once

#include <fstream>
using namespace std;

/** The CABAC bitstream file class
  *
  * The arithmetic coder can use this class to write out ones and zeroes to a file.
  * You can write a different data sink for the arithmetic coder. The required functions 
  * for the arithmetic coder are:
  *  - write(bits, nrBits)
  *  - writeAlignZero()
  *
  * Usage: Create an instance and open the output file. Give the instance to the arithmetic 
  * coder instance and start coding.
  */
class CABAC_BitstreamFile
{
public:
  CABAC_BitstreamFile();
  ~CABAC_BitstreamFile();

  bool openOutputFile(const char *cOtuputFileName);
  bool openInputFile(const char *cInputFileName);
  void closeFile();

  // append uiNumberOfBits least significant bits of uiBits to the current bitstream
  void  write           ( unsigned int uiBits, unsigned int uiNumberOfBits );
  void  writeAlignZero  ();     ///< insert zero bits until the bitstream is byte-aligned
  void  writeByteAlignment();
  
  // read from the stream
  unsigned int readByte();
  unsigned int getNumBitsUntilByteAligned() { return m_num_held_bits & (0x7); }
  
  // Return the number of bits that have been written since the last resetWrittenBits()
  unsigned int getNumberOfWrittenBits() const { return m_num_bits_written + m_num_held_bits; }
  // Reset the bit counter to 0
  void resetWrittenBits() { m_num_bits_written = 0; }
  unsigned int getLastByteRead() { return m_cLastCharRead; }

protected:
  // The bitstream
  fstream bitstreamFile;
  bool bFileOpened;
  bool bInputFile;
  
  unsigned int  m_num_held_bits; /// number of bits not flushed to bytestream.
  unsigned char m_held_bits; /// the bits held and not flushed to bytestream.
                             /// this value is always msb-aligned, bigendian.
  unsigned int  m_num_bits_written;
  char          m_cLastCharRead;
};

