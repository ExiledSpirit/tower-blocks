#pragma once
#include <memory>
#include "raylib.h"
#include "raymath.h"
#include "physics.h"
#include "movement.h"
#include "math/color.h"
#include "entity/entity.h"

namespace entity {
  enum class BlockState { MOVING, PLACED, FALLING };

  class Block: public Entity {
  public:
    // Core Data
    size_t index;
    Vector3 size;
    math::Color color;
    int color_offset;
    BlockState state;

    // Optional Components
    std::unique_ptr<Physics> physics = nullptr;
    std::unique_ptr<Movement> movement = nullptr;

    Block() : Entity({0,0,0}), index(0), size({1,1,1}), color({255,255,255,255}), state(BlockState::PLACED) {}
    Block(size_t idx, Vector3 pos, Vector3 sz, math::Color col) 
        : Entity(pos), index(idx), size(sz), color(col), state(BlockState::PLACED) {}

    // Helper to turn this block into a falling piece
    void SetFalling(Vector3 initialVelocity) {
        state = BlockState::FALLING;
        movement = nullptr; // Stop moving sideways
        physics = std::make_unique<Physics>();
        physics->velocity = initialVelocity;
        physics->rotation_speed = { 2.0f, 1.0f, 0.5f };
    }

    void SetMoving(Movement config) {
      this->state = BlockState::MOVING;
      this->physics = nullptr;
      this->movement = std::make_unique<Movement>(config);
    }

    void SetPlaced() {
      this->state = BlockState::PLACED;
      this->movement = nullptr;
      this->physics = nullptr;
    }
  };
}