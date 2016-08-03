#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

typedef unsigned char  uint8;
typedef unsigned short uint16;

enum { 
  X_COORD_BITS   = 2,
  Y_COORD_BITS   = 3,
  OP_BITS        = 2,
  ARG_BITS       = 5,
};

enum {
  LINE_DX_BITS   = 2,
  LINE_DY_BITS   = 3,
  ARC_A0_BITS    = 2,
  ARC_A1_BITS    = 3,
};

enum {
  SEP_0 = '"',
  SEP_1 = SEP_0 + 5,
  SEP_2 = SEP_1+1,
};

#define MASK(b) ((1<<b)-1)

int NEGATE_X = 1;
int DO_UPPERCASE = 0;
int DO_PUNCTUATION = 1;

int OP_LINE=2;
int OP_ARC=3;
int OP_NLINE=1;

int OP_SHIFT=5;
int ARG_SHIFT=0;
int X_COORD_SHIFT=11;
int Y_COORD_SHIFT=8;

int LINE_DX_SHIFT=3;
int LINE_DY_SHIFT=0;

int ARC_A0_SHIFT=3;
int ARC_A1_SHIFT=0;

uint16 PADDING_VAL=0x4000;

uint16 X_COORD = 0;
uint16 Y_COORD = 0;
uint16 dxmin(-1), dxmax(0);
char dxminc=' ', dxmaxc=' ';
char cur=0;;

unsigned char bmin(-1), bmax(0);

typedef std::vector<uint8> buffer;
buffer buf;

struct shift_info {
  const char* which;
  int* shift;
  int bits;
  shift_info();
  shift_info(const char* s, int* i, int b): which(s), shift(i), bits(b) {}
};

std::vector<shift_info> shifts;

int rand_int(int n) {
  return rand() % n;
}

void permute_shifts() {

  NEGATE_X = rand()%2;

  OP_LINE = rand()%4;
  do { OP_ARC = rand()%4; } while (OP_ARC == OP_LINE); 
  do { OP_NLINE = rand()%4; } while (OP_NLINE == OP_ARC || OP_NLINE == OP_LINE);

  size_t all_bits = OP_BITS + ARG_BITS + X_COORD_BITS + Y_COORD_BITS;
  assert( all_bits <= 16 );

  size_t padding_bits = 16 - all_bits;

  shifts.clear();
  shifts.push_back(shift_info("OP", &OP_SHIFT, OP_BITS));
  shifts.push_back(shift_info("X_COORD", &X_COORD_SHIFT, X_COORD_BITS));
  shifts.push_back(shift_info("Y_COORD", &Y_COORD_SHIFT, Y_COORD_BITS));
  shifts.push_back(shift_info("ARG", &ARG_SHIFT, ARG_BITS));

  for (size_t i=0; i<padding_bits; ++i) {
    shifts.push_back(shift_info("[padding]", 0, 1));
  }

  std::random_shuffle(shifts.begin(), shifts.end(), rand_int);

  size_t total_bits = 0;

  PADDING_VAL = 0;

  for (size_t i=0; i<shifts.size(); ++i) {
    shift_info info = shifts[i];
    if (info.shift) { 
      *info.shift = total_bits; 
    } else {
      if (rand() % 2) { PADDING_VAL = PADDING_VAL | (1 << total_bits); }
    }
    total_bits += info.bits;
  }

  assert( total_bits == 16 );

  if (rand()%2) {
    LINE_DY_SHIFT = 0;
    LINE_DX_SHIFT = LINE_DY_BITS;
  } else {
    LINE_DY_SHIFT = LINE_DX_BITS;
    LINE_DX_SHIFT = 0;
  }

  if (rand()%2) {
    ARC_A0_SHIFT = 0;
    ARC_A1_SHIFT = ARC_A0_BITS;
  } else {
    ARC_A0_SHIFT = ARC_A1_BITS;
    ARC_A1_SHIFT = 0;
  }


}


