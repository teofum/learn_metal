#include <metal_stdlib>

#include "vertex.hpp"

using namespace metal;

struct RasterVertex {
  float4 position [[position]];
  float4 color;
};

vertex RasterVertex vertexShader(
  uint vertexID [[vertex_id]],
  constant Vertex *vertices [[buffer(0)]],
  constant uint2 *viewportSizeP [[buffer(1)]]
) {
  float2 pixelSpacePos = vertices[vertexID].position.xy;
  float2 viewportSize = float2(*viewportSizeP);

  RasterVertex out;
  out.position = float4(0.0, 0.0, 0.0, 1.0);
  out.position.xy = pixelSpacePos / (viewportSize / 2.0);
  out.color = vertices[vertexID].color;

  return out;
}

fragment float4 fragmentShader(RasterVertex in [[stage_in]]) {
  return in.color;
}
