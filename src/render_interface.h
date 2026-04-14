#pragma once
#include "lib.h"
#include "assets.h"

// For font
// To Load TTF Files
#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ftstroke.h>


// For Hashmaps
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

static SDL_Renderer* renderer;
static SDL_Texture* atlas;

// #############################################################################
//                           Render Constats
// #############################################################################
constexpr u32 MAX_TRANSFORMS = 1000;
const u32 FONT_ATLAS_SIZE = 512;

enum FontType
{
  FONT_HEADING,
  FONT_TEXT,

  FONT_COUNT
};

// #############################################################################
//                           Render Structs
// #############################################################################
enum OthoProjectionType
{
  ORTHO_PROJ_GAME,    // Scaled by 4
  ORTHO_PROJ_LIGHTS,  // Scaled by Game - 1
  ORTHO_PROJ_UI_1,    // Scaled by 1
  ORTHO_PROJ_UI_2,    // Scaled by 2
  ORTHO_PROJ_UI_3,    // Scaled by 3
  ORTHO_PROJ_UI_4,    // Scaled by 4
  
  ORTHO_PROJ_COUNT
};

struct OrthographicCamera2D
{
  float zoom = 1.0f;
  Vec2 dimensions;
  Vec2 position;
};

struct Transform
{
  i32 atlasPosPacked;
  Vec2 spriteSize;
  Vec2 pos;
  Vec2 size;
  int matrixIdx;
  int fontIdx;
  int padding[2];
};

struct Color
{
  union
  {
    u32 hex;
    u8 channels[4];
  };

  bool operator==(const Color& other)
  {
    return hex == other.hex;
  }
  bool operator!=(const Color& other)
  {
    return !(hex == other.hex);
  }
};

// This is Cool!
constexpr Color COLOR_GREEN = {.hex = 0x1e6f50FF};
constexpr Color COLOR_GREEN_SAME = {.channels{30, 111, 80, 255}};

struct Material
{
  Color color;

  bool operator==(const Material& other)
  {
    return color == other.color;
  }
};

struct GlobalData
{
  float gameTime;
  float padding;
  IVec2 windowSize;
  Mat4 orthProjGame[ORTHO_PROJ_COUNT];
};

struct Glyph
{
  Vec2 offset;
  Vec2 advance;
  Vec2 textureCoords;
  Vec2 size;
  Vec2 descend;
};

struct CodepointToGlyph
{
  u32 key;
  Glyph value;
};

struct Font
{
  char name[256];
  bool reload;
  char* bitMap;
  int height;
  int size;
  CodepointToGlyph* glyphs = nullptr;
};

struct RenderData
{
  OrthographicCamera2D gameCam;
  OrthographicCamera2D lightsCam;
  OrthographicCamera2D uiCam;
  OrthographicCamera2D uiCams[4];
  GlobalData globalData;

  Vec2 uiSpace;
  Font fonts[FONT_COUNT];

  int transformCount = 0;
  Transform transforms[MAX_TRANSFORMS];

  int uiTransformCount = 0;
  Transform uiTransforms[MAX_TRANSFORMS];

  int lightCount = 0;
  Transform lights[MAX_TRANSFORMS];
};

// #############################################################################
//                           Render Globals
// #############################################################################
static RenderData renderData = {};

// #############################################################################
//                           Render Helper Functions
// #############################################################################
void precise_wait_seconds(double seconds)
{
  // if our machine can produce 70 fps maximum, but we limit 120
  if(seconds < 0.0) return;

  u64 start_tick = SDL_GetPerformanceCounter();
  u64 frequency = SDL_GetPerformanceFrequency();
  u64 target_tick = start_tick + static_cast<u64>(seconds * static_cast<double>(frequency)); // don't complain about the cast, msvc warnings sir

  while (SDL_GetPerformanceCounter() < target_tick)
  {
    SDL_CPUPauseInstruction();
    // _mm_pause();
  }
}

const Glyph& get_glyph_if_exists(const Font& font, int codePoint)
{
  // Cast const away because this is how the library needs it
  CodepointToGlyph* glyphs = (CodepointToGlyph*)font.glyphs;
  Glyph& glyph = hmget(glyphs, codePoint);
  if(glyph.textureCoords != VEC2_ZERO)
  {
    return glyph;
  }

  u32 questionmark = (u32)'?';
  Glyph& glyph2 = hmget(glyphs, questionmark);
  return glyph2;
}

