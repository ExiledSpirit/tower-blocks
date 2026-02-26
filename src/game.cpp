#include "game.h"
#include "external/reasings.h"
#include "raymath.h"

Game::Game()
{
  Camera3D camera = (Camera3D) {
    .position = (Vector3) { .x = 50, .y = 50, .z = 50 },
    .target = (Vector3) { .x = 0, .y = 0, .z = 0 },
    .up = (Vector3) { .x = 0, .y = 1, .z = 0 },
    .fovy = 60,
    .projection = CAMERA_ORTHOGRAPHIC
  };

  this->mainCamera = camera;
}

void Game::Update(float dt)
{
  UpdateGameState();
  UpdateCameraPosition(dt);
  UpdateFallingBlocks(dt);
  UpdateCurrentBlock(dt);
  uiManager.Update(dt); // UI Manager handles its own timers now!
}

void Game::Render3D()
{
  SetShaderValue(this->lighting_shader, GetShaderLocation(this->lighting_shader, "cameraPosition"), &this->mainCamera.position, SHADER_UNIFORM_VEC3);
  BeginMode3D(this->mainCamera);
    DrawPlacedBlocks();
    DrawFallingBlocks();
    DrawCurrentBlock();
  EndMode3D();
}

void Game::Render(float dt)
{
  Render3D();

  // 2. Draw HUD to the Canvas
  uiManager.BeginUI();
    uiManager.DrawScore(this->placed_blocks.size() - 1);
    
    uiManager.DrawActiveOverlay();
  uiManager.EndUI();

  // 3. Draw Canvas to screen with the Post-Processing Shader
  uiManager.Render();
}

void Game::UpdateGameState() {
    bool inputPressed = IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    switch (this->state) {
        case READY_STATE:
            uiManager.SetState(ui::UIState::START); // Sync UI State
            if (inputPressed) {
                this->state = PLAYING_STATE;
                uiManager.SetState(ui::UIState::PLAYING);
                this->current_block = CreateMovingBlock();
            }
            break;

        case PLAYING_STATE:
            if (inputPressed) {
                PlaceBlock();
                this->current_block = CreateMovingBlock();
            }
            break;

        case GAME_OVER_STATE:
            uiManager.SetState(ui::UIState::GAME_OVER); // Sync UI State
            if (inputPressed) InitGame();
            break;
    }
}

void Game::UpdateCameraPosition(float dt) {
  size_t placed_blocks_len = this->placed_blocks.size();

  this->mainCamera.position.y = Lerp(this->mainCamera.position.y, 50 + (2 * placed_blocks_len), dt);
  this->mainCamera.target.y = Lerp(this->mainCamera.target.y, 2 * placed_blocks_len, dt);
}

void Game::UpdateCurrentBlock(float dt) {
  if (this->state != PLAYING_STATE) {
    return;
  }

  entity::Block *block = &this->current_block;
  float *axisPosition = block->movement.axis == X ? &block->position.x : &block->position.z;

  int direction = block->movement.direction == FORWARD ? 1 : -1;
  *axisPosition += direction * block->movement.speed * dt;

  if (fabs(*axisPosition) >= MOVEMENT_THRESHOLD) {
    block->movement.direction = block->movement.direction == FORWARD ? BACKWARD : FORWARD;
    *axisPosition = fmax(fmin(MOVEMENT_THRESHOLD, *axisPosition), -MOVEMENT_THRESHOLD);
  }
}

