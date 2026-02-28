#include "ui/ui_manager.h"
#include "raymath.h"
#include <cmath>

namespace ui
{

UIManager::UIManager() {
  canvas = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
  uiShader = LoadShader(0, "shaders/ui_post.glsl");
  intensityLoc = GetShaderLocation(uiShader, "effectIntensity");
  timeLoc = GetShaderLocation(uiShader, "time");
}

UIManager::~UIManager() {
  UnloadRenderTexture(canvas);
  UnloadShader(uiShader);
}
void UIManager::SpawnPerfect() {
    TriggerPulse(); // Triggers the shader intensity
    TextElement e;
    e.text = "PERFECT!";
    // Move it to the right side of the tower
    e.position = { (float)GetScreenWidth() * 0.65f, 300.0f }; 
    e.fontSize = 35.0f; // Smaller
    e.color = YELLOW;
    e.lifetime = 1.5f;
    e.maxLifetime = 1.5f;
    e.anim = UIAnimType::WIGGLE; // Our sequential logic
    e.useBloom = true;
    elements.push_back(e);
}

void UIManager::SpawnClose() {
  TriggerPulse();
  SpawnMessage("Close...", { (float)GetScreenWidth() * 0.7f, 350.0f }, LIGHTGRAY, false, UIAnimType::NONE);
}

void UIManager::SpawnMessage(std::string text, Vector2 pos, Color color, bool isBloom, UIAnimType anim) {
  elements.push_back({text, pos, 40.0f, color, 1.5f, 1.5f, anim, 0.0f, isBloom});
}

void UIManager::Update(float dt) {
  effectTimer = fmaxf(0.0f, effectTimer - dt * 2.0f);
  
  for (int i = elements.size() - 1; i >= 0; i--) {
    elements[i].lifetime -= dt;
    if (elements[i].lifetime <= 0) elements.erase(elements.begin() + i);
  }
}

void UIManager::DrawScore(size_t score)
{
  const char* title = TextFormat("%zu", score);
  int fontSize = 120;

  int screenWidth = GetScreenWidth();
  int textSize = MeasureText(title, fontSize);

  int position = (screenWidth - textSize) / 2;
  DrawText(title, position, 200, fontSize, RAYWHITE);
}


void UIManager::DrawActiveOverlay()
{
  switch(this->currentState) {
    case UIState::START:
      DrawStartOverlay();
      break;
    case UIState::GAME_OVER:
      DrawGameOverOverlay();
      break;
  }
}

void DrawOverlay(const char *title, const char *subtitle, size_t titleSize, size_t subtitleSize, int titleY, int subtitleY) {
  Color dark = DARKGRAY;
  Color light = GRAY;

  int screenWidth = GetScreenWidth();
  int titleWidth = MeasureText(title, titleSize);
  int subtitleWidth = MeasureText(subtitle, subtitleSize);

  DrawText(title, (screenWidth - titleWidth) / 2, titleY, titleSize, dark);
  DrawText(subtitle, (screenWidth - subtitleWidth) / 2, subtitleY, subtitleSize, light);
}

void UIManager::DrawStartOverlay() {
  DrawOverlay("START GAME", "Click or Press Space", 60, 30, 100, 170);
}

void UIManager::DrawGameOverOverlay() {
  DrawOverlay("GAME OVER", "Click or Press Space", 60, 30, 100, 170);
}

void UIManager::BeginUI() { BeginTextureMode(canvas); ClearBackground(BLANK); }
void UIManager::EndUI() { EndTextureMode(); }
void UIManager::Render() {
    for (auto& e : elements) {
        float timeActive = e.maxLifetime - e.lifetime;
        
        for (int i = 0; i < e.text.length(); i++) {
            Vector2 charPos = { e.position.x + (i * e.fontSize * 0.5f), e.position.y };
            
            // SEQUENTIAL WIGGLE LOGIC
            // Each letter wiggles when timeActive hits its specific slot (0.1s delay per char)
            float charStartTime = i * 0.08f;
            float charWiggleDuration = 0.3f;
            
            if (e.anim == UIAnimType::WIGGLE) {
                if (timeActive > charStartTime && timeActive < charStartTime + charWiggleDuration) {
                    float localT = (timeActive - charStartTime) / charWiggleDuration;
                    charPos.y -= sinf(localT * PI) * 15.0f; // One clean hop/wiggle
                }
            }

            DrawText(TextFormat("%c", e.text[i]), charPos.x, charPos.y, e.fontSize, Fade(e.color, e.lifetime / e.maxLifetime));
        }
    }

    // 2. Draw canvas to screen with shader
    float time = GetTime();
    SetShaderValue(uiShader, intensityLoc, &effectTimer, SHADER_UNIFORM_FLOAT);
    SetShaderValue(uiShader, timeLoc, &time, SHADER_UNIFORM_FLOAT);

    BeginShaderMode(uiShader);
        DrawTextureRec(canvas.texture, (Rectangle){ 0, 0, (float)canvas.texture.width, (float)-canvas.texture.height }, (Vector2){ 0, 0 }, WHITE);
    EndShaderMode();
}

}