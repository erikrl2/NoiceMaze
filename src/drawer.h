/* Copyright (C) 2016 ultitech - All Rights Reserved
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE', which is part of this source code package.
 */

#ifndef H_DRAWER
#define H_DRAWER

#include "mesh.h"

#include <SDL_keycode.h>
#include <stddef.h>

#define DRAWER_WINDOW_RENDERTARGET 0

typedef unsigned int Texture;
typedef unsigned int Program;
typedef unsigned int Rendertarget;

void drawer_init();
void drawer_quit();
void drawer_modelview_set(float matrix[16]);
void drawer_modelview_get(float matrix[16]);
Program drawer_create_program(char* vertex_filename, char* fragment_filename);
void drawer_use_program(Program program);
Texture drawer_load_texture(char* filename);
void drawer_use_texture(Texture texture, unsigned int texture_unit, char* uniform_name);
Rendertarget drawer_create_rendertarget();
void drawer_use_rendertarget(Rendertarget target, char clear);
void drawer_use_rendertarget_texture(Rendertarget target, unsigned int texture_unit, char* uniform_name);
void drawer_draw_mesh(Mesh* mesh);
void drawer_begin_scene(float time_passed);
void drawer_end_scene();
void drawer_set_3d_mode(enum Drawer3DMode mode);
void drawer_create_mesh_vbo(Mesh* mesh);
void drawer_free_mesh_vbo(MeshVBO* vbo);
void drawer_screenshot();
void drawer_print_glinfo();

void drawer_effect_init();
void drawer_effect_shutdown();
void drawer_effect_toggle();
char drawer_effect_is_enabled();
void drawer_effect_begin_frame();
void drawer_effect_set_view_matrix(float view[16]);
int drawer_effect_store_model_matrix(float model[16]);
Texture drawer_effect_apply();
void drawer_effect_render_to_screen(Texture tex);

#endif // H_DRAWER