void Game::UpdateScore(float dt) {
  animations::ScoreAnimation *animation = &this->scoreAnimation;
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

void Game::UpdateOverlay(float dt) {
  animations::OverlayAnimation *animation = &this->overlayAnimation;

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

void Game::UpdateFallingBlocks(float dt) {
  size_t len = this->falling_blocks.size();
  for (size_t i = 0; i < len; i++) {
    entity::FallingBlock *block = &this->falling_blocks.at(i);
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
void Game::DrawBlock(const entity::Block *block, Shader lightingShader) {
  Vector4 normalizedColor = ColorNormalize(block->color);
  Vector3 normalizedColorVec3 = {.x=normalizedColor.x, .y=normalizedColor.y, .z=normalizedColor.z};
  SetShaderValue(lightingShader, GetShaderLocation(lightingShader, "blockColor"), &normalizedColorVec3, SHADER_UNIFORM_VEC3);
  BeginShaderMode(lightingShader);
    DrawCube(block->position, block->size.x, block->size.y, block->size.z, block->color);
  EndShaderMode();
}

void Game::DrawCurrentBlock() {
  if (this->state != PLAYING_STATE) {
    return;
  }

  DrawBlock(&this->current_block, this->lighting_shader);
}

void Game::DrawPlacedBlocks() {
  std::vector blocks = this->placed_blocks;

  for (size_t i = 0; i < blocks.size(); i++) {
    entity::Block *block = &blocks[i];
    DrawBlock(block, this->lighting_shader);
  }
}

entity::Block default_block;

void Game::InitGame() {
  this->lighting_shader = LoadShader("shaders/3d/lighting_vertex.glsl", "shaders/3d/lighting_fragment.glsl");

  this->cube_model = LoadModelFromMesh(GenMeshCube(1, 1, 1));
  this->state = READY_STATE;
  this->placed_blocks.clear();
  this->falling_blocks.clear();
  this->current_block = default_block;
  this->current_block.color_offset = GetRandomValue(0, 100);

  this->scoreAnimation.duration = SCORE_ANIMATION_DURATION;
  this->scoreAnimation.scale = SCORE_ANIMATION_SCALE;

  this->overlayAnimation = {
    .type = animations::START_GAME_OVERLAY,
    .fade = animations::FADING_IN,
    .alpha = 0,
    .offsetY = OVERLAY_ANIMATION_OFFSET_Y
  };

  this->placed_blocks.push_back(default_block);
  this->previous_block = &this->placed_blocks[0];
}

entity::Block Game::CreateMovingBlock() {
  entity::Block *target = this->previous_block;

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

entity::FallingBlock Game::CreateFallingBlock(Vector3 position, Vector3 size, Color color) {
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

void Game::PlaceBlock() {
  entity::Block *current = &this->current_block;
  entity::Block *target = this->previous_block;

  bool isXAxis = current->movement.axis == X;
  float currentPosition = isXAxis ? current->position.x : current->position.z;
  float targetPosition = isXAxis ? target->position.x : target->position.z;
  float currentSize = isXAxis ? current->size.x : current->size.z;
  float targetSize = isXAxis ? target->size.x : target->size.z;

  float delta = currentPosition - targetPosition;
  float overlay = targetSize - fabs(delta);

  if (overlay < 0.1) {
    this->state = GAME_OVER_STATE;
    return;
  }

  bool isPerfectOverlay = fabs(delta) < 0.3;
  if (isPerfectOverlay) { // TODO: Add cool dopamine effect
    uiManager.SpawnPerfect();
    if (isXAxis) {
      current->size.x = target->size.x;
      current->position.x = target->position.x;
    } else {
      current->size.z = target->size.z;
      current->position.z = target->position.z;
    }
  }else {
    if (overlay < 0.5) uiManager.SpawnClose();
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
      this->falling_blocks.push_back(CreateFallingBlock(choppedPositionVec, choppedSizeVec, current->color));
    }
  }

  this->placed_blocks.push_back(this->current_block);
  this->previous_block = &this->placed_blocks.back();
  
  this->scoreAnimation.duration = SCORE_ANIMATION_DURATION;
  this->scoreAnimation.scale = SCORE_ANIMATION_SCALE;
}

void Game::DrawFallingBlocks()
{
  size_t len = this->falling_blocks.size();

  for (size_t i = 0; i < len; i++)
  {
    entity::FallingBlock *block = &this->falling_blocks.at(i);
    if (block->active) {
      Matrix scale = MatrixScale(block->size.x, block->size.y, block->size.z);
      Matrix rotate = MatrixRotateXYZ(block->rotation);
      Matrix translate = MatrixTranslate(block->position.x, block->position.y, block->position.z);
      Matrix transform = MatrixMultiply(scale, MatrixMultiply(rotate, translate));

      this->cube_model.transform = transform;

      Vector4 normalizedColor = ColorNormalize(block->color);
      Vector3 normalizedColorVec3 = {.x=normalizedColor.x, .y=normalizedColor.y, .z=normalizedColor.z};
      SetShaderValue(this->lighting_shader, GetShaderLocation(this->lighting_shader, "blockColor"), &normalizedColorVec3, SHADER_UNIFORM_VEC3);
      this->cube_model.materials[0].shader = this->lighting_shader;
      DrawModel(this->cube_model, {0}, 1.0, block->color);
    }
  }
}