void instr(uint16 type, 
            uint16 arg,
            uint16 x, 
            uint16 y) {

  assert(x >= X_COORD);
  assert(y >= Y_COORD || x > X_COORD);
  uint16 dx = x - X_COORD;
  X_COORD = x;
  x = dx;

  if (dx < dxmin) { dxmin = dx; dxminc = cur; }
  if (dx > dxmax) { dxmax = dx; dxmaxc = cur; }
  
  assert(type <= MASK(OP_BITS));
  assert(arg <= MASK(ARG_BITS));
  assert(x <= MASK(X_COORD_BITS));
  assert(y <= MASK(Y_COORD_BITS));
  
  uint16 val = ( (type << OP_SHIFT) | (arg << ARG_SHIFT) |
                 (x << X_COORD_SHIFT) | (y << Y_COORD_SHIFT) | 
                 PADDING_VAL );

  unsigned char c0 = val&0xff;
  unsigned char c1 = (val>>8)&0xff;

  bmin = std::min(bmin, std::min(c0,c1));
  bmax = std::max(bmax, std::max(c0,c1));
  
  buf.push_back(c0);
  buf.push_back(c1);
  
}


void start(char c, uint16 w) {

  Y_COORD = X_COORD = 0;


  buf.push_back(SEP_0+w);

  if (c != cur+1) {
    buf.push_back(SEP_2);
    buf.push_back(c);
  } 

  cur = c;

}

void line(uint16 x, uint16 y, int dx, int dy) {

  int op = OP_LINE;

  if (dx < 0) {
    assert(NEGATE_X);
    dx = -dx;
    op = OP_NLINE;
  }

  if (dy < 0) { 
    assert(!NEGATE_X);
    dy = -dy;
    op = OP_NLINE;
  }

  assert(y <= MASK(Y_COORD_BITS));
  assert(dx <= MASK(LINE_DX_BITS));
  assert(dy <= MASK(LINE_DY_BITS));

  uint16 arg = (dx << LINE_DX_SHIFT) | (dy << LINE_DY_SHIFT);

  instr(op, arg, x, y);

}

void arc(uint16 x, 
         uint16 y, 
         uint16 a0,
         uint16 a1) {

  assert(y <= MASK(Y_COORD_BITS));

  assert(a0 <= MASK(ARC_A0_BITS));
  assert(a1 <= MASK(ARC_A1_BITS));

  uint16 arg = (a0 << ARC_A0_SHIFT) | (a1 << ARC_A1_SHIFT);

  instr(OP_ARC, arg, x, y);

}


std::string stringify() {

  const unsigned char* ucbuf = (const unsigned char*)(&buf[0]);

  std::ostringstream ostr;
  bool wasHex = false;

  for (size_t i=0; i<buf.size(); ++i) {
    unsigned char b = ucbuf[i];
    if (b == '"') {
      // quote
      ostr << "\\\"";
      wasHex = false;
    } else if (b >= 32 && b <= 126) {
      if (wasHex && isxdigit(b)) {
        // interrupt because it was hex, and we have a hex digit here
        ostr << "\"\"";
      } 
      if (b == '\\') {
        ostr << "\\\\";
      } else {
        ostr << char(b);
      }
      wasHex = false;
    } else {
      wasHex=false;
      switch (b) {
      case '\n': ostr << "\\n"; break;
      case '\t': ostr << "\\t"; break;
      case '\v': ostr << "\\v"; break;
      case '\b': ostr << "\\b"; break;
      case '\r': ostr << "\\r"; break;
      case '\a': ostr << "\\a"; break;
      case '\f': ostr << "\\f"; break;
      case '\0': ostr << "\\0"; break;
      default:
        ostr << "\\x" << std::hex << int(b);
        wasHex = true;
        break;
      }
    } 
  }

  return ostr.str();

}


