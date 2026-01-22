/* Copyright (C) 2016 ultitech - All Rights Reserved
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE', which is part of this source code package.
 */

#version 430 core

uniform sampler2D Image;

out vec4 fragColor;

void main()
{
	fragColor = texelFetch(Image, ivec2(gl_FragCoord.xy), 0);
}
