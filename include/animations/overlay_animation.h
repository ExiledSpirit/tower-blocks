#pragma once

namespace animations
{
typedef enum {
  START_GAME_OVERLAY,
  GAME_OVER_OVERLAY
} OverlayType;

typedef enum {
  FADING_IN,
  FADING_OUT,
  NO_FADING
} FadeState;

class OverlayAnimation
{
public:
  OverlayType type;
  FadeState fade;
  float alpha;
  float offsetY;
};
}