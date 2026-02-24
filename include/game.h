#pragma once
#include "entity/block.h"
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
  std::vector<entity::Block> placed_blocks;
  entity::Block current_block;
  entity::Block *previous_block;
};
