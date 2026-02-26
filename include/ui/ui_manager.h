#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "raylib.h"
#include <vector>
#include <string>

namespace ui
{

enum class UIState { START, PLAYING, GAME_OVER };

enum class UIAnimType { NONE, WIGGLE, POP_IN, FLOAT_UP };

struct TextElement {
    std::string text;
    Vector2 position;
    float fontSize;
    Color color;
    float lifetime;
    float maxLifetime;
    UIAnimType anim;
    float delay; // Used for sequential letter pops
    bool useBloom; // New flag for shader
};

class UIManager {
private:
  UIState currentState = UIState::START;
  RenderTexture2D canvas;
  Shader uiShader;
  std::vector<TextElement> elements;
  
  // Shader locations
  int intensityLoc;
  int timeLoc;
  float effectTimer = 0.0f;

  void DrawStartOverlay();
  void DrawGameOverOverlay();
public:
  UIManager();
  ~UIManager();

  void Update(float dt);
  void BeginUI();
  void EndUI();
  void Render();

  void SetState(UIState newState) { currentState = newState; };
  
  void DrawScore(size_t score);
  void DrawActiveOverlay();
  void SpawnPerfect();
  void SpawnClose();
  void SpawnMessage(std::string text, Vector2 pos, Color color, bool isBloom, UIAnimType anim = UIAnimType::FLOAT_UP);
  void TriggerPulse() { effectTimer = 1.0f; }
};

}

#endif