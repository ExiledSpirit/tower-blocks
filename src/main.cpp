#include "raylib.h"
#include "entity/block.h"
#include "entity/falling_block.h"
#include "entity/movement.h"
#include "game.h"
#include <vector>
#include <format>
#include <iostream>
#include <string>
#include <cmath>
#include <algorithm>
#include <raymath.h>
#include <external/reasings.h>
#include <rlgl.h>

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 1000;
const Color BG_COLOR = (Color){.r = 210, .g = 200, .b = 190, .a = 255};
const int MOVEMENT_THRESHOLD = 16;

const float SCORE_ANIMATION_DURATION = 0.2;
const float SCORE_ANIMATION_SCALE = 1.5;

const int OVERLAY_ANIMATION_OFFSET_Y = -50;

void DrawBlock(const entity::Block *block, Shader lightingShader) {
  Vector4 normalizedColor = ColorNormalize(block->color);
  Vector3 normalizedColorVec3 = {.x=normalizedColor.x, .y=normalizedColor.y, .z=normalizedColor.z};
  SetShaderValue(lightingShader, GetShaderLocation(lightingShader, "blockColor"), &normalizedColorVec3, SHADER_UNIFORM_VEC3);
  BeginShaderMode(lightingShader);
    DrawCube(block->position, block->size.x, block->size.y, block->size.z, block->color);
  EndShaderMode();
}

void DrawCurrentBlock(Game *game) {
  if (game->state != PLAYING_STATE) {
    return;
  }

  DrawBlock(&game->current_block, game->lighting_shader);
}

void DrawPlacedBlocks(Game *game) {
  std::vector blocks = game->placed_blocks;

  for (size_t i = 0; i < blocks.size(); i++) {
    entity::Block *block = &blocks[i];
    DrawBlock(block, game->lighting_shader);
  }
}

entity::Block default_block;