std::string stringvar(const std::string& var, int shift, int bits, int total=8) {
  
  std::ostringstream ostr;
  
  ostr << var;

  bool needshift = shift;
  bool needmask = (bits + shift) < total;

  if (needshift) {
    if ((1 << shift) >= 100) {
      ostr << "<<" << shift;
    } else {
      ostr << "/" << (1 << shift);
    }
  }

  if (needmask) {
    ostr << "&" << ((1 << bits) - 1);
  }
  
  return ostr.str();

}

std::string stringvar(int shift, int bits, int total=8) {


  if (shift>=8) {
    return stringvar("b[1]", shift-8, bits, total);
  } else {
    return stringvar("b[0]", shift, bits, total);
  }

}


std::string stringvars() {

  std::ostringstream ostr;

  ostr << "int o=" << stringvar(OP_SHIFT, OP_BITS) << ", a=" << stringvar(ARG_SHIFT, ARG_BITS) << ";\n";
  ostr << "x+=" << stringvar(X_COORD_SHIFT, X_COORD_BITS) << "; y=" << stringvar(Y_COORD_SHIFT, Y_COORD_BITS) << ";\n";
  ostr << "f i=" << stringvar("a", LINE_DX_SHIFT, LINE_DX_BITS, ARG_BITS) << ", j=" << stringvar("a", LINE_DY_SHIFT, LINE_DY_BITS, ARG_BITS) << ";\n";
  ostr << "f l=-P*(" << stringvar("a", ARC_A0_SHIFT, ARC_A0_BITS, ARG_BITS) << "), "
       << "u=P*(" << stringvar("a", ARC_A1_SHIFT, ARC_A1_BITS, ARG_BITS) << ");\n";

  return ostr.str();

}


enum {
  OP_HLINE = 1,
  OP_VLINE = 2,
  OP_DLINE = 3,
  ARC_R0 = 0,
  ARC_R1 = 1,
  ARC_UPPER = 1,
  ARC_LOWER = 2,
  ARC_FULL  = 3,
};


