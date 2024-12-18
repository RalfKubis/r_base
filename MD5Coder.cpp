﻿/*
 **********************************************************************
 ** Copyright (C) 1990, RSA Data Security, Inc. All rights reserved. **
 **                                                                  **
 ** License to copy and use this software is granted provided that   **
 ** it is identified as the "RSA Data Security, Inc. MD5 Message     **
 ** Digest Algorithm" in all material mentioning or referencing this **
 ** software or this function.                                       **
 **                                                                  **
 ** License is also granted to make and use derivative works         **
 ** provided that such works are identified as "derived from the RSA **
 ** Data Security, Inc. MD5 Message Digest Algorithm" in all         **
 ** material mentioning or referencing the derived work.             **
 **                                                                  **
 ** RSA Data Security, Inc. makes no representations concerning      **
 ** either the merchantability of this software or the suitability   **
 ** of this software for any particular purpose.  It is provided "as **
 ** is" without express or implied warranty of any kind.             **
 **                                                                  **
 ** These notices must be retained in any copies of any part of this **
 ** documentation and/or software.                                   **
 **********************************************************************
 */

#include "r_base/MD5Coder.h"
#include "r_base/Error.h"

#include <cstdint>
#include "r_base/time.h"

namespace nsBase
{

/* Data structure for MD5 (Message Digest) computation */
/* Renamed from MD5_CTX due to name clash with identical structure
 defined in /usr/include/openssl/md5.h on LinuxAMD64 */
struct MD5Coder::MD5_CONTEXT
{
    ::std::uint32_t i[2];       /* number of _bits_ handled mod 2^64 */
    ::std::uint32_t buf[4];     /* scratch buffer */
    unsigned char in[64];     /* input buffer */
    MD5 digest; /* actual digest after MD5Final call */
};



void MD5Init   ( MD5Coder::MD5_CONTEXT * mdContext );
void MD5Update ( MD5Coder::MD5_CONTEXT * mdContext, unsigned char * inBuf, unsigned int inLen );
void MD5Final  ( MD5Coder::MD5_CONTEXT * mdContext );
static void Transform ( uint32_t * buf, uint32_t * inin );



static unsigned char PADDING[64] = {
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* F, G and H are basic MD5 functions: selection, majority, parity */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4 */
/* Rotation is separate from addition to prevent recomputation */
#define FF(a, b, c, d, x, s, ac) \
  {(a) += F ((b), (c), (d)) + (x) + (uint32_t)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) \
  {(a) += G ((b), (c), (d)) + (x) + (uint32_t)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) \
  {(a) += H ((b), (c), (d)) + (x) + (uint32_t)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) \
  {(a) += I ((b), (c), (d)) + (x) + (uint32_t)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }



void MD5Init( MD5Coder::MD5_CONTEXT * mdContext )
{
  mdContext->i[0] = mdContext->i[1] = (uint32_t)0;

  /* Load magic initialization constants.
   */
  mdContext->buf[0] = (uint32_t)0x67452301;
  mdContext->buf[1] = (uint32_t)0xefcdab89;
  mdContext->buf[2] = (uint32_t)0x98badcfe;
  mdContext->buf[3] = (uint32_t)0x10325476;
}



void MD5Update ( MD5Coder::MD5_CONTEXT * mdContext, unsigned char * inBuf, unsigned int inLen )
{
  uint32_t in[16];
  int mdi;
  unsigned int i, ii;

  /* compute number of bytes mod 64 */
  mdi = (int)((mdContext->i[0] >> 3) & 0x3F);

  /* update number of bits */
  if ((mdContext->i[0] + ((uint32_t)inLen << 3)) < mdContext->i[0])
    mdContext->i[1]++;
  mdContext->i[0] += ((uint32_t)inLen << 3);
  mdContext->i[1] += ((uint32_t)inLen >> 29);

  while (inLen--) {
    /* add new character to buffer, increment mdi */
    mdContext->in[mdi++] = *inBuf++;

    /* transform if necessary */
    if (mdi == 0x40) {
      for (i = 0, ii = 0; i < 16; i++, ii += 4)
        in[i] = (((uint32_t)mdContext->in[ii+3]) << 24) |
                (((uint32_t)mdContext->in[ii+2]) << 16) |
                (((uint32_t)mdContext->in[ii+1]) << 8) |
                ((uint32_t)mdContext->in[ii]);
      Transform (mdContext->buf, in);
      mdi = 0;
    }
  }
}



void MD5Final (MD5Coder::MD5_CONTEXT * mdContext)
{
  uint32_t in[16];
  int mdi;
  unsigned int i, ii;
  unsigned int padLen;

  /* save number of bits */
  in[14] = mdContext->i[0];
  in[15] = mdContext->i[1];

  /* compute number of bytes mod 64 */
  mdi = (int)((mdContext->i[0] >> 3) & 0x3F);

  /* pad out to 56 mod 64 */
  padLen = (mdi < 56) ? (56 - mdi) : (120 - mdi);
  MD5Update (mdContext, PADDING, padLen);

  /* append length in bits and transform */
  for (i = 0, ii = 0; i < 14; i++, ii += 4)
    in[i] = (((uint32_t)mdContext->in[ii+3]) << 24) |
            (((uint32_t)mdContext->in[ii+2]) << 16) |
            (((uint32_t)mdContext->in[ii+1]) << 8) |
            ((uint32_t)mdContext->in[ii]);
  Transform (mdContext->buf, in);

  /* store buffer in digest */
  for (i = 0, ii = 0; i < 4; i++, ii += 4)
  {
    mdContext->digest.m8[ii] = (unsigned char)(mdContext->buf[i] & 0xFF);
    mdContext->digest.m8[ii+1] =
      (unsigned char)((mdContext->buf[i] >> 8) & 0xFF);
    mdContext->digest.m8[ii+2] =
      (unsigned char)((mdContext->buf[i] >> 16) & 0xFF);
    mdContext->digest.m8[ii+3] =
      (unsigned char)((mdContext->buf[i] >> 24) & 0xFF);
  }
}



/* Basic MD5 step. Transform buf based on in.
 */
static void Transform (uint32_t *buf, uint32_t *in)
{
  uint32_t a = buf[0], b = buf[1], c = buf[2], d = buf[3];

  /* Round 1 */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
  FF ( a, b, c, d, in[ 0], S11, 3614090360u); /* 1 */
  FF ( d, a, b, c, in[ 1], S12, 3905402710u); /* 2 */
  FF ( c, d, a, b, in[ 2], S13,  606105819u); /* 3 */
  FF ( b, c, d, a, in[ 3], S14, 3250441966u); /* 4 */
  FF ( a, b, c, d, in[ 4], S11, 4118548399u); /* 5 */
  FF ( d, a, b, c, in[ 5], S12, 1200080426u); /* 6 */
  FF ( c, d, a, b, in[ 6], S13, 2821735955u); /* 7 */
  FF ( b, c, d, a, in[ 7], S14, 4249261313u); /* 8 */
  FF ( a, b, c, d, in[ 8], S11, 1770035416u); /* 9 */
  FF ( d, a, b, c, in[ 9], S12, 2336552879u); /* 10 */
  FF ( c, d, a, b, in[10], S13, 4294925233u); /* 11 */
  FF ( b, c, d, a, in[11], S14, 2304563134u); /* 12 */
  FF ( a, b, c, d, in[12], S11, 1804603682u); /* 13 */
  FF ( d, a, b, c, in[13], S12, 4254626195u); /* 14 */
  FF ( c, d, a, b, in[14], S13, 2792965006u); /* 15 */
  FF ( b, c, d, a, in[15], S14, 1236535329u); /* 16 */

  /* Round 2 */
#define S21 5
#define S22 9
#define S23 14
#define S24 20
  GG ( a, b, c, d, in[ 1], S21, 4129170786u); /* 17 */
  GG ( d, a, b, c, in[ 6], S22, 3225465664u); /* 18 */
  GG ( c, d, a, b, in[11], S23,  643717713u); /* 19 */
  GG ( b, c, d, a, in[ 0], S24, 3921069994u); /* 20 */
  GG ( a, b, c, d, in[ 5], S21, 3593408605u); /* 21 */
  GG ( d, a, b, c, in[10], S22,   38016083u); /* 22 */
  GG ( c, d, a, b, in[15], S23, 3634488961u); /* 23 */
  GG ( b, c, d, a, in[ 4], S24, 3889429448u); /* 24 */
  GG ( a, b, c, d, in[ 9], S21,  568446438u); /* 25 */
  GG ( d, a, b, c, in[14], S22, 3275163606u); /* 26 */
  GG ( c, d, a, b, in[ 3], S23, 4107603335u); /* 27 */
  GG ( b, c, d, a, in[ 8], S24, 1163531501u); /* 28 */
  GG ( a, b, c, d, in[13], S21, 2850285829u); /* 29 */
  GG ( d, a, b, c, in[ 2], S22, 4243563512u); /* 30 */
  GG ( c, d, a, b, in[ 7], S23, 1735328473u); /* 31 */
  GG ( b, c, d, a, in[12], S24, 2368359562u); /* 32 */

  /* Round 3 */
#define S31 4
#define S32 11
#define S33 16
#define S34 23
  HH ( a, b, c, d, in[ 5], S31, 4294588738u); /* 33 */
  HH ( d, a, b, c, in[ 8], S32, 2272392833u); /* 34 */
  HH ( c, d, a, b, in[11], S33, 1839030562u); /* 35 */
  HH ( b, c, d, a, in[14], S34, 4259657740u); /* 36 */
  HH ( a, b, c, d, in[ 1], S31, 2763975236u); /* 37 */
  HH ( d, a, b, c, in[ 4], S32, 1272893353u); /* 38 */
  HH ( c, d, a, b, in[ 7], S33, 4139469664u); /* 39 */
  HH ( b, c, d, a, in[10], S34, 3200236656u); /* 40 */
  HH ( a, b, c, d, in[13], S31,  681279174u); /* 41 */
  HH ( d, a, b, c, in[ 0], S32, 3936430074u); /* 42 */
  HH ( c, d, a, b, in[ 3], S33, 3572445317u); /* 43 */
  HH ( b, c, d, a, in[ 6], S34,   76029189u); /* 44 */
  HH ( a, b, c, d, in[ 9], S31, 3654602809u); /* 45 */
  HH ( d, a, b, c, in[12], S32, 3873151461u); /* 46 */
  HH ( c, d, a, b, in[15], S33,  530742520u); /* 47 */
  HH ( b, c, d, a, in[ 2], S34, 3299628645u); /* 48 */

  /* Round 4 */
#define S41 6
#define S42 10
#define S43 15
#define S44 21
  II ( a, b, c, d, in[ 0], S41, 4096336452u); /* 49 */
  II ( d, a, b, c, in[ 7], S42, 1126891415u); /* 50 */
  II ( c, d, a, b, in[14], S43, 2878612391u); /* 51 */
  II ( b, c, d, a, in[ 5], S44, 4237533241u); /* 52 */
  II ( a, b, c, d, in[12], S41, 1700485571u); /* 53 */
  II ( d, a, b, c, in[ 3], S42, 2399980690u); /* 54 */
  II ( c, d, a, b, in[10], S43, 4293915773u); /* 55 */
  II ( b, c, d, a, in[ 1], S44, 2240044497u); /* 56 */
  II ( a, b, c, d, in[ 8], S41, 1873313359u); /* 57 */
  II ( d, a, b, c, in[15], S42, 4264355552u); /* 58 */
  II ( c, d, a, b, in[ 6], S43, 2734768916u); /* 59 */
  II ( b, c, d, a, in[13], S44, 1309151649u); /* 60 */
  II ( a, b, c, d, in[ 4], S41, 4149444226u); /* 61 */
  II ( d, a, b, c, in[11], S42, 3174756917u); /* 62 */
  II ( c, d, a, b, in[ 2], S43,  718787259u); /* 63 */
  II ( b, c, d, a, in[ 9], S44, 3951481745u); /* 64 */

  buf[0] += a;
  buf[1] += b;
  buf[2] += c;
  buf[3] += d;
}


/* Copyright (C) Ralf Kubis */

R_DTOR_IMPL(MD5Coder) = default;
R_CMOV_IMPL(MD5Coder) = default;
R_MOVE_IMPL(MD5Coder) = default;


MD5Coder::MD5Coder()
{
    clear();
}


void
MD5Coder::clear()
{
    context = ::std::make_unique<MD5Coder::MD5_CONTEXT>();
    MD5Init(context.get());
}


void
MD5Coder::merge(
    void  const  * buffer
,   ::std::size_t  buffer_len
)
{
    if (DBC_FAIL(!context->digest))
        "20aaaa39-5a85-4f4d-952e-61e050fcfb63"_log("coder was already finalized").throw_error();

    MD5Update(context.get(), (unsigned char*)buffer, buffer_len);
}


MD5
MD5Coder::query()
{
    if (!context->digest)
        MD5Final(context.get());
    DBC_ASSERT(context->digest);

    return context->digest;
}


void
MD5Coder::query(
    unsigned char target[16]
)
{
    if (!context->digest)
        MD5Final(context.get());
    DBC_ASSERT(context->digest);

    for (int i=16 ; i-- ;)
        target[i] = context->digest.m8[i];
}


MD5Coder &
    operator<<(
            MD5Coder          & c
        ,   ::std::string const & s
        )
        {
            if (!s.empty())
                c.merge(s.data(), s.size());
            return c;
        }


MD5Coder &
    operator<<(
            MD5Coder          & c
        ,   ::std::basic_string<::std::uint8_t> const & s
        )
        {
            if (!s.empty())
                c.merge(s.data(), s.size());
            return c;
        }


MD5Coder &
    operator<<(
            MD5Coder      & c
        ,   MD5     const & d
        )
        {
            c.merge(d.m8, sizeof(d.m8));
            return c;
        }


MD5Coder &
    operator<<(
            MD5Coder            & c
        ,   ::uuids::uuid const & d
        )
        {
            for (auto b : d.as_bytes())
                c.merge(&b, 1);
            return c;
        }

MD5Coder &
    operator<<(
            MD5Coder                                    & c
        ,   ::std::chrono::system_clock::time_point const & d
        )
        {
            return c << to_string(d);
        }

MD5Coder & operator<<(MD5Coder & c, ::std::int8_t   d){c.merge(&d, sizeof(d)); return c;}
MD5Coder & operator<<(MD5Coder & c, ::std::uint8_t  d){c.merge(&d, sizeof(d)); return c;}
MD5Coder & operator<<(MD5Coder & c, ::std::int16_t  d){c.merge(&d, sizeof(d)); return c;}
MD5Coder & operator<<(MD5Coder & c, ::std::uint16_t d){c.merge(&d, sizeof(d)); return c;}
MD5Coder & operator<<(MD5Coder & c, ::std::int32_t  d){c.merge(&d, sizeof(d)); return c;}
MD5Coder & operator<<(MD5Coder & c, ::std::uint32_t d){c.merge(&d, sizeof(d)); return c;}
MD5Coder & operator<<(MD5Coder & c, ::std::int64_t  d){c.merge(&d, sizeof(d)); return c;}
MD5Coder & operator<<(MD5Coder & c, ::std::uint64_t d){c.merge(&d, sizeof(d)); return c;}
MD5Coder & operator<<(MD5Coder & c,            bool d){return c << (d ? ::std::uint8_t(1) : ::std::uint8_t(0));}
MD5Coder & operator<<(MD5Coder & c,           float d){c.merge(&d, sizeof(d)); return c;}
MD5Coder & operator<<(MD5Coder & c,          double d){c.merge(&d, sizeof(d)); return c;}

}
