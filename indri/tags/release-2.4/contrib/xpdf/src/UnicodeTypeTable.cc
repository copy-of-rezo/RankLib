//========================================================================
//
// UnicodeMapTables.cc
//
// Copyright 2003 Glyph & Cog, LLC
//
//========================================================================

#include <stdlib.h>
#include "CharTypes.h"
#include "UnicodeTypeTable.h"

struct UnicodeMapTableEntry {
  char *vector;
  char type;
};

static UnicodeMapTableEntry table[256] = {
  { "NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLNNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNLNNNNNNNNNNLNNNNLNNNNNLLLLLLLLLLLLLLLLLLLLLLLNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNLLLLLLLL", 'X' },
  { NULL, 'L' },
  { "LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNLLLLLLLNNNNNNNNNNNNNNLLNNNNNNNNNNNNNNLLLLLNNNNNNNNNLNNNNNNNNNNNNNNNNN", 'X' },
  { "NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNLLLLLNNNNNNNNNNNLNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL", 'X' },
  { "LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL", 'X' },
  { "LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNRNRNNRNRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRNNNNNNNNNNN", 'X' },
  { "NNNNNNNNNNNNNRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNRNNNRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRNNNNNNNNNNNNNNNRRNNNNNNNNNNNNNNNNNNNRRRRRR", 'X' },
  { "RRRRRRRRRRRRRRNNRNRRRRRRRRRRRRRRRRRRRRRRRRRRRNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN", 'X' },
  { NULL, 'N' },
  { "NNNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNNLLLLNNNNNNNNLLLLNLLLNNNNLLLLLLLLLLLLLNNLLLLLLLLLLLLLNNNNNNNNNNNNNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNNLLLLNNNNLLLLLLLLNLLLLLLLLLLLLLLLLLLLLNNLLLLLLLLLLLLLLNNLLLLLLLNNNNN", 'X' },
  { "NNNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNNLLLLNNNNNNNNNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNLLLNNNNNNNNNNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNNLLLLNNNNNNNNLLLLNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNNNNNNNNNNNNNNN", 'X' },
  { "NNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNNLLNLNNNLLLLLLLLLNNNNNNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLNNNNNNNNNNNNNNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNLLLLLLLLLLLLNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL", 'X' },
  { "LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNNNNNNLLLLNNNNNNNNNNNNNNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNLLLLLNNLLLLLNNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL", 'X' },
  { "LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNNLLLLLLLLLNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNNNLLLLLLLNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL", 'X' },
  { "LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNLLNNNNNNNNNNNNLLLLLLLNNNNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNLLNNNNNNNNNLLLLLLLLLLNNNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL", 'X' },
  { "LLLLLLLLLLLLLLLLLLLLLLLLNNLLLLLLLLLLLLLLLLLLLLLLLLLLLNLNLNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNNNNNNNNNNNNNNNNNNNLNNNNNLNNLLLLNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNLLLLLLLLLNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL", 'X' },
  { "LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNNNLNNNNNNLNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL", 'X' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { "LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNNNNNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLNNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL", 'X' },
  { "LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNNNNNNLLLLLLLLNLLNNNNNNNNNNNLLLLLLLNLLLLLLLLLLLLLLNNNNNNNNNNNNNNNNNNNNNN", 'X' },
  { "NNNNNNNNNNNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL", 'X' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { "LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNLNNNLLLLLLLLLLLNNNLLLLLLLLLLLLNNNNLLLLLLLLLLLLLNNNLLLLLLLLLLLLLNNN", 'X' },
  { "NNNNNNNNNNNNNNLRNNNNNNNNNNNNNNNNNNNNNNNNNNLRNLRNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNLNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN", 'X' },
  { "NNLNNNNLNNLLLLLLLLLLNLNNNLLLLLNNNNNNLNLNLNLLLLNLLLNLLLLLLLNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN", 'X' },
  { NULL, 'N' },
  { "NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNNNNNNNNNNNNNNNNNNNNNNNNNLNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN", 'X' },
  { "NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNNNNNNNNNNNNNNNNNNNNN", 'X' },
  { NULL, 'N' },
  { NULL, 'N' },
  { NULL, 'N' },
  { NULL, 'N' },
  { NULL, 'N' },
  { NULL, 'N' },
  { NULL, 'N' },
  { NULL, 'N' },
  { NULL, 'N' },
  { NULL, 'N' },
  { NULL, 'N' },
  { "NNNNNLLLNNNNNNNNNNNNNNNNNNNNNNNNNLLLLLLLLLNNNNNNNLLLLLNNLLLNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNNNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNLLLL", 'X' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { "LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL", 'X' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { NULL, 'L' },
  { "LLLLLLLLLLLLLLLLLLLLLLLLRRRRRRNRRRRRRRRRRNRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR", 'X' },
  { NULL, 'R' },
  { "RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRNNRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRNNNN", 'X' },
  { "NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRNNN", 'X' },
  { "NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLNNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLNNNNNNNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNLL", 'X' }
};

static inline char getType(Unicode c) {
  int i;
  char type;

  if (c > 0xffff) {
    type = 'X';
  } else {
    i = (c >> 8) & 0xff;
    if ((type = table[i].type) == 'X') {
      type = table[i].vector[c & 0xff];
    }
  }
  return type;
}

GBool unicodeTypeL(Unicode c) {
  return getType(c) == 'L';
}

GBool unicodeTypeR(Unicode c) {
  return getType(c) == 'R';
}