void store_glyph_in_bitmap(FT_Face fontFace, FT_UInt codepoint, FT_Stroker stroker, Font& font, 
  char* fontAtlas, int& col, int& row, const int padding, const int fontSize)
{
    FT_UInt glyphIdx = FT_Get_Char_Index(fontFace, codepoint);
    FT_Load_Glyph(fontFace, glyphIdx, FT_LOAD_DEFAULT);

    FT_Glyph ftGlyph;
    FT_Get_Glyph(fontFace->glyph, &ftGlyph);
    FT_Glyph_StrokeBorder(&ftGlyph, stroker, false, true);
    FT_Glyph_To_Bitmap(&ftGlyph, FT_RENDER_MODE_NORMAL, nullptr, true);

    FT_Error error = FT_Render_Glyph(fontFace->glyph, FT_RENDER_MODE_NORMAL);
    FT_Bitmap bitmapOutlineGlyph = reinterpret_cast<FT_BitmapGlyph>(ftGlyph)->bitmap;
    FT_Bitmap bitmapGlyph = fontFace->glyph->bitmap;

    // blit the glyph here on your target surface.
    // For positioning use bitmapGlyph->left, bitmapGlyph->top
    // For iteration over the glyph data use bitmapGlyph->bitmap.buffer, 
    // bitmapGlyph->bitmap.width, bitmapGlyph->bitmap.rows, bitmapGlyph->bitmap.pitch.

    if (col + bitmapOutlineGlyph.width + padding >= 512)
    {
      col = padding;
      row += fontSize + padding;
    }

    // Font Height
    font.height = 
      max((fontFace->size->metrics.ascender - fontFace->size->metrics.descender) >> 6, 
          font.height);

    for (unsigned int y = 0; y < bitmapOutlineGlyph.rows; ++y)
    {
      for (unsigned int x = 0; x < bitmapOutlineGlyph.width; ++x)
      {

        fontAtlas[(row + y) * FONT_ATLAS_SIZE * 2 + (col + x) * 2] =
            bitmapOutlineGlyph.buffer[y * bitmapOutlineGlyph.width + x];
      }
    }

    float thicknessX = (bitmapOutlineGlyph.width - bitmapGlyph.width) / 2.0f;
    float thicknessY = (bitmapOutlineGlyph.rows - bitmapGlyph.rows) / 2.0f;
    int chatsThicknessX = floorf(thicknessX);
    int chatsThicknessY = floorf(thicknessY);

    for (unsigned int y = 0; y < bitmapGlyph.rows; ++y)
    {
      for (unsigned int x = 0; x < bitmapGlyph.width; ++x)
      {
        fontAtlas[(row + y + chatsThicknessY) * FONT_ATLAS_SIZE * 2 + 
                  (col + x + chatsThicknessX) * 2 + 1]
          = bitmapGlyph.buffer[y * bitmapGlyph.width + x];
      }
    }

    // Glyph* glyph = &renderData->glyphs[glyphIdx];
    Glyph glyph = {};
    glyph.textureCoords = Vec2{(float)col, (float)row};
    glyph.descend = Vec2{0.0f, (float)((fontFace->size->metrics.descender >> 6) * fontSize / 16)};
    glyph.size = 
    { 
      (float)bitmapOutlineGlyph.width, 
      (float)bitmapOutlineGlyph.rows
    };
    glyph.advance = 
    {
      (float)(fontFace->glyph->advance.x >> 6), 
      (float)(fontFace->glyph->advance.y >> 6)
    };
    glyph.offset =
    {
      (float)fontFace->glyph->bitmap_left,
      (float)fontFace->glyph->bitmap_top,
    };
    // Add Glyph to font
    // *(font.glyphs->insert(glyph))[glyphIdx] = glyph;
    // font.glyphs->emplace(codepoint, glyph);
    hmput(font.glyphs, codepoint, glyph);

    // col += fontFace->glyph->bitmap.width + padding;
    col += bitmapOutlineGlyph.width + padding;
}

