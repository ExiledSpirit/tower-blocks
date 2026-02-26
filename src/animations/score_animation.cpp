#include "animations/score_animation.h"

namespace animations
{
ScoreAnimation::ScoreAnimation(): scale(0), duration(0) {}

ScoreAnimation::ScoreAnimation(float scale, float duration)
  : scale(scale), duration(duration) {}

}