#include "entity/block.h"

namespace entity {

Block::Block(size_t index, Vector3 pos, Vector3 size, Color color, int color_offset, Movement movement) 
    : index(index), position(pos), size(size), color(color), color_offset(color_offset), movement(movement) {}

}