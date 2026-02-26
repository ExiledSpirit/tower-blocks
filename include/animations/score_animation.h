#pragma once

namespace animations {

class ScoreAnimation {
public:
  ScoreAnimation();
  ScoreAnimation(float scale, float duration);
  float scale;
  float duration;
};

}