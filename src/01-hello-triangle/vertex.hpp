#ifndef LEARN_METAL_VERTEX_HPP
#define LEARN_METAL_VERTEX_HPP

#include <simd/simd.h>

using namespace simd;

struct Vertex {
  float2 position;
  float4 color;
};

#endif //LEARN_METAL_VERTEX_HPP