void encode() {

  buf.clear();

  bmin = -1;
  bmax = 0;
  dxmin = -1;
  dxmax = 0;
  dxminc = dxmaxc = cur = 0;

  start(' ', 1);

  start('!', 1);
  line(0, 0, 0, 0);
  line(0, 1, 0, 4);

  if (DO_PUNCTUATION) {
    
    start('"', 2);
    line(0,4,0,1);
    line(1,4,0,1);
    
    start('#', 4); 
    line(0, 1, 3, 0);
    line(0, 4, 3, 0);
    line(1, 0, 0, 5);
    line(2, 0, 0, 5);
    
    start('$', 3);
    if (!NEGATE_X) {
      line(0, 3, 2, -1); 
    }
    line(1, 0, 0, 5);
    arc(1, 2, 0, 1); 
    arc(1, 3, 2, 1); 
    if (NEGATE_X) {
      line(2, 2, -2, 1); 
    }

    start('%', 4);
    line(0, 2, 3, 1);
    arc(1, 4, 0, 3);
    arc(2, 1, 0, 3);

    start('&', 5);
    line(0, 1, 0, 1);
    arc(1, 1, 0, 1); 
    arc(1, 2, 3, 0);
    line(1, 3, 2, 0);
    arc(1, 4, 0, 3); 
    line(2, 1, 0, 2);
    arc(3, 1, 0, 1);
  
    start('(', 2);
    line(0, 1, 0, 3);
    arc(1, 1, 0, 0);
    arc(1, 4, 3, 0);

    start(')', 2);
    arc(0, 1, 1, 0);
    arc(0, 4, 2, 0);
    line(1, 1, 0, 3);

    start('*', 3);
    line(0, 2, 2, 2);
    if (!NEGATE_X) {
      line(0, 4, 2, -2);
    }
    line(1, 1, 0, 4);
    if (NEGATE_X) {
      line(2, 2, -2, 2);
    }

    start('+', 3);
    line(0, 3, 2, 0);
    line(1, 2, 0, 2);
  

    start(',', 2);
    line(0, 0, 1, 1);

    start('-', 3);
    line(0, 3, 2, 0);
  
  }

  start('.', 1);
  line(0, 0, 0, 0);

  if (DO_PUNCTUATION) {
    start('/', 3);
    line(0, 0, 2, 5);
  }

  start('0', 3);
  line(0, 1, 0, 3);
  arc(1, 1, 0, 1); 
  arc(1, 4, 2, 1); 
  line(2, 1, 0, 3);

  start('1', 2);
  line(0, 4, 1, 1);
  line(1, 0, 0, 5);

  start('2', 3);
  line(0, 0, 2, 0);
  line(0, 0, 2, 4);
  arc(1, 4, 2, 1); 

  start('3', 3);
  line(0, 5, 2, 0);
  arc(1, 1, 0, 1); 
  arc(1, 2, 2, 0); 
  line(1, 3, 1, 2);
  line(2, 1, 0, 1);

  start('4', 3);
  line(0, 2, 1, 3);
  line(0, 2, 2, 0);
  line(2, 0, 0, 5);

  start('5', 3);
  line(0, 2, 0, 3);
  line(0, 5, 2, 0);
  arc(1, 1, 0, 1); 
  arc(1, 2, 2, 1); 
  line(2, 1, 0, 1);

  start('6', 3);
  line(0, 1, 0, 3);
  arc(1, 1, 0, 1); 
  arc(1, 2, 2, 1); 
  arc(1, 4, 2, 1); 
  line(2, 1, 0, 1);

  start('7', 3);
  line(0, 0, 2, 5);
  line(0, 5, 2, 0);

  start('8', 3);
  line(0, 1, 0, 1);
  arc(1, 1, 0, 1); 
  arc(1, 2, 2, 1); 
  arc(1, 4, 0, 3); 
  line(2, 1, 0, 1);

  start('9', 3);
  line(0, 3, 0, 1);
  arc(1, 1, 0, 1); 
  arc(1, 3, 0, 1); 
  arc(1, 4, 2, 1); 
  line(2, 1, 0, 3);

  if (DO_PUNCTUATION) {
    start('@', 4);
    line(0, 1, 0, 3);
    line(1, 0, 1, 0);
    line(1, 5, 1, 0);
    arc(1, 1, 0, 0);
    arc(1, 4, 3, 0);
    arc(2, 4, 2, 0);
    arc(2, 2, 0, 3);
    line(3, 1, 0, 3);
  }

  if (DO_UPPERCASE) {

    start('A', 3);
    line(0, 0, 0, 4);
    line(0, 2, 2, 0);
    arc(1, 4, 2, 1);
    line(2, 0, 0, 4);

    start('B', 3);
    line(0, 0, 0, 5);
    line(0, 0, 1, 0);
    line(0, 3, 1, 0);
    line(0, 5, 1, 0);
    arc(1, 1, 1, 0);
    arc(1, 2, 2, 0);
    arc(1, 4, 1, 1);
    line(2, 1, 0, 1);

    start('C', 3);
    line(0, 1, 0, 3);
    arc(1, 1, 0, 1);
    arc(1, 4, 2, 1);

    start('D', 3);
    line(0, 0, 0, 5);
    line(0, 0, 1, 0);
    line(0, 5, 1, 0);
    arc(1, 1, 1, 0);
    arc(1, 4, 2, 0);
    line(2, 1, 0, 3);

    start('E', 3);
    line(0, 0, 0, 5);
    line(0, 0, 2, 0);
    line(0, 3, 1, 0);
    line(0, 5, 2, 0);

    start('F', 3);
    line(0, 0, 0, 5);
    line(0, 3, 1, 0);
    line(0, 5, 2, 0);

    start('G', 3);
    line(0, 1, 0, 3);
    arc(1, 1, 0, 1);
    line(1, 2, 1, 0);
    arc(1, 4, 2, 1);
    line(2, 0, 0, 2);

    start('H', 3);
    line(0, 0, 0, 5);
    line(0, 3, 2, 0);
    line(2, 0, 0, 5);

    start('I', 3);
    line(0, 0, 2, 0);
    line(0, 5, 2, 0);
    line(1, 0, 0, 5);

    start('J', 3);
    arc(1, 1, 0, 1);
    line(2, 1, 0, 4);

    start('K', 3);
    line(0, 0, 0, 5);
    line(0, 3, 2, 2);

  if (!NEGATE_X) {
      line(0, 3, 2, -3);
    }
    if (NEGATE_X) {
      line(2, 0, -2, 3); // was 2, 5
    }

    start('L', 3);
    line(0, 0, 0, 5);
    line(0, 0, 2, 0);

    start('M', 5);
    line(0, 0, 0, 5);

  if (NEGATE_X) {
      line(2, 2, -2, 3); // was 2, 0
    } else {
      line(0, 5, 2, -3);
    }

    line(2, 2, 2, 3);
    line(4, 0, 0, 5);

    start('N', 3);
    line(0, 0, 0, 5);
    if (NEGATE_X) {
      line(2, 0, -2, 5);
    } else {
      line(0, 5, 2, -5);
    } 
    line(2, 0, 0, 5);

    start('O', 3);
    line(0, 1, 0, 3);
    arc(1, 1, 0, 1); 
    arc(1, 4, 2, 1); 
    line(2, 1, 0, 3);

    start('P', 3);
    line(0, 0, 0, 5);
    line(0, 2, 1, 0);
    line(0, 5, 1, 0);
    arc(1, 3, 1, 0);
    arc(1, 4, 2, 0);
    line(2, 3, 0, 1);

    start('Q', 3);
    line(0, 1, 0, 3);
    arc(1, 1, 0, 1); 
    if (!NEGATE_X) {
      line(1, 1, 1, -1);
    }
    arc(1, 4, 2, 1); 
    if (NEGATE_X) {
      line(2, 0, -1, 1);
    }
    line(2, 1, 0, 3);

    start('R', 3);
    line(0, 0, 0, 5);
    line(0, 2, 1, 0);
    line(0, 5, 1, 0);
    if (!NEGATE_X) {
      line(1, 2, 1, -2);
    }
    arc(1, 3, 1, 0);
    arc(1, 4, 2, 0);
    if (NEGATE_X) {
      line(2, 0, -1, 2);
    }
    line(2, 3, 0, 1);

    start('S', 3);
    if (!NEGATE_X) {
      line(0, 4, 2, -3);
    }
    arc(1, 1, 0, 1);
    arc(1, 4, 2, 1);
    if (NEGATE_X) {
      line(2, 1, -2, 3);
    }

    start('T', 3);
    line(0, 5, 2, 0);
    line(1, 0, 0, 5);

    start('U', 3);
    line(0, 1, 0, 4);
    arc(1, 1, 0, 1);
    line(2, 1, 0, 4);

    start('V', 3);
    if (!NEGATE_X) {
      line(0, 5, 1, -5);
    }
    if (NEGATE_X) {
      line(1, 0, -1, 5);
    }
    line(1, 0, 1, 5);

    start('W', 5);
    if (!NEGATE_X) {
      line(0, 5, 1, -5);
    }
    if (NEGATE_X) {
      line(1, 0, -1, 5);
    }
    line(1, 0, 1, 3);
    if (!NEGATE_X) {
      line(2, 3, 1, -3);
    } 
    if (NEGATE_X) {
      line(3, 0, -1, 3);
    }
    line(3, 0, 1, 5);

    start('X', 3);
    line(0, 0, 2, 5);
    if (!NEGATE_X) {
      line(0, 5, 2, -5);
    } else {
      line(2, 0, -2, 5);
    }

    start('Y', 3);
    if (!NEGATE_X) {
      line(0, 5, 1, -3);
    } 
    line(1, 0, 0, 2);
    if (NEGATE_X) {
      line(1, 2, -1, 3);
    } 
    line(1, 2, 1, 3);
    
    start('Z', 3);
    line(0, 0, 2, 0);
    line(0, 0, 2, 5);
    line(0, 5, 2, 0);
  }

  start('a', 3);
  line(0, 3, 1, 0); 
  arc(1, 1, 0, 3); 
  arc(1, 2, 2, 0); 
  line(2, 0, 0, 2);   

  start('b', 3);
  line(0, 0, 0, 5); 
  arc(1, 1, 0, 1); 
  arc(1, 2, 2, 1); 
  line(2, 1, 0, 1); 

  start('c', 3);
  line(0, 1, 0, 1); 
  arc(1, 1, 0, 1); 
  arc(1, 2, 2, 1); 

  start('d', 3);
  line(0, 1, 0, 1); 
  arc(1, 1, 0, 1); 
  arc(1, 2, 2, 1); 
  line(2, 0, 0, 5); 
    
  start('e', 3);
  line(0, 1, 0, 1); 
  line(0, 2, 2, 0); 
  arc(1, 1, 0, 1); 
  arc(1, 2, 2, 1); 

  start('f', 3);
  line(0, 3, 2, 0); 
  line(1, 0, 0, 4); 
  arc(2, 4, 3, 0); 
  
  start('g', 3);
  line(0, 0, 1, 0); 
  arc(1, 1, 1, 0); 
  arc(1, 2, 0, 3); 
  line(2, 1, 0, 2); 

  start('h', 3);
  line(0, 0, 0, 5); 
  arc(1, 2, 2, 1); 
  line(2, 0, 0, 2); 

  start('i', 1);
  line(0, 0, 0, 3);
  line(0, 4, 0, 0); 

  start('j', 3);
  arc(1, 1, 0, 1); 
  line(2, 1, 0, 2);
  line(2, 4, 0, 0); 

  start('k', 3);
  line(0, 0, 0, 5);
  line(0, 2, 2, 1);
  if (NEGATE_X) { 
    line(2, 0, -2, 2); 
  }
  if (!NEGATE_X) {
    line(0, 2, 2, -2); 
  }

  start('l', 1);
  line(0, 0, 0, 5);

  start('m', 5);
  line(0, 0, 0, 3); 
  arc(1, 2, 2, 1); 
  line(2, 0, 0, 2); 
  arc(3, 2, 2, 1); 
  line(4, 0, 0, 2); 

  start('n', 3);
  line(0, 0, 0, 3); 
  arc(1, 2, 2, 1); 
  line(2, 0, 0, 2); 

  start('o', 3);
  line(0, 1, 0, 1); 
  arc(1, 1, 0, 1); 
  arc(1, 2, 2, 1); 
  line(2, 1, 0, 1); 

  start('p', 3);
  line(0, 0, 0, 3);
  arc(1, 2, 0, 3); 

  start('q', 3); 
  arc(1, 2, 0, 3); 
  line(2, 0, 0, 3);

  start('r', 3);
  line(0, 0, 0, 3); 
  arc(1, 2, 2, 1); 

  start('s', 3);
  if (!NEGATE_X) {
    line(0, 2, 2, -1); 
  }
  arc(1, 1, 0, 1); 
  arc(1, 2, 2, 1); 
  if (NEGATE_X) {
    line(2, 1, -2, 1); 
  }

  start('t', 3);
  line(0, 3, 2, 0); 
  line(1, 1, 0, 4); 
  arc(2, 1, 0, 0); 

  start('u', 3);
  line(0, 1, 0, 2); 
  arc(1, 1, 0, 1); 
  line(2, 0, 0, 3); 

  start('v', 3);
  if (!NEGATE_X) {
    line(0, 3, 1, -3); 
  }
  line(1, 0, 1, 3);
  if (NEGATE_X) {
    line(1, 0, -1, 3); 
  }

  start('w', 5);
  if (!NEGATE_X) {
    line(0, 3, 1, -3);    
  }
  line(1, 0, 1, 2);
  if (NEGATE_X) {
    line(1, 0, -1, 3); 
  }
  if (!NEGATE_X) {
    line(2, 2, 1, -2);   
  }
  line(3, 0, 1, 3);
  if (NEGATE_X) {
    line(3, 0, -1, 2); 
  }

  start('x', 3);
  line(0, 0, 2, 3);
  if (NEGATE_X) {
    line(2, 0, -2, 3); 
  }
  if (!NEGATE_X) {
    line(0, 3, 2, -3); 
  }

  start('y', 3);
  line(0, 0, 1, 0); 
  line(0, 2, 0, 1);
  arc(1, 1, 1, 0); 
  arc(1, 2, 0, 1); 
  line(2, 1, 0, 2); 

  start('z', 3);
  line(0, 0, 2, 0);
  line(0, 0, 2, 3);
  line(0, 3, 2, 0);

  
  
}

