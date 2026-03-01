#include "raylib.h"
#include "raymath.h"
#include "entity/physics.h"
#include "entity/movement.h"

namespace entity
{
class Entity
{
public:
  entity::Physics physics;
  entity::Movement movement;
  Vector3 position;

  Entity(Vector3 pos): position(pos) {}

  virtual ~Entity() = default;
};
}