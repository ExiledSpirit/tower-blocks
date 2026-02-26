#pragma once
#include "entity/block.h"
#include "entity/falling_block.h"
#include "animations/score_animation.h"
#include "animations/overlay_animation.h"
#include <vector>

typedef enum {
  READY_STATE,
  PLAYING_STATE,
  GAME_OVER_STATE
} GameState;

class Game {
public:
  Game();

  GameState state;
  Model cube_model;
  std::vector<entity::Block> placed_blocks;
  std::vector<entity::FallingBlock> falling_blocks;
  entity::Block current_block;
  entity::Block *previous_block;
  animations::ScoreAnimation scoreAnimation;
  animations::OverlayAnimation overlayAnimation;
};
