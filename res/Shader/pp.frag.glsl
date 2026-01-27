/* Copyright (C) 2016 ultitech - All Rights Reserved
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE', which is part of this source code package.
 */

#version 430 core

out vec4 out_fragColor;

uniform sampler2D Image;
uniform bool EffectEnabled = true;

void main() {
  vec3 c = texelFetch(Image, ivec2(gl_FragCoord.xy), 0).rgb;

  if (EffectEnabled) {
    out_fragColor = vec4(c.r, c.r, c.r, 1);
  } else {
    out_fragColor = vec4(c, 1);
  }
}