void decode() {

  std::ofstream ostr("test.svg");

  ostr << "<?xml version=\"1.0\" standalone=\"no\"?>\n";
  ostr << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"  \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";
  ostr << "<svg width=\"800 px\" height=\"600 px\" xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";
  

  int print = 0;
  uint16 w = 0;
  uint16 scl = 10;
  uint16 ox = 1;
  uint16 oy = 6;
  
  int cnt = 0;
  cur = 0;

  ostr << "  <path fill=\"none\" stroke=\"black\" stroke-width=\"4\" stroke-linecap=\"round\" d=\"";

  for (size_t i=0; buf[i]; ++i) {
    
    uint8 c1 = buf[i];
    uint8 c0 = buf[++i];
    uint16 val = (c0 << 8) | c1;

    if (c1 < bmin || c1 > bmax) {

      if (c0 == SEP_2) {
        cur = buf[++i];
      } else {
        --i;
        cur++;
      }

      ox += w;
      w = c1 - SEP_0; 
      if (print) std::cout << "\nstart(" << cur << ", " << w << ")\n";
      X_COORD = 0;
      if (cnt++ >= 13) { oy += 6; ox = 1; cnt = 0;}

    } else {
      uint16 x = (val >> X_COORD_SHIFT) & MASK(X_COORD_BITS);
      uint16 y = (val >> Y_COORD_SHIFT) & MASK(Y_COORD_BITS);
      uint16 op = (val >> OP_SHIFT) & MASK(OP_BITS);
      uint16 arg = (val >> ARG_SHIFT) & MASK(ARG_BITS);
      X_COORD += x;
      if (op == OP_LINE || op == OP_NLINE) {
        int ldx = ((arg >> LINE_DX_SHIFT) & MASK(LINE_DX_BITS));
        int ldy = (arg >> LINE_DY_SHIFT) & MASK(LINE_DY_BITS);
        if (op == OP_NLINE) { 
          if (NEGATE_X) {
            ldx *= -1; 
          } else {
            ldy *= -1;
          }
        }
        if (print) std::cout << "line(" << X_COORD << ", " << y << ", " << ldx << ", " << ldy << ");\n";
        ostr << "M " << scl*(ox+X_COORD) << " " << scl*(oy-y) << " l " << scl*ldx << " " << -scl*ldy << " ";
      } else if (op == OP_ARC) {
        uint16 a0 = (arg >> ARC_A0_SHIFT) & MASK(ARC_A0_BITS);
        uint16 a1 = (arg >> ARC_A1_SHIFT) & MASK(ARC_A1_BITS);
        if (print) std::cout << "arc(" << X_COORD << ", " << y << ", " << a0 << ", " << a1 << ");\n";
        float t0 = -M_PI+a0*M_PI/2;
        float t1 = t0 + (a1+1)*M_PI/2;
        float cx = scl*(ox+X_COORD);
        float cy = scl*(oy-y);
        for (float f=t0; f<t1; f+=M_PI/32) {
          float px = cx+scl*cos(f);
          float py = cy-scl*sin(f);
          if (f == t0) {
            ostr << "M " << px << " " << py << " ";
          } else {
            ostr << "L " << px << " " << py << " ";
          }
        }
      }
    }
  }

  ostr << "\" />\n";

  ostr << "</svg>\n";


}

