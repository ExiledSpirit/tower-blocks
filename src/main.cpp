#include "raylib.h"
#include "entity/block.h"
#include "game.h"
#include <vector>
#include <format>
#include <iostream>
#include <string>

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 1000;
const Color BG_COLOR = (Color){.r = 210, .g = 200, .b = 190, .a = 255};

void DrawBlock(const entity::Block *block) {
  DrawCube(block->position, block->size.x, block->size.y, block->size.z, block->color);
  DrawCubeWires(block->position, block->size.x, block->size.y, block->size.z, BLACK);
}

void DrawCurrentBlock(Game *game) {
  if (!game->state != PLAYING_STATE) {
    return;
  }
}

void DrawPlacedBlocks(Game *game) {
  std::vector blocks = game->placed_blocks;

  for (size_t i = 0; i < blocks.size(); i++) {
    entity::Block *block = &blocks[i];
    DrawBlock(block);
  }
}

entity::Block default_block;

void InitGame(Game *game) {
  game->state = READY_STATE;
  game->current_block = default_block;

  game->placed_blocks.push_back(default_block);
  game->previous_block = &game->placed_blocks[0];
}

entity::Block CreateMovingBlock(Game *game) {
  entity::Block *target = game->previous_block;

  return {
    target->position,
    target->size,
    target->color,
    target->movement
  };
}

void UpdateGameState(Game *game) {
  bool inputPressed = IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

  switch (game->state) {
    case READY_STATE: {
      if (inputPressed) {
        game->state = PLAYING_STATE;
        game->current_block = CreateMovingBlock(game);
      }
      break;
    }

    case PLAYING_STATE: {
      break;
    }

    case GAME_OVER_STATE: {
      break;
    }
  }
}

float Lerp(float a, float b, float t) {
  return a + (b - a) * t;
}

void UpdateCameraPosition([[maybe_unused]]Camera3D *camera, Game *game, float dt) {
  size_t placed_blocks_len = game->placed_blocks.size();

  camera->position.y = Lerp(camera->position.y, 50 + (2 * placed_blocks_len), dt);
  camera->target.y = Lerp(camera->target.y, 2 * placed_blocks_len, dt);
}

int main() {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tower Blocks");
  SetTargetFPS(120);

  Camera3D camera = (Camera3D) {
    .position = (Vector3) { .x = 50, .y = 50, .z = 50 },
    .target = (Vector3) { .x = 0, .y = 0, .z = 0 },
    .up = (Vector3) { .x = 0, .y = 1, .z = 0 },
    .fovy = 60,
    .projection = CAMERA_ORTHOGRAPHIC
  };

  entity::Block block;
  Game game = Game();
  InitGame(&game);

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();
    UpdateGameState(&game);
    UpdateCameraPosition(&camera, &game, dt);

    BeginDrawing();
      ClearBackground(BG_COLOR);
      BeginMode3D(camera);
        DrawPlacedBlocks(&game);
        DrawCurrentBlock(&game);
      EndMode3D();
      DrawText(TextFormat("%zu blocks", game.placed_blocks.size()), 20, 20, 20, BLACK);
    EndDrawing();
  }

  CloseWindow();
  return 1;
}
