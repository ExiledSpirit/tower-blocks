#pragma once
#include "raylib.h"
#include "raymath.h" // Needed for fmax/fmin/fabs

namespace entity
{

enum Direction { FORWARD, BACKWARD };
enum Axis      { X, Z };

struct Movement {
    float speed;
    Direction direction;
    Axis axis;
    float threshold = 20.0f; // MOVEMENT_THRESHOLD moved here

    void Update(Vector3& position, float dt) {
        // Decide which coordinate to change
        float* axisPosition = (axis == X) ? &position.x : &position.z;

        // Apply movement
        int dirMultiplier = (direction == FORWARD) ? 1 : -1;
        *axisPosition += dirMultiplier * speed * dt;

        // Handle the "Ping-Pong" bounce
        if (fabs(*axisPosition) >= threshold) {
            direction = (direction == FORWARD) ? BACKWARD : FORWARD;
            
            // Clamp it so it doesn't get stuck outside the threshold
            if (*axisPosition > threshold) *axisPosition = threshold;
            if (*axisPosition < -threshold) *axisPosition = -threshold;
        }
    }
};

}