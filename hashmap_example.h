#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

struct Glyph
{
  int codePoint;
  int height;
  int width;
  int advance;

  operator bool()
  {
    return codePoint != 0 && height != 0 && width != 0 && advance != 0;
  }
};

struct GlyphToCodepoint
{
  int key; // 0 - 1mil
  Glyph value;
};

void test_hashmap()
{
  GlyphToCodepoint* table = nullptr;
  const int codePoint = 43206; // Some chin char
  Glyph chinGlyph = {
    .codePoint = codePoint, 
    .height = 24, 
    .width = 48, 
    .advance = 42};

  int codePoint2 = 4326; // Some other chin char
  hmput(table, codePoint, chinGlyph);
  hmput(table, codePoint2, chinGlyph);

  Glyph& testRef = hmget(table, codePoint2);
  codePoint2 = 2342;
  if(Glyph test = hmget(table, codePoint2))
  {
    int a = 0;
  }

  testRef.advance = 10'000.0f;
}