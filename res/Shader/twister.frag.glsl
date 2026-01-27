/* Copyright (C) 2016 ultitech - All Rights Reserved
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE', which is part of this source code package.
 */

#version 430 core

layout(location = 0) out vec4 fragColor;
layout(location = 1) out int id;

uniform int ObjectID;

void main() {
  fragColor = vec4(1.0);
  id = ObjectID;
}
