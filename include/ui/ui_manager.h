#pragma once
#include <raylib.h>

namespace ui
{

class UIManager {
public:
  RenderTexture2D canvas;
  Shader uiShader;
  int intensityLoc;
  int timeLoc;
  float effectTimer = 0.0f;

  UIManager() {
    // Create a canvas the size of the window
    canvas = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    uiShader = LoadShader(0, "shaders/ui/ui_post.glsl");
    intensityLoc = GetShaderLocation(uiShader, "effectIntensity");
    timeLoc = GetShaderLocation(uiShader, "time");
  }

  void TriggerPerfectEffect() {
    effectTimer = 1.0f; // Start the "dopamine" pulse
  }

  void Update(float dt) {
    if (effectTimer > 0) effectTimer -= dt * 2.0f; // Fade out
  }

  void BeginUI() {
    BeginTextureMode(canvas);
    ClearBackground(BLANK); // Essential: keep it transparent!
  }

  void EndUI() {
    EndTextureMode();
  }

  void Render() {
    float time = (float)GetTime();
    SetShaderValue(uiShader, intensityLoc, &effectTimer, SHADER_UNIFORM_FLOAT);
    SetShaderValue(uiShader, timeLoc, &time, SHADER_UNIFORM_FLOAT);

    BeginShaderMode(uiShader);
      // Draw the canvas texture to the screen
      // Note: texture.height is flipped because textures are Y-up in OpenGL
      DrawTextureRec(canvas.texture, 
        (Rectangle){ 0, 0, (float)canvas.texture.width, (float)-canvas.texture.height }, 
        (Vector2){ 0, 0 }, WHITE);
    EndShaderMode();
  }
};
}