/* Copyright (C) 2016 ultitech - All Rights Reserved
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE', which is part of this source code package.
 */

#version 430 core

out vec4 fragColor;

void main() {
  if(!gl_FrontFacing) discard;
  fragColor = vec4(1.0);
}