void InitGame(Game *game) {
  game->lighting_shader = LoadShader("shaders/lighting_vertex.glsl", "shaders/lighting_fragment.glsl");

  game->cube_model = LoadModelFromMesh(GenMeshCube(1, 1, 1));
  game->state = READY_STATE;
  game->placed_blocks.clear();
  game->falling_blocks.clear();
  game->current_block = default_block;
  game->current_block.color_offset = GetRandomValue(0, 100);

  game->scoreAnimation.duration = SCORE_ANIMATION_DURATION;
  game->scoreAnimation.scale = SCORE_ANIMATION_SCALE;

  game->overlayAnimation = {
    .type = animations::START_GAME_OVERLAY,
    .fade = animations::FADING_IN,
    .alpha = 0,
    .offsetY = OVERLAY_ANIMATION_OFFSET_Y
  };

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

entity::FallingBlock CreateFallingBlock(Vector3 position, Vector3 size, Color color) {
  return (entity::FallingBlock) {
    position,
    size,
    { 0 },
    {
      .x = GetRandomValue(-300, 300) / 100.f,
      .y = GetRandomValue(-300, 300) / 100.f,
      .z = GetRandomValue(-300, 300) / 100.f,
    },
    { 0, -12, 0 },
    color,
    true
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

  float delta = currentPosition - targetPosition;
  float overlay = targetSize - fabs(delta);

  if (overlay < 0.1) {
    game->state = GAME_OVER_STATE;
    return;
  }

  bool isPerfectOverlay = fabs(delta) < 0.3;
  if (isPerfectOverlay) { // TODO: Add cool dopamine effect
    if (isXAxis) {
      current->size.x = target->size.x;
      current->position.x = target->position.x;
    } else {
      current->size.z = target->size.z;
      current->position.z = target->position.z;
    }
  }else {
    if (isXAxis) {
      current->size.x = overlay;
      current->position.x = (targetPosition) + (delta / 2);
    } else {
      current->size.z = overlay;
      current->position.z = (targetPosition) + (delta / 2);
    }

    float choppedSize = currentSize - overlay;
    if (choppedSize > 0.1) {
      Vector3 choppedPositionVec = current->position;
      Vector3 choppedSizeVec = current->size;

      if (isXAxis) {
        choppedSizeVec.x = choppedSize;
        if (delta > 0) {
          choppedPositionVec.x = currentPosition + currentSize / 2 - choppedSize / 2;
        } else {
          choppedPositionVec.x = currentPosition - currentSize / 2 + choppedSize / 2;
        }
      } else {
        choppedSizeVec.z = choppedSize;
        if (delta > 0) {
          choppedPositionVec.z = currentPosition + currentSize / 2 - choppedSize / 2;
        } else {
          choppedPositionVec.z = currentPosition - currentSize / 2 + choppedSize / 2;
        }
      }
      game->falling_blocks.push_back(CreateFallingBlock(choppedPositionVec, choppedSizeVec, current->color));
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
        game->overlayAnimation.fade = animations::FADING_OUT;
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
      game->overlayAnimation.fade = animations::FADING_IN;
      game->overlayAnimation.type = animations::GAME_OVER_OVERLAY;
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

void DrawOverlay([[maybe_unsused]]const Game *game, const char *title, const char *subtitle, size_t titleSize, size_t subtitleSize, int titleY, int subtitleY) {
  Color dark = Fade(DARKGRAY, game->overlayAnimation.alpha);
  Color light = Fade(GRAY, game->overlayAnimation.alpha);

  int screenWidth = GetScreenWidth();
  int titleWidth = MeasureText(title, titleSize);
  int subtitleWidth = MeasureText(subtitle, subtitleSize);

  DrawText(title, (screenWidth - titleWidth) / 2, titleY + game->overlayAnimation.offsetY, titleSize, dark);
  DrawText(subtitle, (screenWidth - subtitleWidth) / 2, subtitleY + game->overlayAnimation.offsetY, subtitleSize, light);
}

void DrawGameStartOverlay(const Game *game) {
  if (game->overlayAnimation.type != animations::START_GAME_OVERLAY) {
    return;
  }
  
  DrawOverlay(game, "START GAME", "Click or Press Space", 60, 30, 100, 170);
}

void DrawGameOverOverlay(Game *game) {
  if (game->overlayAnimation.type != animations::GAME_OVER_OVERLAY) {
    return;
  }

  DrawOverlay(game, "GAME OVER", "Click or Press Space", 60, 30, 100, 170);
}

void DrawGameScore(Game *game) {
  if (game->state == READY_STATE) {
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

const float FADE_SPEED = 2.5;

void UpdateOverlay(Game *game, float dt) {
  animations::OverlayAnimation *animation = &game->overlayAnimation;

  if (animation->fade == animations::FADING_IN) {
    animation->alpha += dt * FADE_SPEED;
    // Ease for smooth start/stop
    float t = EaseSineInOut(dt, 0.0f, 1.0f, FADE_SPEED);
    animation->offsetY = Lerp(OVERLAY_ANIMATION_OFFSET_Y, 0, animation->alpha);

    if (animation->alpha >= 1) {
      animation->alpha = 1;
      animation->offsetY = 0;
      animation->fade = animations::NO_FADING;
    }
  } else if (animation->fade == animations::FADING_OUT) {
    animation->alpha -= dt * FADE_SPEED;
    animation->offsetY = Lerp(OVERLAY_ANIMATION_OFFSET_Y, 0, animation->alpha);

    if (animation->alpha <= 0) {
      animation->alpha = 0;
      animation->offsetY = OVERLAY_ANIMATION_OFFSET_Y;
      animation->fade = animations::NO_FADING;
    }
  }
}

void DrawFallingBlocks(Game *game)
{
  size_t len = game->falling_blocks.size();

  for (size_t i = 0; i < len; i++)
  {
    entity::FallingBlock *block = &game->falling_blocks.at(i);
    if (block->active) {
      Matrix scale = MatrixScale(block->size.x, block->size.y, block->size.z);
      Matrix rotate = MatrixRotateXYZ(block->rotation);
      Matrix translate = MatrixTranslate(block->position.x, block->position.y, block->position.z);
      Matrix transform = MatrixMultiply(scale, MatrixMultiply(rotate, translate));

      game->cube_model.transform = transform;

      Vector4 normalizedColor = ColorNormalize(block->color);
      Vector3 normalizedColorVec3 = {.x=normalizedColor.x, .y=normalizedColor.y, .z=normalizedColor.z};
      SetShaderValue(game->lighting_shader, GetShaderLocation(game->lighting_shader, "blockColor"), &normalizedColorVec3, SHADER_UNIFORM_VEC3);
      game->cube_model.materials[0].shader = game->lighting_shader;
      DrawModel(game->cube_model, {0}, 1.0, block->color);
    }
  }
}

void UpdateFallingBlocks(Game *game, float dt) {
  size_t len = game->falling_blocks.size();
  for (size_t i = 0; i < len; i++) {
    entity::FallingBlock *block = &game->falling_blocks.at(i);
    if (block->active)
    {
      block->rotation.x += block->rotation_speed.x * dt;
      block->rotation.y += block->rotation_speed.y * dt;
      block->rotation.z += block->rotation_speed.z * dt;

      block->position.x += block->velocity.x * dt;
      block->position.y += block->velocity.y * dt;
      block->position.z += block->velocity.z * dt;

      if (block->position. y < -100) {
        block->active = false;
      }
    }
  }
}

int main() {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tower Blocks");

  int monitorHz = GetMonitorRefreshRate(GetCurrentMonitor());
  SetTargetFPS(monitorHz);

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
    UpdateFallingBlocks(&game, dt);
    UpdateCurrentBlock(&game, dt);
    UpdateScore(&game, dt);
    UpdateOverlay(&game, dt);

    SetShaderValue(game.lighting_shader, GetShaderLocation(game.lighting_shader, "cameraPosition"), &camera.position, SHADER_UNIFORM_VEC3);

    BeginDrawing();
      ClearBackground(BG_COLOR);
      BeginMode3D(camera);
        DrawPlacedBlocks(&game);
        DrawFallingBlocks(&game);
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
