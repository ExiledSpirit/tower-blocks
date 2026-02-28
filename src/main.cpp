#include "raylib.h"
#include "game.h"

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 1000;
const Color BG_COLOR = (Color){.r = 0x87, .g = 0xCE, .b = 0xEB, .a = 255};

int main() {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tower Blocks");

  int monitorHz = GetMonitorRefreshRate(GetCurrentMonitor());
  SetTargetFPS(monitorHz);

  /** TODO: probably gonna make it part of terrain class in the future  */
  Shader balatroShader = LoadShader(0, "shaders/ui/balatro.fs");
  RenderTexture2D target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
  
  int iResolutionLoc = GetShaderLocation(balatroShader, "iResolution");
  int iTimeLoc = GetShaderLocation(balatroShader, "iTime");
  float resolution[2] = { (float)GetScreenWidth(), (float)GetScreenHeight() };
  SetShaderValue(balatroShader, iResolutionLoc, resolution, SHADER_UNIFORM_VEC2);
  /**  */

  Game game = Game();
  game.InitGame();

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();
    float time = (float)GetTime();

    game.Update(dt);
    SetShaderValue(balatroShader, iTimeLoc, &time, SHADER_UNIFORM_FLOAT);

    BeginDrawing();
      ClearBackground(RAYWHITE);

      BeginShaderMode(balatroShader);
        DrawTextureRec(target.texture, 
                        (Rectangle){ 0, 0, (float)target.texture.width, (float)-target.texture.height }, 
                        (Vector2){ 0, 0 }, WHITE);
      EndShaderMode();

      game.Render(dt);

      DrawFPS(10, 10);
    EndDrawing();
  }

  // cleanups
  UnloadShader(balatroShader);
  UnloadRenderTexture(target);
  CloseWindow();
  return 0;
}