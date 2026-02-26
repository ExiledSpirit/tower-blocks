#pragma once
#include "raylib.h"

namespace entity {

class FallingBlock
{
public:
  Vector3 position;
  Vector3 size;
  Vector3 rotation;
  Vector3 rotation_speed;
  Vector3 velocity;
  Color color;
  bool active;
};

}