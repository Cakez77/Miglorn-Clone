#pragma once
#include "lib.h"
#include "assets.h"
#include "input.h"
#include "sound.h"

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
constexpr u32 MAX_VERTICES = 200;
const u32 FONT_ATLAS_SIZE = 1024;

enum FontType
{
  FONT_HEADING_1X,
  FONT_HEADING_2X,
  FONT_HEADING_3X,
  FONT_TEXT_1X,
  FONT_TEXT_2X,
  FONT_TEXT_3X,

  FONT_COUNT
};

// #############################################################################
//                           Render Structs
// #############################################################################
enum OthoProjectionType
{
  ORTHO_PROJ_GAME,    // Scaled by 4
  ORTHO_PROJ_LIGHTS,  // Scaled by Game - 1
  ORTHO_PROJ_UI,      // Scaled by 4 (same as game)
  ORTHO_PROJ_TEXT,    // Scaled by 1
  ORTHO_PROJ_COUNT
};

struct OrthographicCamera2D
{
  float zoom = 1.0f;
  Vec2 dimensions;
  Vec2 position;
};

struct Vertex
{
  Vec2 pos;
};

struct Transform
{
  i32 atlasPosPacked;
  i32 spriteSize;
  Vec2 pos;
  Vec2 size;
  int pack1; // [renderOptions - 24 Bits][fontIdx - 4 Bits][matrixIdx - 4 Bits]
  float layer;
};
static_assert(sizeof(Transform) % 16 == 0, "Transform is not 16 Byte aligned");

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

struct LightObstacle
{
  float startDeg;
  float endDeg;
  float dist;
  float padding;
};

struct GlobalData
{
  float gameTime;
  int lightObstacleCount = 0;
  IVec2 windowSize;
  Vec2 camPos;
  Vec2 padding;
  Mat4 orthProjGame[ORTHO_PROJ_COUNT];
  LightObstacle obstacles[10];
};
static_assert(sizeof(GlobalData) % 16 == 0, "GlobalData is not 16 Byte aligned");

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
  OrthographicCamera2D textCam;
  GlobalData globalData;

  float uiLayer = 0;
  float uiScale = 1.0f;
  float gameScale = 1.0f;

  Vec2 viewportOffset;
  Vec2 viewportSize;

  Vec2 uiSpace;
  Font fonts[FONT_COUNT];

  int vertexCount = 0;
  Vertex vertices[MAX_VERTICES];

  int transformCount = 0;
  Transform transforms[MAX_TRANSFORMS];

  int uiTransformCount = 0;
  Transform uiTransforms[MAX_TRANSFORMS];

  int textTransformCount = 0;
  Transform textTransforms[MAX_TRANSFORMS];

  int lightCount = 0;
  Transform lights[100];
};

struct TransformData
{
  Vec2 pos;
  Vec2 size;
  float layer;
  int matIdx;
  int fontIdx;
  int renderOptions;
};

struct DrawData
{
  float scale = 1;
  float layer = 0;
  int renderOptions = 0;
};

// #############################################################################
//                           Render Globals
// #############################################################################
static RenderData* renderData;

// #############################################################################
//                           Render Camera Utility
// #############################################################################
Vec2 window_to_ui(Vec2 screenPos)
{
  const OrthographicCamera2D& camera = renderData->uiCam;

  float xPos = (screenPos.x - renderData->viewportOffset.x) / 
               renderData->viewportSize.x * camera.dimensions.x; // [0; dimensions.x]

  float yPos = (screenPos.y - renderData->viewportOffset.y) / 
               renderData->viewportSize.y * camera.dimensions.y; // [0; dimensions.y]

  return Vec2{xPos, yPos} / renderData->uiCam.zoom;
}

// #############################################################################
//                           Render Helper Functions
// #############################################################################
i32 pack_int(const IVec2& v)
{
  return v.x + (v.y << 16);
}

i32 pack_int(const Vec2& v)
{
  return (int)v.x + ((int)v.y << 16);
}

Transform get_transform(Vec2 atlasOffset, Vec2 spriteSize, const TransformData& data = {})
{
  Transform t = 
  {
    .atlasPosPacked = pack_int(atlasOffset),
    .spriteSize = pack_int(spriteSize),
    .pos = data.pos,
    .size = data.size,
    .pack1 = data.matIdx + (data.fontIdx << 4) + (data.renderOptions << 8),
    .layer = data.layer,
  };

  return t;
}

