#ifndef LEARN_METAL_MATRICES_HPP
#define LEARN_METAL_MATRICES_HPP

#include <simd/simd.h>

using namespace simd;

namespace mat {
float4x4 identity();

float4x4 translation(float3 t);

float4x4 rotation(float angle, float3 axis);

float4x4 scaling(float3 s);

float4x4 scaling(float s);

float4x4 projection(float fov, float aspect, float near, float far);
}

#endif //LEARN_METAL_MATRICES_HPP