void usage() {
  std::cout << "usage: encode -random\n";
  std::cout << "   or: encode -final\n";
  exit(1);
}

int main(int argc, char** argv) {

  if (argc != 2) {
    usage();
  }

  std::string arg(argv[1]);
  bool randomize;

  if (arg == "-random")  {
    randomize = true;
  } else if (arg == "-final") {
    randomize = false;
  } else {
    usage();
  }

  srand(time(NULL));

  size_t best(-1);
  bool was_good = false;

  for (int i=0; i<100000; ++i) {

    if (randomize) { permute_shifts(); }

    encode();
    bool this_good = (bmin > SEP_1);
    if (!this_good) { continue; }

    std::string dstr = stringify();
    std::string vstr = stringvars();
    
    size_t len = dstr.length() + vstr.length();
    
    if (this_good && (!was_good || len < best)) {

      was_good = true;
      best = len;

      if (shifts.size()) {
        std::cerr << "layout (LSB first):\n";
        size_t start = 0;
        for (size_t i=0; i<shifts.size(); ++i) {
          std::cerr << "  " << std::setw(10) << shifts[i].which << ": " << start << ":" << start + shifts[i].bits << "\n";
          start += shifts[i].bits;
        }
        std::cerr << "\n";
      }

      std::cerr << "NEGATE_X = " << NEGATE_X << "\n\n";
      std::cerr << "OP_LINE = " << OP_LINE << "\n";
      std::cerr << "OP_ARC = " << OP_ARC << "\n";
      std::cerr << "OP_NLINE = " << OP_NLINE << "\n\n";
      std::cerr << "OP_SHIFT = " << OP_SHIFT << "\n";
      std::cerr << "ARG_SHIFT = " << ARG_SHIFT << "\n";
      std::cerr << "X_COORD_SHIFT = " << X_COORD_SHIFT << "\n";
      std::cerr << "Y_COORD_SHIFT = " << Y_COORD_SHIFT << "\n\n";
      std::cerr << "LINE_DX_SHIFT = " << LINE_DX_SHIFT << "\n";
      std::cerr << "LINE_DY_SHIFT = " << LINE_DY_SHIFT << "\n\n";
      std::cerr << "ARC_A0_SHIFT = " << ARC_A0_SHIFT << "\n";
      std::cerr << "ARC_A1_SHIFT = " << ARC_A1_SHIFT << "\n\n";
      std::cerr << "PADDING_VAL = 0x" << std::hex << PADDING_VAL << std::dec << "\n\n";
      std::cerr << "SEP_0 = " << char(SEP_0) << "\n";
      std::cerr << "SEP_1 = " << char(SEP_1) << "\n";
      std::cerr << "SEP_2 = " << char(SEP_2) << "\n\n";
      
      std::cerr << "B* = \"" << dstr << "\"\n";
      std::cerr << "// data: " << dstr.length() << ", ideal: " << buf.size() << "\n";
      std::cerr << "// bmin: " << (int)bmin << "='" << bmin << "', "
                << "bmax: " << (int)bmax << "='" << bmax << "'\n";
      std::cerr << "// dxmin: " << (int)dxmin << " at " << dxminc << ", "
                << "dxmax: " << (int)dxmax << " at " << dxmaxc << "\n";
      std::cerr << vstr;
      std::cerr << "// vars: " << vstr.length() << " total: " << len << "\n";
      std::cerr << "\n";
      
      decode();
      std::cerr << "\n";

    }

    if (!randomize) { break; }

  }


  return 0;

}
