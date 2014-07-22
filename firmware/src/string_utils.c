#include <stdint.h>

#include "string_utils.h"

static void strreverse( char *b, char *e ) 
{       
        char a;
        while(e>b)
        {
          a=*e;
          *e--=*b;
          *b++=a;
        }
}

void string2uint( int32_t value, char padd, char *o )
{
    char *s = o;
    char c = 0;
    do
    {
      *o++ = (value%10)+'0';
      value/=10;
      ++c;
    }
    while( value );
    while( c < padd )
    {
      *o++ = ' ';
      ++c;
    }
    *o = 0;
    strreverse( s, o-1 );
}

void string2int( int32_t value, char padd, char *o )
{
    char *s = o;
    char c = 0;
    char sign = ' ';
    if( value < 0 )
    {
        sign = '-';
        value = -value;
    }
    do
    {
      *o++ = (value%10)+'0';
      value/=10;
      ++c;
    }
    while( value );
    while( c < padd )
    {
      *o++ = ' ';
      ++c;
    }
    *o++ = sign;
    *o = 0;
    strreverse( s, o-1 );
}

void xhc2string( uint16_t iint, uint16_t ifrac, char ipadd, char fpadd, char *o )
{
    char *s = o;
    char c = 0;
    char sign = ' ';
    if( ifrac & 0x8000u )
    {
        sign = '-';
        ifrac &= ~0x8000u;
    }
    
    do
    {
      *o++ = (ifrac%10)+'0';
      ifrac/=10;
      ++c;
    }
    while( ifrac );
    while( c < fpadd )
    {
      *o++ = '0';
      ++c;
    }
    *o++ = '.';
    c = 0;
    do
    {
      *o++ = (iint%10)+'0';
      iint/=10;
      ++c;
    }
    while( iint );
    while( c < ipadd )
    {
      *o++ = ' ';
      ++c;
    }
    *o++ = sign;
    *o = 0;
    strreverse( s, o-1 );
}

void int2strprec( int32_t v, char padd, char *o )
{
    char *s = o;
    char c = 0;
    char sign = ' ';
    if( v < 0 )
    {
      sign = '-';
      v = -v;
    }
    do
    {
      *o++ = (v%10)+'0';
      ++c;
      if( c == 2 )
      {
        *o++ = '.';
      }
      v/=10;
    }
    while( v );
    while( c < padd )
    {
        *o++ = ' ';
    }
      
    *o++ = sign;
    *o = 0;
    strreverse( s, o-1 );
}
