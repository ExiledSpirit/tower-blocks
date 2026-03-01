#include "game.h"
#include "external/reasings.h"
#include "entity/movement.h"
#include "raylib.h"
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

void DrawTerrain()
{
  DrawCube({0, -2, 0}, 50, 4, 50, {0xac, 0xca, 0x84, 255}); // TODO: Change for terrain.Draw() and terrain.update() in the future
}

void Game::Render3D()
{
  SetShaderValue(this->lighting_shader, GetShaderLocation(this->lighting_shader, "cameraPosition"), &this->mainCamera.position, SHADER_UNIFORM_VEC3);
  BeginMode3D(this->mainCamera);
    DrawTerrain();
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
    if (this->state != PLAYING_STATE) return;

    // Delegate the movement logic to the component we built!
    if (current_block.movement) {
        current_block.movement->Update(current_block.position, dt);
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
  for (auto& block : falling_blocks)
  {
    if (block.physics)
    {
      block.physics->Integrate(block.position, dt);

      if (block.position.y < -50.f)
      {
        block.physics = nullptr;
      }
    }
  }
}

entity::Block& Game::GetPreviousBlock() {
    return placed_blocks[previousBlockIndex];
}

void Game::DrawBlock(const entity::Block *block, Shader lightingShader) {
  math::Color color = block->color;
  Vector4 normalizedColor = ColorNormalize({.r = color.r, .g = color.g, .b = color.b, .a = color.a});
  Vector3 normalizedColorVec3 = {.x=normalizedColor.x, .y=normalizedColor.y, .z=normalizedColor.z};
  SetShaderValue(lightingShader, GetShaderLocation(lightingShader, "blockColor"), &normalizedColorVec3, SHADER_UNIFORM_VEC3);
  BeginShaderMode(lightingShader);
    DrawCube(block->position, block->size.x, block->size.y, block->size.z, {.r = color.r, .g = color.g, .b = color.b, .a = color.a});
  EndShaderMode();
}

void Game::DrawCurrentBlock() {
  if (this->state != PLAYING_STATE) {
    return;
  }

  DrawBlock(&this->current_block, this->lighting_shader);
}

void Game::DrawPlacedBlocks() {
  const std::vector<entity::Block>& blocks = this->placed_blocks;

  for (size_t i = 0; i < blocks.size(); i++) {
    const entity::Block *block = &blocks[i];
    DrawBlock(block, this->lighting_shader);
  }
}
void Game::InitGame() {
  this->lighting_shader = LoadShader("shaders/3d/lighting_vertex.glsl", "shaders/3d/lighting_fragment.glsl");
  this->cube_model = LoadModelFromMesh(GenMeshCube(1, 1, 1));
  
  this->state = READY_STATE;
  this->placed_blocks.clear();
  this->falling_blocks.clear();

  // 1. Create and move the BASE block into the tower first
  entity::Block baseBlock(0, {0,0,0}, {10, 2, 10}, {255, 255, 255, 255});
  this->placed_blocks.push_back(std::move(baseBlock));

  // 2. NOW it is safe to point to it
  this->previous_block = &this->placed_blocks.back();

  // 3. Create the first MOVING block (the one the player controls)
  // Note: Ensure your Block constructor handles these arguments
  this->current_block = entity::Block(1, {0, 2, 0}, {10, 2, 10}, {200, 200, 200, 255});
  this->current_block.color_offset = GetRandomValue(0, 100);

  // ... Animation Init ...
  this->scoreAnimation.duration = SCORE_ANIMATION_DURATION;
  this->scoreAnimation.scale = SCORE_ANIMATION_SCALE;

  this->overlayAnimation = {
    .type = animations::START_GAME_OVERLAY,
    .fade = animations::FADING_IN,
    .alpha = 0,
    .offsetY = OVERLAY_ANIMATION_OFFSET_Y
  };
}
entity::Block Game::CreateMovingBlock() {
    entity::Block* target = this->previous_block;

    // 1. Determine Axis (Flip from X to Z or vice versa)
    // Note the -> arrow for the unique_ptr
    entity::Axis axis = (target->index % 2 == 0) ? entity::Z : entity::X;
    entity::Direction direction = (GetRandomValue(0, 1) == 0) ? entity::FORWARD : entity::BACKWARD;

    // 2. Calculate Position
    Vector3 position = target->position;
    position.y += target->size.y;

    if (axis == entity::X) {
        position.x = (direction == entity::FORWARD ? -1 : 1) * MOVEMENT_THRESHOLD;
    } else {
        position.z = (direction == entity::FORWARD ? -1 : 1) * MOVEMENT_THRESHOLD;
    }

    // 3. Generate Color
    size_t index = target->index + 1;
    int offset = target->color_offset + (int)index;
    math::Color newColor = {
        (unsigned char)(sinf(0.3f * offset) * 55 + 200),
        (unsigned char)(sinf(0.3f * offset + 2.0f) * 55 + 200),
        (unsigned char)(sinf(0.3f * offset + 4.0f) * 55 + 200),
        255
    };

    // 4. Construct the Block
    entity::Block newBlock(index, position, target->size, newColor);
    newBlock.color_offset = target->color_offset;

    // 5. Configure Movement using our new method
    float speed = 16.0f + (index * 0.5f);
    newBlock.SetMoving({.speed = speed, .direction = direction, .axis = axis});

    // 6. Move it out (Crucial for unique_ptr support)
    return newBlock;
}

entity::Block Game::CreateFallingBlock(Vector3 position, Vector3 size, math::Color color) {
    // 1. Create a standard block (id 0 because debris doesn't need an index)
    entity::Block debris(0, position, size, color);

    // 2. Randomize the "tumble" velocity
    Vector3 initialVel = {
        (float)GetRandomValue(-300, 300) / 100.0f,
        (float)GetRandomValue(-100, 100) / 100.0f, // Slight vertical pop
        (float)GetRandomValue(-300, 300) / 100.0f
    };

    // 3. Use the state-transition method we built
    // This internally creates the Physics component and sets the state
    debris.SetFalling(initialVel);

    // 4. Return it using move semantics
    return debris;
}
void Game::PlaceBlock() {
  entity::Block& current = this->current_block; 
  entity::Block& target = *this->previous_block;

  bool isXAxis = current.movement->axis == entity::X;
  float currentPos = isXAxis ? current.position.x : current.position.z;
  float targetPos  = isXAxis ? target.position.x  : target.position.z;
  float currentSize = isXAxis ? current.size.x    : current.size.z;
  float targetSize  = isXAxis ? target.size.x     : target.size.z;

  float delta = currentPos - targetPos;
  float overlay = targetSize - fabs(delta);

  // Game Over Check
  if (overlay < 0.1f) {
    this->state = GAME_OVER_STATE;
    return;
  }

  bool isPerfect = fabs(delta) < 0.3f;

  if (isPerfect) {
    // Snap to target for that "Perfect" feel
    if (isXAxis) current.position.x = target.position.x;
    else         current.position.z = target.position.z;
    
    uiManager.SpawnPerfect();
  } else {
    // --- THE SLICE (The part that stays) ---
    float newSize = overlay;
    float newPos = targetPos + (delta / 2.0f);

    // --- THE CHOP (The debris) ---
    float choppedSize = currentSize - overlay;
    float choppedPos = (delta > 0) 
        ? (newPos + newSize / 2.0f + choppedSize / 2.0f) 
        : (newPos - newSize / 2.0f - choppedSize / 2.0f);

    // Update Current Block Size/Pos
    if (isXAxis) {
      current.size.x = newSize;
      current.position.x = newPos;
    } else {
      current.size.z = newSize;
      current.position.z = newPos;
    }

    // Create the debris block
    Vector3 dPos = current.position;
    Vector3 dSize = current.size;
    if (isXAxis) { dPos.x = choppedPos; dSize.x = choppedSize; }
    else         { dPos.z = choppedPos; dSize.z = choppedSize; }

    this->falling_blocks.push_back(CreateFallingBlock(dPos, dSize, current.color));
  }

  // 2. Finalize the Current Block state
  current.SetPlaced();

  // 3. Move it to the tower (Current becomes empty here!)
  this->placed_blocks.push_back(std::move(current));
  
  // 4. Update the 'previous' pointer safely
  this->previous_block = &this->placed_blocks.back();

  // 5. Spawn the next moving block
  this->current_block = CreateMovingBlock();
}

void Game::DrawFallingBlocks() {
    for (auto& block : this->falling_blocks) {
        // We only draw blocks that actually have physics (debris)
        if (block.physics) {
            // 1. Build the Transformation Matrix
            // Rotation is now retrieved from the physics component
            auto scale     = MatrixScale(block.size.x, block.size.y, block.size.z);
            auto rotate    = MatrixRotateXYZ(block.physics->rotation); 
            auto translate = MatrixTranslate(block.position.x, block.position.y, block.position.z);
            
            // Combine: Scale -> Rotate -> Translate (SRT Order)
            this->cube_model.transform = MatrixMultiply(scale, MatrixMultiply(rotate, translate));

            // 2. Update Shader Uniforms
            ::Color rayColor = { block.color.r, block.color.g, block.color.b, block.color.a };
            Vector4 normalized = ColorNormalize(rayColor);
            Vector3 colorVec3 = { normalized.x, normalized.y, normalized.z };
            
            int loc = GetShaderLocation(this->lighting_shader, "blockColor");
            SetShaderValue(this->lighting_shader, loc, &colorVec3, SHADER_UNIFORM_VEC3);
            
            // 3. Render the Model
            this->cube_model.materials[0].shader = this->lighting_shader;
            DrawModel(this->cube_model, {0, 0, 0}, 1.0f, rayColor);
        }
    }
}