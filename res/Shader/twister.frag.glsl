/* Copyright (C) 2016 ultitech - All Rights Reserved
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE', which is part of this source code package.
 */

#version 430 core

layout(location = 0) out vec4 out_fragColor;
layout(location = 1) out int out_id;
layout(location = 2) out vec2 out_flow;

uniform int ObjectID;

void main() {
  out_fragColor = vec4(1.0);
  out_id = ObjectID;
  out_flow = vec2(0);
}
