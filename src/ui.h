#pragma once
#include "sound.h"
#include "input.h"
#include "lib.h"
#include "assets.h"
#include "render_interface.h"

// #############################################################################
//                           UI Constants
// #############################################################################

// #############################################################################
//                           UI Structs
// #############################################################################
struct UIID
{
  int ID;
  int layer;

  operator bool()
  {
    return ID != 0;
  }

  bool operator==(const UIID other)
  {
    return ID == other.ID && layer == other.layer;
  }
  bool operator!=(const UIID other)
  {
    return !(*this == other);
  }
};

struct UIElement
{
  Vec2 pos;
  union
  {
    float scale;
    Vec2 size;
  };

  operator bool()
  {
    return pos && scale || pos && size;
  }
};

struct SavedUIElement
{
  int key;
  UIElement value;
};

struct UIState
{ 
  SavedUIElement* table = nullptr;

  SoundID hoverSound = SOUND_NONE;

  UIID hotLastFrame;
  UIID hotThisFrame;
  UIID active;
  UIID activatedThisFrame;
};

// #############################################################################
//                           UI Globals
// #############################################################################
static UIState* uiState;

// #############################################################################
//                           UI Helper Functions
// #############################################################################
void set_active(int ID)
{
  uiState->active = {ID, 0};
}

void set_hot(int ID, int layer = 0, const SoundID hoverSoundIn = SOUND_NONE)
{
  if(uiState->hotThisFrame.layer <= layer)
  {
    uiState->hotThisFrame.ID = ID;
    uiState->hotThisFrame.layer = layer;
    uiState->hoverSound = hoverSoundIn;
  }
}

bool is_active(int ID)
{
  return uiState->active.ID && (uiState->active.ID == ID);
}

bool is_hot(int ID)
{
  return uiState->hotLastFrame.ID && (uiState->hotLastFrame.ID == ID);
}

bool ui_is_hot()
{
  return uiState->hotLastFrame.ID || uiState->hotThisFrame.ID;
}

bool ui_is_active()
{
  return uiState->active.ID;
}

void update_ui()
{
  if(!is_down(ACTION_PRIMARY) && !just_released(ACTION_PRIMARY))
  {
    uiState->active = {};
  }

  if(uiState->hotLastFrame != uiState->hotThisFrame && uiState->hoverSound)
  {
    play_sound(uiState->hoverSound, {.flags = SOUND_OPTION_SFX | SOUND_OPTION_UI});
  }

  uiState->hotLastFrame = uiState->hotThisFrame;
  uiState->hotThisFrame = {};
  uiState->hoverSound = SOUND_NONE;
}

// #############################################################################
//                           Immediate Mode UI 
// #############################################################################
bool do_button(SpriteID spriteID, Vec2 pos, int ID, UIDrawData data = {})
{
  // #ifdef DEBUG
  // Get saves UIElement to reposition
  UIElement& e = hmget(uiState->table, ID);
  if(e)
  {
    pos = floor_vec2(e.pos);
  }
  else
  {
    hmput(uiState->table, ID, UIElement{.pos = pos});
  }
  // #endif
  const Vec2 anchorPos = floor_vec2(renderData->uiSpace/renderData->uiCam.zoom * data.anchor);

  if(is_down(ACTION_SCALE_DOWN))
  {
    char* test = format_text("%.0f, %.0f", pos.x, pos.y);
    draw_ui_text(test, anchorPos + pos);
  }

  // Move the button down if it's pressed

  if(is_active(ID) && e)
  {
    e.pos = input->mousePosUI - anchorPos;
    pos = floor_vec2(e.pos);
  }
  else
  {
    // if(is_active(ID))
    // {
    //   pos.y += 2;
    // }

    // if(is_hot(ID))
    // {
    //   data.drawData.scale = 1.1f;
    // }
  }

  draw_ui_sprite(spriteID, pos, data);

  const Sprite& sprite = SPRITES[spriteID];
  const Vec2 rectPos = anchorPos + pos;
  Rect buttonRect = {rectPos, sprite.size * data.drawData.scale};
  Vec2 mousePos = input->mousePosUI;

  if(is_active(ID))
  {
    // It was consume_just_released before because of split Render & update loops
    if(point_in_rect_center(mousePos, buttonRect) && consume_just_released(ACTION_PRIMARY))
    {
      play_sound(data.playSound, {.flags = SOUND_OPTION_SFX | SOUND_OPTION_UI});

      // if this isn't here then the button stutters when clicking
      set_hot(ID); 

      // Set inactive     
      uiState->active = {};
      return true;
    }
  }
  else if(is_hot(ID))
  {
    if(consume_just_pressed(ACTION_PRIMARY))
    {
      set_active(ID);
    }
  }

  if(point_in_rect_center(mousePos, buttonRect))
  {
    set_hot(ID, renderData->uiLayer, data.hoverSound);
  }
   
  return false;
}
