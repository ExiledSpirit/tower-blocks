#pragma once

#include "raylib.h"
#include "entity/movement.h"
#include <cstddef>

namespace entity {

inline const Vector3 default_pos = { 0.0f, 0.0f, 0.0f };
inline const Vector3 default_size = { 10.0f, 2.0f, 10.0f };
inline const Color default_color = { .r = 150, .g = 150, .b = 150, .a = 255 };
inline const Movement default_movement = { .speed = 0, .direction = FORWARD, .axis = X };

class Block {
public:
  Block(size_t index = 0, 
    Vector3 pos = default_pos,
    Vector3 size = default_size,
    Color color = default_color,
    int color_offset = 0,
    Movement movement = default_movement);

  size_t index;
  Vector3 position;
  Vector3 size;
  Color color;
  int color_offset;
  Movement movement;
};

}