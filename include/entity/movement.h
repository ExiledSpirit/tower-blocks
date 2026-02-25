#pragma once

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
