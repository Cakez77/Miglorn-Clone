#include "game.h"
#include "render_interface.h"


EXPORT_FN void update_game(GameState* gameStateIn, RenderData* renderDataIn, const float dt)
{
  if(gameStateIn != gameState ||
     renderDataIn != renderData)
  {
    gameState = gameStateIn;
    uiState = &gameState->uiState;
    renderData = renderDataIn;
    input = &gameState->input;
  }

  if(!gameState->initialized)
  {
    // Setup Game Camera init game cam init_game_cam
    {
      renderData->gameCam.dimensions = vec_2(renderData->globalData.windowSize);
      renderData->gameCam.position = gameState->playerPos;
      renderData->gameCam.zoom = 4;
    }
  }

  Vec2& playerPos = gameState->playerPos;
  const float playerSpeed = 100;

  // Update Player
  {
    if(is_down(ACTION_LEFT))
    {
      playerPos.x -= playerSpeed * dt;
    }
    if(is_down(ACTION_UP))
    {
      playerPos.y -= playerSpeed * dt;
    }
    if(is_down(ACTION_DOWN))
    {
      playerPos.y += playerSpeed * dt;
    }
    if(is_down(ACTION_RIGHT))
    {
      playerPos.x += playerSpeed * dt;
    }
  }	

  float scale = sinf(gameState->gameTime) + 2;
  draw_sprite(SPRITE_SDV, {0, 0});

  const float targetX = -48;
  static float timer2 = 0;
  static float startX = 32;
  timer2 += dt;
  const float duration = 0.8f;
  timer2 = fmodf(timer2, duration);

  const float t = ease_out_back(timer2 * 2/duration);
  const float xPos = lerp_no_cap(startX, targetX, t);

  // draw_ui_sprite(SPRITE_BUTTON, {xPos, 64}, {.anchor = {1}});
  // do_button(SPRITE_BUTTON, {xPos, 64}, line_id(1), {.drawData.anchor = {1}});
  char* mousePosScreen = format_text("Mouse Pos Window: %.2f, %.2f", input->mousePosScreen.x, input->mousePosScreen.y);
  char* mousePosText = format_text("Mouse Pos: %.2f, %.2f", input->mousePosUI.x, input->mousePosUI.y);
  // draw_ui_text(mousePosScreen, {4,0}, {.anchor = {0, 0.5}});
  // draw_ui_text(mousePosText, {4,10}, {.anchor = {0, 0.5}});

  // do_button(SPRITE_UI_BACKGROUND, {5, 0}, line_id(1), {.anchor = {0, 1}});
  // renderData->uiLayer += 1;
  // do_button(SPRITE_BUTTON, {10, 0}, line_id(1), {.anchor = {1, 0.5}});
  // do_button(SPRITE_BUTTON, {10, 0}, line_id(1), {.anchor = {0, 1}});
  // do_button(SPRITE_BUTTON, {10, 0}, line_id(1), {.anchor = {0.5, 0.5}});
  // renderData->uiLayer -= 1;

  // Update Game Camera
  {
    renderData->gameCam.position = gameState->playerPos;

    const OrthographicCamera2D& gameCam = renderData->gameCam;
    float zoom = gameCam.zoom? gameCam.zoom : 1.0f;
    Vec2 dimensions =  gameCam.dimensions / zoom;
    Vec2 pos = gameCam.position;

    renderData->globalData.orthProjGame[ORTHO_PROJ_GAME] =
      orthographic_projection(pos.x-dimensions.x / 2.0f,
                              pos.x+dimensions.x / 2.0f,
                              -pos.y-dimensions.y / 2.0f,
                              -pos.y+dimensions.y / 2.0f);
  }

  draw_sprite(SPRITE_PLAYER, gameState->playerPos);
  const Vec2 TREES[] = 
  {
    {10, 20},
    {-20, 40},
    {-10, -23},
    {42, -38},
    {67, 76},
  };

  struct TestStuff
  {
    Vec2 v1;
    Vec2 v2;
    float angle1;
    float angle2;
  };

  Array<TestStuff, 100> sortedWalls = {};
  const float lightRadius = 256;

  renderData->globalData.lightObstacleCount = 0;
  for(int i = 0; i < ArraySize(TREES); i++)
  {
    // Make sure to never get ARRAY FULL!
    if(renderData->globalData.lightObstacleCount > ArraySize(renderData->globalData.obstacles))
    {
      break;
    }

    const Vec2 pos = TREES[i];
    draw_sprite(SPRITE_PINE_TREE, pos);

    Vec2 direction = pos - playerPos;
    // direction.y *= 1.0f / HEIGHT_FACTOR_ELLIPSE;

    float distance = length(direction);
    Vec2 dirNorm = normalize(direction);
    const float distanceSquared = length_squared(direction);

    const float colliderRadius = 6;
    const float radiusSquared = lightRadius * lightRadius;
    if(distanceSquared < radiusSquared)
    {
      Vec2 v1 = playerPos + direction + normal_from_vec_top(dirNorm) * colliderRadius;
      Vec2 v2 = playerPos + direction - normal_from_vec_top(dirNorm) * colliderRadius;
      float angle = get_angle_correct(direction);

      // These are in radians
      float angle1 = get_angle_correct(direction + normal_from_vec_top(dirNorm)*colliderRadius);
      float angle2 = get_angle_correct(direction - normal_from_vec_top(dirNorm)*colliderRadius);
      float deg1 = (angle1 + PI) * 180/PI;
      float deg2 = (angle2 + PI) * 180/PI;

      if(deg1 < deg2)
      {
        sortedWalls.add({.v1 = v1, .v2 = v2, .angle1 = deg1, .angle2 = deg2});
      }
      else
      {
        sortedWalls.add({.v1 = v2, .v2 = v1, .angle1 = deg2, .angle2 = deg1});

      }

      LightObstacle o = {};
      o.dist = distanceSquared / radiusSquared *4;
      o.startDeg = deg1;
      o.endDeg = deg2;
      renderData->globalData.obstacles[renderData->globalData.lightObstacleCount++] = o;
    }
  }

  // 32x32 * 8 -> l
  draw_light(gameState->playerPos, 8);

  const float left = playerPos.x - lightRadius;
  const float right = playerPos.x + lightRadius;
  const float top = playerPos.y - lightRadius;
  const float bottom = playerPos.y + lightRadius;

  // Sort walls & build Vertices
  {
    qsort(sortedWalls.elements, sortedWalls.count,
      sizeof(TestStuff),
      [](const void* a, const void* b)->int
      {
        // a - b <=, b - a >=
        float value = ((const TestStuff*)a)->angle1 - ((const TestStuff*)b)->angle1;
        return value < 0? -1 : (value > 0? 1 : 0);
      });
  }


  // 
  if(false)
  {
    float prevAngle = 0;
    Vec2 prevV = {};

    int count = 0;
    for(TestStuff& test : sortedWalls)
    {
      const float nextAngle = test.angle1;

      if(nextAngle)

      renderData->vertices[renderData->vertexCount++] = Vertex{gameState->playerPos};
      renderData->vertices[renderData->vertexCount++] = Vertex{test.v1};
      renderData->vertices[renderData->vertexCount++] = Vertex{test.v2};

      // We have a gap
      const float epsilon = 1.0f; // Pixel gap
      if(prevAngle && nextAngle > (prevAngle - epsilon))
      {
        const int iteration = 2;
        if(count >= iteration && count < iteration +1 )
        {
          const float len = 0.8f;
          const Vec2 f1 = gameState->playerPos + normalize(prevV - gameState->playerPos) * lightRadius * len;
          const Vec2 f2 = gameState->playerPos + normalize(test.v1 - gameState->playerPos) * lightRadius * len;
          renderData->vertices[renderData->vertexCount++] = Vertex{gameState->playerPos};
          renderData->vertices[renderData->vertexCount++] = Vertex{f1};
          renderData->vertices[renderData->vertexCount++] = Vertex{f2};

        }
      }

      if(count == 3)
      {
        draw_sprite(SPRITE_WHITE, test.v1, Vec2{4,4});
      }

      count++;

      prevAngle = test.angle2;
      prevV = test.v2;
    }
  }

  if(just_pressed(ACTION_SCALE_DOWN))
  {
    // 5% ui Scale
    renderData->uiScale = max(1.0f, renderData->uiScale - 0.05f);
    const Vec2& windowSize = vec_2(renderData->globalData.windowSize);
    const float uiScale = renderData->uiScale;
    renderData->uiSpace = Vec2{ceilf(windowSize.x / uiScale), ceilf(windowSize.y / uiScale)};
  }

  if(just_pressed(ACTION_SCALE_UP))
  {
    // 5% ui Scale
    renderData->uiScale = min(2.0f, renderData->uiScale + 0.05f);
    const Vec2& windowSize = vec_2(renderData->globalData.windowSize);
    const float uiScale = renderData->uiScale;
    renderData->uiSpace = Vec2{ceilf(windowSize.x / uiScale), ceilf(windowSize.y / uiScale)};
  }

  // char* uiScaleText = format_text("UI Scale: %.2f", renderData->uiScale);
  // draw_ui_text(uiScaleText, {8, 0}, {.anchor = {0, 0.5f}});

  // draw_ui_text("Penis, Cakez is a good programmer.", {2, 10}, {
  //   .fontType = FONT_TEXT_1X,
  // });
  // draw_ui_text("Penis, Cakez is a good programmer.", {2, 0}, {
  //   .fontType = FONT_TEXT_2X,
  //   .anchor = {0, 0.5f}
  // });
  // draw_ui_text("Penis, Cakez is a good programmer.", {2, 10}, {
  //   .fontType = FONT_TEXT_3X,
  //   .anchor = {0.5}
  // });

  // draw_sprite(SPRITE_PINE_TREE, {100, 200});
  // draw_sprite(SPRITE_PINE_TREE, {-300, 240});
  // draw_sprite(SPRITE_PINE_TREE, {160, 280});
  // draw_sprite(SPRITE_PINE_TREE, {-100, 220});
  // draw_sprite(SPRITE_PINE_TREE, {200, 150});

  // draw_sprite(SPRITE_WHITE, gameState->playerPos, {160, 160}, {
  //   .renderOptions = RENDER_OPTION_LIGHT_RAYMARCH});
  // draw_sprite(SPRITE_WHITE, gameState->playerPos, {16, 16}, {
  //   .scale = 10,
  //   .renderOptions = RENDER_OPTION_LIGHT_RAYMARCH});
}