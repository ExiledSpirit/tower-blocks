#pragma once

#include "raylib.h"

namespace entity {

typedef enum {
  FORWARD,
  BACKWARD
} Direction;

typedef enum {
  X,
  Z
} Axis;

typedef struct {
  float speed;
  Direction direction;
  Axis axis;
} Movement;

inline const Vector3 default_pos = { 0.0f, 0.0f, 0.0f };
inline const Vector3 default_size = { 10.0f, 2.0f, 10.0f };
inline const Color default_color = { .r = 150, .g = 150, .b = 150, .a = 255 };
inline const Movement default_movement = { .speed = 0, .direction = FORWARD, .axis = X };

class Block {
public:
  Block(
    Vector3 pos = default_pos,
    Vector3 size = default_size,
    Color color = default_color,
    Movement movement = default_movement);

  Vector3 position;
  Vector3 size;
  Color color;
  Movement movement;
};

}