void load_font(FontType fontType, char* filePath, int fontSize, int outlineThickness, int extraHeight = 0)
{
  SDL_Log("Loading Font: %s", filePath);
  
  // We store 2 colos in the atlas, Green for the Glyph, Red for the outline
  constexpr int ATLAS_SIZE = FONT_ATLAS_SIZE * FONT_ATLAS_SIZE * 2;
  static char BITMAPS[FONT_COUNT][ATLAS_SIZE];
  memset(BITMAPS[fontType], 0, ATLAS_SIZE);

  FT_Library fontLibrary;
  FT_Init_FreeType(&fontLibrary);

  FT_Face fontFace;
  FT_New_Face(fontLibrary, filePath, 0, &fontFace);
  ASSERT_MSG(fontFace, "Failed to call FT_New_Face");

  FT_Set_Pixel_Sizes(fontFace, 0, fontSize);

  int padding = 4 + outlineThickness;
  int row = 10; // Top padding
  int col = padding;

  FT_Stroker stroker;
  FT_Stroker_New(fontLibrary, &stroker);

  // This generates a basic font
  Font& font = renderData.fonts[fontType];
  font = {}; 
  font.reload = true; // Signal to the renderer to reload the font
  char* bitMap = BITMAPS[fontType];
  font.bitMap = bitMap;
  memcpy(font.name, filePath, str_len(filePath));
  font.height = fontSize;
  font.size = fontSize;

  //  2 * 64 result in 2px outline
  FT_Stroker_Set(stroker, outlineThickness * 64, 
                 FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);

  // Load common chars
  const char8_t* chineseChars = u8"不可思议";
  const char8_t* latinChars = u8"äüöÄÜÖßêéáéíóúüñÁÉÍÓÚÜÑ¿¡áàãçÇãéáàêîôûÁáÀàÃãÇçÉéÍíÓóÕõÚúÔôțăâîşţ";
  const char8_t* polishChars = u8"ĘęÓóĄąŚśŻżŹźĆćŃńŁł";
  const char8_t* turkishChars = u8"IıĞğŞşÇçÖöÜüİ";
  const char8_t* russianUrkChars = u8"АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНнОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯяҐґЄєІіЇї";

  while(FT_UInt chnCodepoint = decode_utf8(&chineseChars))
  {
    store_glyph_in_bitmap(fontFace, chnCodepoint, stroker, font, bitMap, col, row, padding, fontSize);
  }
  while(FT_UInt gerCodepoint = decode_utf8(&latinChars))
  {
    store_glyph_in_bitmap(fontFace, gerCodepoint, stroker, font, bitMap, col, row, padding, fontSize);
  }
  while(FT_UInt gerCodepoint = decode_utf8(&polishChars))
  {
    store_glyph_in_bitmap(fontFace, gerCodepoint, stroker, font, bitMap, col, row, padding, fontSize);
  }
  while(FT_UInt gerCodepoint = decode_utf8(&turkishChars))
  {
    store_glyph_in_bitmap(fontFace, gerCodepoint, stroker, font, bitMap, col, row, padding, fontSize);
  }
  while(FT_UInt gerCodepoint = decode_utf8(&russianUrkChars))
  {
    store_glyph_in_bitmap(fontFace, gerCodepoint, stroker, font, bitMap, col, row, padding, fontSize);
  }

  for (FT_ULong codepoint = 32; codepoint < 127; ++codepoint)
  {
    store_glyph_in_bitmap(fontFace, codepoint, stroker, font, bitMap, col, row, padding, fontSize);
  }

  FT_Done_Face(fontFace);
  FT_Done_FreeType(fontLibrary);

  // '\n' Glyph
  Glyph glyph = {};
  u32 lineBreak = '\n';
  hmput(font.glyphs, lineBreak, glyph);

  font.height = font.height + extraHeight;
  font.size = fontSize + extraHeight;
}


// float world_to_ui_x(float x)
// {
//   const OrthographicCamera2D& gameCam = renderData->gameCamera;
//   const OrthographicCamera2D& uiCam = renderData->uiCamera;
//   float xPos = (x) / gameCam.dimensions.x * uiCam.dimensions.x; // [0; dimensions.x]
//   return xPos;
// }
// float world_to_ui_y(float y)
// {
//   const OrthographicCamera2D& gameCam = renderData->gameCamera;
//   const OrthographicCamera2D& uiCam = renderData->uiCamera;
//   float yPos = (y) / gameCam.dimensions.y * uiCam.dimensions.y; // [0; dimensions.y]
//   return yPos;
// }

