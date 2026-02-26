#include "raylib.h"
#include "game.h"

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 1000;
const Color BG_COLOR = (Color){.r = 0x87, .g = 0xCE, .b = 0xEB, .a = 255};

int main() {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tower Blocks");

  int monitorHz = GetMonitorRefreshRate(GetCurrentMonitor());
  SetTargetFPS(monitorHz);

  Game game = Game();
  game.InitGame();

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();
    
    game.Update(dt);

    BeginDrawing();
      ClearBackground(BG_COLOR);
      game.Render(dt);

      DrawFPS(10, 10);
    EndDrawing();
  }

  CloseWindow();
  return 1;
}
