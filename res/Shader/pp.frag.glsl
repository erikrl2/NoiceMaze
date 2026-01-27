/* Copyright (C) 2016 ultitech - All Rights Reserved
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE', which is part of this source code package.
 */

#version 430 core

out vec4 fragColor;

uniform sampler2D Image;

void main() {
  //fragColor = texelFetch(Image, ivec2(gl_FragCoord.xy), 0);
  fragColor = vec4(vec3(texelFetch(Image, ivec2(gl_FragCoord.xy), 0).r), 1);
}
