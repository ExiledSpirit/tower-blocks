#include "raylib.h"
#include "entity/block.h"
#include "entity/movement.h"
#include "game.h"
#include <vector>
#include <format>
#include <iostream>
#include <string>
#include <cmath>
#include <algorithm>
#include <raymath.h>

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 1000;
const Color BG_COLOR = (Color){.r = 210, .g = 200, .b = 190, .a = 255};
const int MOVEMENT_THRESHOLD = 16;
const float SCORE_ANIMATION_DURATION = 0.2;
const float SCORE_ANIMATION_SCALE = 1.5;

void DrawBlock(const entity::Block *block) {
  DrawCube(block->position, block->size.x, block->size.y, block->size.z, block->color);
  DrawCubeWires(block->position, block->size.x, block->size.y, block->size.z, BLACK);
}

void DrawCurrentBlock(Game *game) {
  if (game->state != PLAYING_STATE) {
    return;
  }

  DrawBlock(&game->current_block);
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
  game->placed_blocks.clear();
  game->current_block = default_block;
  game->current_block.color_offset = GetRandomValue(0, 100);

  game->placed_blocks.push_back(default_block);
  game->previous_block = &game->placed_blocks[0];
}

entity::Block CreateMovingBlock(Game *game) {
  entity::Block *target = game->previous_block;

  Axis axis = target->movement.axis == X ? Z : X;
  Direction direction = GetRandomValue(0, 1) == 0 ? FORWARD : BACKWARD;

  Vector3 position = target->position;
  position.y += target->size.y;

  if (axis == X) {
    position.x = (direction == FORWARD ? -1 : 1) * MOVEMENT_THRESHOLD;
  } else {
    position.z = (direction == FORWARD ? -1 : 1) * MOVEMENT_THRESHOLD;
  }

  size_t index = target->index + 1;
  int offset = target->color_offset + index;
  float r = sinf(0.3 * offset) * 55 + 200;
  float g = sinf(0.3 * offset + 2) * 55 + 200;
  float b = sinf(0.3 * offset + 4) * 55 + 200;

  return {
    index,
    position,
    target->size,
    (Color){ r, g, b, 255 },
    target->color_offset,
    (Movement){
      .speed = 16 + index * 0.5,
      .direction = direction,
      .axis = axis
    }
  };
}

void PlaceBlock(Game *game) {
  entity::Block *current = &game->current_block;
  entity::Block *target = game->previous_block;

  bool isXAxis = current->movement.axis == X;
  float currentPosition = isXAxis ? current->position.x : current->position.z;
  float targetPosition = isXAxis ? target->position.x : target->position.z;
  float currentSize = isXAxis ? current->size.x : current->size.z;
  float targetSize = isXAxis ? target->size.x : target->size.z;

  float delta = fabs(currentPosition) - fabs(targetPosition);
  float overlay = targetSize - fabs(delta);

  if (overlay < 0.1) {
    game->state = GAME_OVER_STATE;
    return;
  }

  bool isPerfectOverlay = fabs(delta) < 0.2;
  if (isPerfectOverlay) { // TODO: Add cool dopamine effect
    if (isXAxis) {
      current->size.x = target->size.x;
      current->position.x = target->position.x;
    } else {
      current->size.z = target->size.z;
      current->position.z = target->position.z;
    }
  }else { // TODO: Create a a piece of size equals the current piece cutoff size (old_size - overlay) and make it drop  
    int direction_factor = (current->movement.direction == FORWARD ? -1 : +1); 
    if (isXAxis) {
      current->size.x = overlay;
      current->position.x = (targetPosition) + (fabs(delta) / 2) * direction_factor;
    } else {
      current->size.z = overlay;
      current->position.z = (targetPosition) + (fabs(delta) / 2) * direction_factor;
    }
  }

  game->placed_blocks.push_back(game->current_block);
  game->previous_block = &game->placed_blocks.back();
  
  game->scoreAnimation.duration = SCORE_ANIMATION_DURATION;
  game->scoreAnimation.scale = SCORE_ANIMATION_SCALE;
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
      if (inputPressed) {
        PlaceBlock(game);
        game->current_block = CreateMovingBlock(game);
      }
      break;
    }

    case GAME_OVER_STATE: {
      if (inputPressed) {
        InitGame(game);
      }
      break;
    }
  }
}

