/**
    Copyright (C) powturbo 2013-2015
    GPL v2 License
  
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    - homepage : https://sites.google.com/site/powturbo/
    - github   : https://github.com/powturbo
    - twitter  : https://twitter.com/powturbo
    - email    : powturbo [_AT_] gmail [_DOT_] com
**/
//     vp4dd.c - "Integer Compression" Turbo PforDelta 
  #ifndef USIZE
#include <string.h>
#include "conf.h"
#include "bitpack.h"

#include "vp4dc.h"

#define PAD8(__x) ( (((__x)+8-1)/8) )
#include <stdint.h>
//------------------------------------------
#define P4DSIZE 128 //64 //

#define P4D p4d
#define P4DENC p4denc
#define P4DE p4de

#define BITPACK bitpack
#define USIZE 32
#include __FILE__
#undef USIZE

#define USIZE 16
#include __FILE__
#undef USIZE

#undef BITPACK 
#undef P4DENC
#undef P4DE
#undef P4D
//------------------------------------------
#define P4DSIZE 128 //64 //

#define P4DENC p4dencv
#define P4DE p4dev
#define BITPACK bitpackv
#define USIZE 32
#include __FILE__
#undef USIZE

/*#define USIZE 16
#include __FILE__*/

  #else
#pragma clang diagnostic push 
#pragma clang diagnostic ignored "-Wparentheses"

#define uint_t TEMPLATE3(uint, USIZE, _t)
#define P4DN   (P4DSIZE/64)

    #ifdef P4D
unsigned TEMPLATE2(P4D, USIZE)(uint_t *__restrict in, unsigned n, unsigned *pbx) {
  uint_t *ip; unsigned b=0, cnt[USIZE+1] = {0}; 
  
  for(ip = in; ip != in+(n&~3); ) {
    ++cnt[TEMPLATE2(bsr, USIZE)(*ip)]; b |= *ip++;
    ++cnt[TEMPLATE2(bsr, USIZE)(*ip)]; b |= *ip++;
    ++cnt[TEMPLATE2(bsr, USIZE)(*ip)]; b |= *ip++;
    ++cnt[TEMPLATE2(bsr, USIZE)(*ip)]; b |= *ip++;
  }
  while(ip != in+n) ++cnt[TEMPLATE2(bsr, USIZE)(*ip)], b |= *ip++;
  b = TEMPLATE2(bsr, USIZE)(b); 

  int i; unsigned bx = b, ml = PAD8(n*b)+1, x = cnt[b];
  for(i = b-1; i >= 0; --i) {
    unsigned l = PAD8(n*i) + 2+P4DN*8+PAD8(x*(bx-i)); 
	if(unlikely(l < ml)) b = i, ml = l; x += cnt[i]; //if(x >= 64) break;
  }
  *pbx = bx - b;
  return b;
} 
#endif
 
unsigned char *TEMPLATE2(P4DE, USIZE)(uint_t *__restrict in, unsigned n, unsigned char *__restrict out, unsigned b, unsigned bx) { unsigned i, xn, msk = (1ull<<b)-1, c;
  if(!bx) return TEMPLATE2(BITPACK, USIZE)(in,  n, out, b);
  
  uint_t             _in[P4DSIZE], inx[P4DSIZE*2]; 
  unsigned long long xmap[P4DN+1];  
  unsigned           miss[P4DSIZE];
  for(i = 0; i < P4DN; i++) xmap[i] = 0;
  for(xn = i = 0; i != n&~3; ) {
    miss[xn] = i; xn += in[i] > msk; _in[i] = in[i] & msk; i++;
    miss[xn] = i; xn += in[i] > msk; _in[i] = in[i] & msk; i++;
    miss[xn] = i; xn += in[i] > msk; _in[i] = in[i] & msk; i++;
    miss[xn] = i; xn += in[i] > msk; _in[i] = in[i] & msk; i++;
  }  
  while(i != n) { miss[xn] = i; xn += in[i] > msk; _in[i] = in[i] & msk; i++; }
  for(i = 0; i < xn; i++) {
    unsigned c = miss[i]; 
    xmap[c>>6] |= (1ull << (c&0x3f)); 
    inx[i] = in[c] >> b; 
  }
  out = TEMPLATE2(BITPACK, USIZE)(_in,  n, out,  b);	
  for(i=0;i < P4DN; i++) { *(unsigned long long *)out = xmap[i]; out += 8; } 
  return TEMPLATE2(bitpack, USIZE)(inx, xn, out, bx);  						
}

unsigned char *TEMPLATE2(P4DENC, USIZE)(uint_t *__restrict in, unsigned n, unsigned char *__restrict out) {  
  unsigned bx,b = TEMPLATE2(p4d, USIZE)(in, n, &bx);
  P4DSAVE(out, b, bx);
  return TEMPLATE2(P4DE, USIZE)(in, n, out, b, bx);
}
#pragma clang diagnostic pop
  #endif
