/* Copyright (C) 2016 ultitech - All Rights Reserved
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE', which is part of this source code package.
 */

#version 430 core

layout (location = 0) in vec3 in_position;

void main()
{
	gl_Position = vec4(in_position, 1.0);
}