void UpdateCameraPosition([[maybe_unused]]Camera3D *camera, Game *game, float dt) {
  size_t placed_blocks_len = game->placed_blocks.size();

  camera->position.y = Lerp(camera->position.y, 50 + (2 * placed_blocks_len), dt);
  camera->target.y = Lerp(camera->target.y, 2 * placed_blocks_len, dt);
}

void DrawGameStartOverlay(const Game *game) {
  if (game->state != READY_STATE) {
    return;
  }

  const char* title = "START GAME";
  int fontSize = 60;

  int screenWidth = GetScreenWidth();
  int textSize = MeasureText(title, fontSize);

  int position = (screenWidth - textSize) / 2;
  DrawText(title, position, 50, fontSize, DARKGRAY);
}

void DrawGameScore(Game *game) {
  if (game->state != PLAYING_STATE) {
    return;
  }

  size_t score = game->placed_blocks.size() - 1;
  const char* title = TextFormat("%zu", score);
  int fontSize = 120 * game->scoreAnimation.scale;

  int screenWidth = GetScreenWidth();
  int textSize = MeasureText(title, fontSize);

  int position = (screenWidth - textSize) / 2;
  DrawText(title, position, 200, fontSize, DARKGRAY);
}

void DrawGameOverOverlay(Game *game) {
  if (game->state != GAME_OVER_STATE) {
    return;
  }

  const char* title = "GAME OVER!";
  int fontSize = 60;

  int screenWidth = GetScreenWidth();
  int textSize = MeasureText(title, fontSize);

  int position = (screenWidth - textSize) / 2;
  DrawText(title, position, 50, fontSize, DARKGRAY);

  fontSize = 40;
  DrawText(TextFormat("SCORE: %zu", game->placed_blocks.size()), position, 100, fontSize, LIME);
}

void UpdateCurrentBlock(Game *game, float dt) {
  if (game->state != PLAYING_STATE) {
    return;
  }

  entity::Block *block = &game->current_block;
  float *axisPosition = block->movement.axis == X ? &block->position.x : &block->position.z;

  int direction = block->movement.direction == FORWARD ? 1 : -1;
  *axisPosition += direction * block->movement.speed * dt;

  if (fabs(*axisPosition) >= MOVEMENT_THRESHOLD) {
    block->movement.direction = block->movement.direction == FORWARD ? BACKWARD : FORWARD;
    *axisPosition = fmax(fmin(MOVEMENT_THRESHOLD, *axisPosition), -MOVEMENT_THRESHOLD);
  }
}

void UpdateScore(Game *game, float dt) {
  animations::ScoreAnimation *animation = &game->scoreAnimation;
  if (animation->duration > 0) {
    animation->duration -= dt;

    float t = 1 - animation->duration / SCORE_ANIMATION_DURATION;
    animation->scale = Lerp(SCORE_ANIMATION_SCALE, 1.0, t);

    if (animation->duration <= 0) {
      animation->duration = 0;
      animation->scale = 1;
    }
  }
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
    UpdateCurrentBlock(&game, dt);
    UpdateScore(&game, dt);

    BeginDrawing();
      ClearBackground(BG_COLOR);
      BeginMode3D(camera);
        DrawPlacedBlocks(&game);
        DrawCurrentBlock(&game);
      EndMode3D();
      DrawGameStartOverlay(&game);
      DrawGameScore(&game);
      DrawGameOverOverlay(&game);

      DrawFPS(10, 10);
    EndDrawing();
  }

  CloseWindow();
  return 1;
}
