#include "game.h"
#include "render_interface.h"

bool init_game()
{
  // Init Game Camera
  {
    renderData.gameCam.dimensions = vec_2(renderData.globalData.windowSize);
    renderData.gameCam.position = gameState.playerPos;
    renderData.gameCam.zoom = 4;
  }

  // Init UI Cams
  for(int i = 0; i < ArraySize(renderData.uiCams); i++)
  {
    const float uiScale = 1.0f;
    const Vec2 windowSize = vec_2(renderData.globalData.windowSize);
    OrthographicCamera2D& uiCam = renderData.uiCams[i];
    uiCam.dimensions = {ceilf(windowSize.x / uiScale), ceilf(windowSize.y / uiScale)};
    uiCam.position = {}; // Top Left
    uiCam.zoom = (float)(i+1);

    float zoom = uiCam.zoom? uiCam.zoom : 1.0f;
    Vec2 dimensions =  uiCam.dimensions / zoom;
    renderData.globalData.orthProjGame[i+ORTHO_PROJ_UI_1] = 
      orthographic_projection(0.0f, dimensions.x, 0.0f, -dimensions.y);
  }

  return true;
}

void update_game(const float dt)
{
  Vec2& playerPos = gameState.playerPos;
  const float playerSpeed = 100;

  if(just_pressed(gameState, ACTION_PRIMARY))
  {
    load_font(FONT_HEADING, "assets/fonts/alagard.ttf", 16, 1);
  }

  if(just_pressed(gameState, ACTION_LEFT))
  {
    load_font(FONT_HEADING, "assets/fonts/alagard.ttf", 16, 1);
  }

  // Update Player
  {
    if(is_down(gameState, ACTION_LEFT))
    {
      playerPos.x -= playerSpeed * dt;
    }
    if(is_down(gameState, ACTION_UP))
    {
      playerPos.y -= playerSpeed * dt;
    }
    if(is_down(gameState, ACTION_DOWN))
    {
      playerPos.y += playerSpeed * dt;
    }
    if(is_down(gameState, ACTION_RIGHT))
    {
      playerPos.x += playerSpeed * dt;
    }
  }	

  // Update Game Camera
  {
    renderData.gameCam.position = gameState.playerPos;

    const OrthographicCamera2D& gameCam = renderData.gameCam;
    float zoom = gameCam.zoom? gameCam.zoom : 1.0f;
    Vec2 dimensions =  gameCam.dimensions / zoom;
    Vec2 pos = gameCam.position;

    renderData.globalData.orthProjGame[ORTHO_PROJ_GAME] =
      orthographic_projection(pos.x-dimensions.x / 2.0f,
                              pos.x+dimensions.x / 2.0f,
                              -pos.y-dimensions.y / 2.0f,
                              -pos.y+dimensions.y / 2.0f);
  }

  float scale = sinf(gameState.gameTime) + 2;
  draw_sprite(SPRITE_SDV, {0, 0});
  draw_sprite(SPRITE_PLAYER, gameState.playerPos);

  // draw_ui_text("Penis, Cakez is a good programmer.", {10, 120}, {
  //   .fontType = FONT_TEXT
  // });
  // draw_ui_text("Penis, Cakez is a good programmer.", {10, 130}, {
  //   .fontType = FONT_HEADING
  // });
  // draw_ui_text("Penis, Cakez is a good programmer.", {10, 150}, {
  //   .fontType = FONT_HEADING,
  //   .baseFontSize = FONT_SIZE_4
  // });
  draw_ui_text("Penis, Cakez is a good programmer.", {10, 180}, {
    .fontType = FONT_HEADING,
    .baseFontSize = FONT_SIZE_1,
  });
  draw_ui_text("Penis, Cakez is a good programmer.", {10, 180}, {
    .fontType = FONT_HEADING,
    .baseFontSize = FONT_SIZE_2,
  });
  draw_ui_text("Penis, Cakez is a good programmer.", {10, 180}, {
    .fontType = FONT_HEADING,
    .baseFontSize = FONT_SIZE_3,
  });
  // draw_sprite(SPRITE_PINE_TREE, {100, 200});
  // draw_sprite(SPRITE_PINE_TREE, {-300, 240});
  // draw_sprite(SPRITE_PINE_TREE, {160, 280});
  // draw_sprite(SPRITE_PINE_TREE, {-100, 220});
  // draw_sprite(SPRITE_PINE_TREE, {200, 150});

  draw_light(gameState.playerPos);
}