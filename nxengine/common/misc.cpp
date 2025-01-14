#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <ctype.h>

#include "basics.h"
#include "misc.fdh"
#include "../nx.h"

#ifdef _WIN32
#include "msvc_compat.h"
#endif

#ifndef MSB_FIRST
uint16_t fgeti(FILE *fp)
{
   uint16_t value;
   fread(&value, 2, 1, fp);
   return value;
}

uint32_t fgetl(FILE *fp)
{
   uint32_t value;
   fread(&value, 4, 1, fp);
   return value;
}

void fputi(uint16_t word, FILE *fp)
{
   fwrite(&word, 2, 1, fp);
}

void fputl(uint32_t word, FILE *fp)
{
   fwrite(&word, 4, 1, fp);
}

double fgetfloat(FILE *fp)
{
   char buf[8];
   double *float_ptr;
   int i;

   for(i=0;i<4;i++) fgetc(fp);
   for(i=0;i<8;i++) buf[i] = fgetc(fp);

   float_ptr = (double *)&buf[0];
   return *float_ptr;
}

void fputfloat(double q, FILE *fp)
{
   char *float_ptr;
   int i;

   float_ptr = (char *)&q;

   for(i=0;i<4;i++) fputc(0, fp);
   for(i=0;i<8;i++) fputc(float_ptr[i], fp);

   return;
}
#else
uint16_t fgeti(FILE *fp)
{
   uint16_t a, b;
   a = fgetc(fp);
   b = fgetc(fp);
   return (b << 8) | a;
}

uint32_t fgetl(FILE *fp)
{
   uint32_t a, b, c, d;
   a = fgetc(fp);
   b = fgetc(fp);
   c = fgetc(fp);
   d = fgetc(fp);
   return (d<<24)|(c<<16)|(b<<8)|(a);
}

void fputi(uint16_t word, FILE *fp)
{
   fputc(word, fp);
   fputc(word >> 8, fp);
}

void fputl(uint32_t word, FILE *fp)
{
   fputc(word, fp);
   fputc(word >> 8, fp);
   fputc(word >> 16, fp);
   fputc(word >> 24, fp);
}

double fgetfloat(FILE *fp)
{
   char buf[8];
   double *float_ptr;
   int i;

   for(i=0;i<4;i++) fgetc(fp);
   for(i=0;i<8;i++) buf[7 - i] = fgetc(fp);

   float_ptr = (double *)&buf[0];
   return *float_ptr;
}

void fputfloat(double q, FILE *fp)
{
   char *float_ptr;
   int i;

   float_ptr = (char *)&q;

   for(i=0;i<4;i++) fputc(0, fp);
   for(i=0;i<8;i++) fputc(float_ptr[7 - i], fp);

   return;
}
#endif

// write a string to a file-- does NOT null-terminate it
void fputstringnonull(const char *buf, FILE *fp)
{
   if (buf[0])
      fprintf(fp, "%s", buf);
}


// reads strlen(str) bytes from file fp, and returns true if they match "str"
bool fverifystring(FILE *fp, const char *str)
{
   int i;
   char result = 1;
   int stringlength = strlen(str);

   for(i=0;i<stringlength;i++)
   {
      if (fgetc(fp) != str[i]) result = 0;
   }

   return result;
}

// read data from a file until CR
void fgetline(FILE *fp, char *str, int maxlen)
{
   int k;
   str[0] = 0;
   fgets(str, maxlen - 1, fp);

   // trim the CRLF that fgets appends
   for(k=strlen(str)-1;k>=0;k--)
   {
      if (str[k] != 13 && str[k] != 10) break;
      str[k] = 0;
   }
}

bool file_exists(const char *fname)
{
   FILE *fp;

   fp = fopen(fname, "rb");
   if (!fp) return 0;
   fclose(fp);
   return 1;
}

/*
   void c------------------------------() {}
   */

static uint32_t seed = 0;

// return a random number between min and max inclusive
int random(int min, int max)
{
   int range, val;

   if (max < min)
   {
      NX_ERR("random(): warning: max < min [%d, %d]\n", min, max);
      min ^= max;
      max ^= min;
      min ^= max;
   }

   range = (max - min);

   if (range >= RAND_MAX)
   {
      NX_ERR("random(): range > RAND_MAX\n", min, max);
      return 0;
   }

   val = getrand() % (range + 1);
   return val + min;
}

uint32_t getrand()
{
   seed = (seed * 0x343FD) + 0x269EC3;
   return seed;
}

void seedrand(uint32_t newseed)
{
   seed = newseed;
}

/*
   void c------------------------------() {}
   */


bool strbegin(const char *bigstr, const char *smallstr)
{
   int i;

   for(i=0;smallstr[i];i++)
      if (bigstr[i] != smallstr[i]) return false;

   return true;
}

// a strncpy that works as you might expect
void maxcpy(char *dst, const char *src, int maxlen)
{
   int len = strlen(src);

   if (len >= maxlen)
   {
      if (maxlen >= 2) memcpy(dst, src, maxlen - 2);
      if (maxlen >= 1) dst[maxlen - 1] = 0;
   }
   else
   {
      memcpy(dst, src, len + 1);
   }
}

/*
   void c------------------------------() {}
   */

static int boolbyte, boolmask_r, boolmask_w;

// prepare for a boolean read operation
void fresetboolean(void)
{
   boolmask_r = 256;
   boolmask_w = 1;
   boolbyte = 0;
}

// read a boolean value (a single bit) from a file
char fbooleanread(FILE *fp)
{
   char value;

   if (boolmask_r == 256)
   {
      boolbyte = fgetc(fp);
      boolmask_r = 1;
   }

   value = (boolbyte & boolmask_r) ? 1:0;
   boolmask_r <<= 1;
   return value;
}

void fbooleanwrite(char bit, FILE *fp)
{
   if (boolmask_w == 256)
   {
      fputc(boolbyte, fp);
      boolmask_w = 1;
      boolbyte = 0;
   }

   if (bit)
   {
      boolbyte |= boolmask_w;
   }

   boolmask_w <<= 1;
}

void fbooleanflush(FILE *fp)
{
   fputc(boolbyte, fp);
   boolmask_w = 1;
}



