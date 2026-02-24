#include "entity/block.h"

namespace entity {

Block::Block(Vector3 pos, Vector3 size, Color color, Movement movement) 
    : position(pos), size(size), color(color), movement(movement) {}

}