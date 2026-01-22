/* Copyright (C) 2016 ultitech - All Rights Reserved
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE', which is part of this source code package.
 */

#version 430 core

uniform sampler2D Image;
uniform ivec2 ScreenSize;
uniform vec2 GaussValues[11];

out vec4 fragColor;

void main()
{
	vec2 circle_pos = (gl_FragCoord.xy / vec2(ScreenSize)) * 2.0 - 1.0;
	float len = length(circle_pos);
	
	vec4 color;
	for(int i=0; i<11; i++)
	{
		color += texture(Image, (gl_FragCoord.xy + circle_pos * len * 5 * GaussValues[i].x) / vec2(ScreenSize)) * GaussValues[i].y;
	}
	
	fragColor = color;
}
