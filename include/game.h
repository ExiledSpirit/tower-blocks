#pragma once
#include "entity/block.h"
#include "entity/falling_block.h"
#include "animations/score_animation.h"
#include "animations/overlay_animation.h"
#include "ui/ui_manager.h"
#include <vector>

// CONSTANTS
const int MOVEMENT_THRESHOLD = 16;

const float SCORE_ANIMATION_DURATION = 0.2;
const float SCORE_ANIMATION_SCALE = 1.5;

const int OVERLAY_ANIMATION_OFFSET_Y = -50;
const float FADE_SPEED = 2.5;


typedef enum {
  READY_STATE,
  PLAYING_STATE,
  GAME_OVER_STATE
} GameState;

class Game {
public:
  Game();

  Camera3D mainCamera;
  Camera3D gameOverCamera;
  GameState state;
  Shader lighting_shader;
  Model cube_model;
  std::vector<entity::Block> placed_blocks;
  std::vector<entity::FallingBlock> falling_blocks;
  entity::Block current_block;
  entity::Block *previous_block;
  animations::ScoreAnimation scoreAnimation;
  animations::OverlayAnimation overlayAnimation;
  ui::UIManager uiManager;

  void InitGame();
  void Update(float dt);
  void Render(float dt);
private:
  /// @brief Action methods
  entity::Block CreateMovingBlock();
  void PlaceBlock();
  entity::FallingBlock CreateFallingBlock(Vector3 position, Vector3 size, Color color);

  /// @brief Update methods
  void UpdateGameState();
  void UpdateCameraPosition(float dt);
  void UpdateCurrentBlock(float dt);
  void UpdateScore(float dt);
  void UpdateOverlay(float dt);
  void UpdateFallingBlocks(float dt);

  /// @brief Render methods
  void Render3D();
  void RenderHUD();

  // 3D Rendering
  void DrawGround();
  void DrawPlacedBlocks();
  void DrawFallingBlocks();
  void DrawCurrentBlock();
  void DrawBlock(const entity::Block *block, Shader lightingShader);
};
