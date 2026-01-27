/* Copyright (C) 2016 ultitech - All Rights Reserved
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE', which is part of this source code package.
 */

#version 430 core

uniform sampler2D Diffuse;

in vec3 position;
in vec2 texcoord;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out int id;

uniform int ObjectID;

void main() {
  vec3 tex_color = texture(Diffuse, texcoord).rgb;
  float x = length(position);
  float intensity = 1.0 / (1.1 + 3.0 * x * x) + 0.1;
  fragColor = vec4(tex_color * intensity, 1.0);
  id = ObjectID;
}