Transform get_transform(SpriteID spriteID, const TransformData& data = {})
{
  const Sprite& sprite = SPRITES[spriteID];
  return get_transform(sprite.atlasOffset, sprite.size, data);
}

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
  Font& font = renderData->fonts[fontType];
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
//                           Render Functions Game
// #############################################################################
void draw_sprite(const SpriteID spriteID, Vec2 pos, Vec2 size, const DrawData& drawData = {})
{
  // Early return if full
  if(renderData->transformCount >= ArraySize(renderData->transforms))
  {
    return;
  }

  float layer = pos.y + 100'000'000.0f; // To handle negative y pos
  const Sprite& sprite = SPRITES[spriteID];
  pos = pos - (size/2 - sprite.pivotOffset) * drawData.scale;

  Transform t = get_transform(spriteID, {
    .pos = pos,
    .size = size * drawData.scale,
    .layer = layer,
    .matIdx = ORTHO_PROJ_GAME,
    .renderOptions = drawData.renderOptions});
  renderData->transforms[renderData->transformCount++] = t;
}

void draw_sprite(const SpriteID spriteID, Vec2 pos, const DrawData& drawData = {})
{
  const Sprite& sprite = SPRITES[spriteID];
  draw_sprite(spriteID, pos, sprite.size, drawData);
}

enum RenderOptions
{
  RENDER_OPTION_LIGHT_RAYMARCH = BIT(0),
};

void draw_light(Vec2 pos, float scale = 1)
{
  // Early return in case the array is full
  if(renderData->lightCount >= ArraySize(renderData->lights))
  {
    return;
  }

  const Sprite& sprite = SPRITES[SPRITE_LIGHT];
  pos = pos - (sprite.size/2 - sprite.pivotOffset) * scale;
  Transform t = get_transform(SPRITE_LIGHT, {
    .pos = pos,
    .size = sprite.size * scale,
    .matIdx = ORTHO_PROJ_GAME,
    .renderOptions = RENDER_OPTION_LIGHT_RAYMARCH});
  renderData->lights[renderData->lightCount++] = t;
}

// #############################################################################
//                           Render Functions UI
// #############################################################################
struct UIDrawData
{
  SoundID playSound = SOUND_NONE;
  SoundID hoverSound = SOUND_NONE;
  Vec2 anchor;
  DrawData drawData;
};

void draw_ui_sprite(const SpriteID spriteID, Vec2 pos, Vec2 size, const UIDrawData& uiData = {})
{
  // Early return in case the array is full
  if(renderData->uiTransformCount >= ArraySize(renderData->uiTransforms))
  {
    return;
  }

  const Sprite& sprite = SPRITES[spriteID];
  pos = floor_vec2(renderData->uiSpace/renderData->uiCam.zoom * uiData.anchor) + 
    (pos - (size/2 - sprite.pivotOffset) * uiData.drawData.scale);

  Transform t = get_transform(spriteID, {
    .pos = pos,
    .size = size * uiData.drawData.scale,
    .layer = renderData->uiLayer,
    .matIdx = ORTHO_PROJ_UI,
    .renderOptions = uiData.drawData.renderOptions});
  renderData->uiTransforms[renderData->uiTransformCount++] = t;
}

void draw_ui_sprite(const SpriteID spriteID, Vec2 pos, const UIDrawData& uiData = {})
{
  const Sprite& sprite = SPRITES[spriteID];
  draw_ui_sprite(spriteID, pos, sprite.size, uiData);
}

// #############################################################################
//                           Render Functions Text
// #############################################################################
struct TextData
{
  FontType fontType = FONT_TEXT_2X;
  Vec2 anchor;
  float fontSize = 1.0f;
  float rotation = 0;
  int renderOptions;
  // todo: function over time like sinf
  // todo: individual scaling
};

void draw_ui_text(char* text, Vec2 pos, const TextData& textData = {})
{
  pos = floor_vec2(renderData->uiSpace * textData.anchor) + pos * renderData->uiCam.zoom;
  const Font& font = renderData->fonts[textData.fontType];
  const float spaceWidth = get_glyph_if_exists(font, ' ').advance.x * textData.fontSize;
  const float fontHeight = font.height * textData.fontSize;
  const Words words = split_words(text);
  pos.y -= fontHeight/2;
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

      // Return if array got full while drawing text
      if(renderData->textTransformCount >= ArraySize(renderData->textTransforms))
      {
        return;
      }

      // Draw the Glyph
      const Glyph& glyph = get_glyph_if_exists(font, codepoint);
      Transform t = get_transform(glyph.textureCoords, glyph.size, {
        .pos = pos - glyph.offset * textData.fontSize,
        .size = glyph.size * textData.fontSize,
        .layer = renderData->uiLayer,
        .matIdx = ORTHO_PROJ_TEXT,
        .fontIdx = (int)textData.fontType,
        .renderOptions = textData.renderOptions});
      renderData->textTransforms[renderData->textTransformCount++] = t;

      pos.x += glyph.advance.x * textData.fontSize;
    }
    
    // Add space after every word
    // lineWidth += wordWidth + spaceWidth;
    pos.x += spaceWidth;
  }
}