#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-attributes"

#ifndef LEARN_METAL_VERTEX_HPP
#define LEARN_METAL_VERTEX_HPP

#include <simd/simd.h>

using namespace simd;

struct Vertex {
  float3 position [[attribute(0)]];
  float4 color [[attribute(1)]];
};

struct Transforms {
  float4x4 model;
  float4x4 view;
  float4x4 projection;
};

#endif //LEARN_METAL_VERTEX_HPP

#pragma clang diagnostic pop