// Vec2 world_to_ui(Vec2 worldPos)
// {
//   const OrthographicCamera2D& gameCam = renderData->gameCamera;
//   const OrthographicCamera2D& uiCam = renderData->uiCamera;

//   float xPos = world_to_ui_x(worldPos.x);
//   float yPos = world_to_ui_y(worldPos.y);

//   return {xPos, yPos};
// }

// #############################################################################
//                           Render Functions
// #############################################################################
i32 pack_int(const IVec2& v)
{
  return v.x + (v.y << 16);
}

i32 pack_int(const Vec2& v)
{
  return (int)v.x + ((int)v.y << 16);
}

void draw_sprite(const SpriteID spriteID, Vec2 pos, float scale = 1)
{
  const Sprite& sprite = SPRITES[spriteID];
  Transform t = 
  {
    .atlasPosPacked = pack_int(sprite.atlasOffset),
    .spriteSize = vec_2(sprite.size),
    .pos = pos - vec_2(sprite.size/2 - sprite.pivotOffset) * scale,
    .size = vec_2(sprite.size) * scale,
    .matrixIdx = ORTHO_PROJ_GAME
  };
  renderData.transforms[renderData.transformCount++] = t;
}

void draw_light(Vec2 pos, float scale = 1)
{
  const Sprite& sprite = SPRITES[SPRITE_LIGHT];
  Transform t = 
  {
    .atlasPosPacked = pack_int(sprite.atlasOffset),
    .spriteSize = vec_2(sprite.size),
    .pos = pos - vec_2(sprite.size/2 - sprite.pivotOffset) * scale,
    .size = vec_2(sprite.size) * scale,
    .matrixIdx = ORTHO_PROJ_LIGHTS
  };
  renderData.lights[renderData.lightCount++] = t;
}

enum BaseFontSize
{
  FONT_SIZE_1 = ORTHO_PROJ_UI_1,
  FONT_SIZE_2 = ORTHO_PROJ_UI_2,
  FONT_SIZE_3 = ORTHO_PROJ_UI_3,
  FONT_SIZE_4 = ORTHO_PROJ_UI_4,
};

struct TextData
{
  FontType fontType = FONT_TEXT;
  BaseFontSize baseFontSize = FONT_SIZE_3;
  float fontSize = 1.0f;
  float rotation = 0;
  // todo: function over time like sinf
  // todo: individual scaling
};

void draw_ui_text(char* text, Vec2 pos, const TextData& textData = {})
{
  const Font& font = renderData.fonts[textData.fontType];
  const float spaceWidth = get_glyph_if_exists(font, ' ').advance.x * textData.fontSize;
  const Words words = split_words(text);
  Vec2 origin = pos;

  for(int wordIdx = 0; wordIdx < words.count; wordIdx++)
  {
    const unsigned int* word = words.values[wordIdx];

    if(*word == '\r')
    {
      continue;
    }

    if(*word == '\n')
    {
      // Move back to origin and down
      pos.x = origin.x;
      pos.y += font.height * textData.fontSize;
      continue;
    }

    if(*word == '[' && *(word + 1) == 'h')
    {
      // Move back to origin and down
      int height = (*(word + 2) - '0') + 2;
      pos.x = origin.x;
      pos.y += height * textData.fontSize;
      continue;
    }

    while(unsigned int codepoint = *(word++))
    {
      if(codepoint == '|')
      {
        continue;
      }

      if(codepoint == '\r')
      {
        continue;
      }

      // Draw the Glyph
      const Glyph& glyph = get_glyph_if_exists(font, codepoint);
      Transform t = 
      {
        .atlasPosPacked = pack_int(glyph.textureCoords),
        .spriteSize = glyph.size,
        .pos = pos - glyph.offset * textData.fontSize,
        .size = glyph.size * textData.fontSize,
        .matrixIdx = textData.baseFontSize,
        .fontIdx = (int)textData.fontType
      };
      renderData.uiTransforms[renderData.uiTransformCount++] = t;
      pos.x += glyph.advance.x * textData.fontSize;
    }
    
    // Add space after every word
    // lineWidth += wordWidth + spaceWidth;
    pos.x += spaceWidth;
  }
}