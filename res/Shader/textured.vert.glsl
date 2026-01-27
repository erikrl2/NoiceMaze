/* Copyright (C) 2016 ultitech - All Rights Reserved
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE', which is part of this source code package.
 */

#version 430 core

layout(location = 0) in vec3 in_position;
layout(location = 2) in vec2 in_texcoord;

out vec2 texcoord;
out vec3 worldPos;
out vec3 viewPos;

uniform mat4 MMatrix;
uniform mat4 MVMatrix;
uniform mat4 MVPMatrix;

void main() {
  texcoord = in_texcoord;

  worldPos = vec3(MMatrix * vec4(in_position, 1.0));
  viewPos = vec3(MVMatrix * vec4(in_position, 1.0));
  gl_Position = MVPMatrix * vec4(in_position, 1.0);
}
