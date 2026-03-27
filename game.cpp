#include "lib.h"
#include "assets.h"

#include "raylib.h"
#include "raymath.h"
#include "render_interface.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

const Vector2 WORLD_SIZE = {480, 272};

#define WINDOW_TITLE "Window title"

// void draw_text_centered(const char* text, Vector2 pos, float fontSize)
// {
// 	const Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, 1);
//     pos.x -= textSize.x / 2.0f;
// 	DrawText(text, pos.x, pos.y, fontSize, WHITE);
// }

enum GameAction
{
    ACTION_PRIMARY,
    ACTION_SECONDARY,
    ACTION_LEFT,
    ACTION_UP,
    ACTION_DOWN,
    ACTION_RIGHT,

    ACTION_COUNT
};

struct ActionToKey
{
    union
    {
        KeyboardKey key;
        MouseButton mouseBtn;
    };
    bool down;
    int halfTransitionCount;
};

struct GameOptions
{
    ActionToKey keybinds[ACTION_COUNT] = 
    {
        [ACTION_PRIMARY] = {.mouseBtn = MOUSE_LEFT_BUTTON},
        [ACTION_SECONDARY] = {.mouseBtn = MOUSE_RIGHT_BUTTON},
        [ACTION_LEFT] = {.key = KEY_A},
        [ACTION_UP] = {.key = KEY_W},
        [ACTION_DOWN] = {.key = KEY_S},
        [ACTION_RIGHT] = {.key = KEY_D},
    };
};

struct GameState
{
    GameOptions options;
    Vector2 playerPos = WORLD_SIZE;
    Vector2 playerVel = {};
    Camera2D playerCam = {};
};

bool is_down(const GameState& gameState, GameAction gameAction)
{
    bool isDown = IsKeyDown(gameState.options.keybinds[gameAction].key);
    return isDown;
}

bool just_pressed(const GameState& gameState, GameAction gameAction)
{
    bool isDown = IsKeyPressed(gameState.options.keybinds[gameAction].key);
    return isDown;
}

bool just_released(const GameState& gameState, GameAction gameAction)
{
    bool isDown = IsKeyReleased(gameState.options.keybinds[gameAction].key);
    return isDown;
}

void test_hashmap()
{
    struct Glyph
    {
        int codePoint;
        int height;
        int width;
        int advance;

        operator bool()
        {
            return codePoint != 0 && 
                height != 0 &&
                width != 0 &&
                advance != 0;
        }
    };

    struct GlyphToCodepoint
    {
        int key; // 0 - 1mil
        Glyph value;
    };

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
        test.codePoint = 10;
    }
    testRef.advance = 10'000.0f;
}

int main(void)
{
    // Init game
    GameState gameState = {};

    InitWindow(WORLD_SIZE.x*2, WORLD_SIZE.y*2, WINDOW_TITLE);
    // Move window to my other screen, just for me
    SetWindowPosition(-1860, 100); 
    SetTargetFPS(60);

    // Texture loading
    atlas = LoadTexture("assets/textures/atlas.png");
    SetTextureFilter(atlas, TEXTURE_FILTER_POINT);
    Vec2& playerPos = gameState.playerPos;
    playerPos = WORLD_SIZE + Vec2{0, 100};
    Vec2& playerVel = gameState.playerVel;
    Vec2 pos1 = playerPos + Vec2{100, - 120};
    Vec2 pos2 = playerPos + Vec2{-100};
    Vec2 pos3 = playerPos + Vec2{10, 20};
    gameState.playerCam.zoom = 2;
    gameState.playerCam.rotation = 0;
    gameState.playerCam.offset = Vec2{(float)GetScreenWidth()*0.5f, (float)GetScreenHeight()*0.5f};
    static Vec2 camPos = playerPos;

    while (!WindowShouldClose())
    {
        const float dt = GetFrameTime();

        BeginDrawing();
        ClearBackground(BLACK);

        // Update cam after player moved and before he is drawn
        const float dist = Vector2Distance(camPos, playerPos);
        camPos = Vector2Lerp(camPos, playerPos, dt * dist);
        gameState.playerCam.target = camPos;
        BeginMode2D(gameState.playerCam);
        DrawRectangle(-2000, -2000, 4000, 4000, DARKGREEN);

        // Update and Draw Game
        {
            Vec2 prevVel = playerVel;
            // Move Player
            {
                playerVel = {};
                if(is_down(gameState, ACTION_LEFT))
                {
                    playerVel.x = -1;
                }
                if(is_down(gameState, ACTION_UP))
                {
                    playerVel.y = -1;
                }
                if(is_down(gameState, ACTION_DOWN))
                {
                    playerVel.y = 1;
                }
                if(is_down(gameState, ACTION_RIGHT))
                {
                    playerVel.x = 1;
                }
            }

            const float maxSpeed = 200;
            const float acc = 10;
            static float speed = 0;
            // if(prevVel.x < 0 && playerVel.x > 0 ||
            //    prevVel.y < 0 && playerVel.y > 0 ||
            //    prevVel.x > 0 && playerVel.x < 0 ||
            //    prevVel.y > 0 && playerVel.y < 0)
            // {
            //     speed = maxSpeed/2;
            // }

            if(playerVel != VEC2_ZERO)
            {
                playerVel = Vector2Normalize(playerVel);
                speed = approach(speed, maxSpeed, acc);
            }
            else
            {
                speed = 0;
            }
            const float moveDist = speed * dt;
            playerPos += playerVel * moveDist;
            draw_sprite(SPRITE_PLAYER, playerPos);
        }

        // Draw Foliage
        draw_sprite(SPRITE_PINE_TREE, pos1);
        draw_sprite(SPRITE_PINE_TREE, pos2);
        draw_sprite(SPRITE_PINE_TREE, pos3);
        draw_sprite(SPRITE_VIKING_HOUSE, WORLD_SIZE);


        EndMode2D();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}