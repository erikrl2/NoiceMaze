/* Copyright (C) 2016 ultitech - All Rights Reserved
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE', which is part of this source code package.
 */

#version 430 core

in vec2 texcoord;
in vec3 worldPos;
in vec3 viewPos;

layout(location = 0) out vec4 out_fragColor;
layout(location = 1) out int out_id;
layout(location = 2) out vec2 out_flow;

uniform sampler2D Diffuse;
uniform bool EffectEnabled = true;
uniform int ObjectID;
uniform mat4 VPMatrix;
uniform vec2 ViewportSize;

vec2 uvFromWorld(vec3 worldDir) {
  vec4 clip = VPMatrix * vec4(worldDir, 1);
  if (clip.w <= 0.0) return vec2(-1);
  vec3 ndc = clip.xyz / clip.w;
  return ndc.xy * 0.5 + 0.5;
}

vec2 getScreenspaceFlowDir(vec3 dirWorld) {
  float eps = 0.2; // TODO: test different initial eps
  vec2 uv0 = uvFromWorld(worldPos);
  vec2 uv1;
  for (int k = 0; k < 8; k++) {
    uv1 = uvFromWorld(worldPos + normalize(dirWorld) * eps);
    if (uv1.x > 0) break;
    eps *= 0.5;
  }
  vec2 dPx = (uv1 - uv0) * ViewportSize;
  return dPx / eps;
}

void main() {
  if (EffectEnabled) {
    out_id = ObjectID;
    out_flow = vec2(0);
    if (ObjectID == 2) out_flow = getScreenspaceFlowDir(vec3(0, 1, 0)); // WALL
  } else {
    vec3 tex_color = texture(Diffuse, texcoord).rgb;
    float x = length(viewPos);
    float intensity = 1.0 / (1.1 + 3.0 * x * x) + 0.1;
    out_fragColor = vec4(tex_color * intensity, 1.0);
  }
}
