#pragma once
#include "raylib.h"
#include "raymath.h"

namespace entity {
    class Physics {
    public:
        Vector3 velocity = { 0, 0, 0 };
        Vector3 rotation = { 0, 0, 0 };
        Vector3 rotation_speed = { 0, 0, 0 };
        float gravity = -15.0f; // Higher gravity feels "snappier" in stacker games
        bool isGrounded = false;

        void Integrate(Vector3& position, float dt) {
            if (!isGrounded)
            {
                velocity.y += gravity * dt;
            }

            position = Vector3Add(position, Vector3Scale(velocity, dt));
            rotation = Vector3Add(rotation, Vector3Scale(rotation_speed, dt));
        }
    };
}