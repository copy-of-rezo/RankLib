/*==========================================================================
 * Copyright (c) 2003 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.cs.cmu.edu/~lemur/license.html
 *
 *==========================================================================
*/


//
// WriteBuffer
//
// tds - 13 November 2003
//

#ifndef LEMUR_WRITEBUFFER_HPP
#define LEMUR_WRITEBUFFER_HPP

#include "File.hpp"

class WriteBuffer {
private:
  char* _buffer;
  size_t _bufferSize;
  File& _file;
  size_t _position;
  File::offset_type _filePos;

public:
  WriteBuffer( File& file, size_t bufferSize );
  ~WriteBuffer();

  // gives a memory pointer to the next <length>
  // bytes in the file.
  char* write( size_t length );

  // tells the WriteBuffer that <length> bytes
  // of the last write(size_t) call were not used
  // and should be returned as a part of the
  // next write(size_t) call.  This function cannot
  // be used in conjunction with the write(char*, size_t)
  // call.
  void unwrite( size_t length );
  
  // standard write semantics; will
  // perform an unbuffered write if
  // <length> is long enough to warrant it
  void write( const char* data, size_t length );

  // flushes the internal buffer out to 
  // the ofstream. does not explicitly
  // flush the ofstream.
  void flush();

  // returns the current write pointer
  // position
  File::offset_type tellp() const;
};

#endif // LEMUR_WRITEBUFFER_HPP
