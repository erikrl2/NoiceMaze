/* Copyright (C) 2016 ultitech - All Rights Reserved
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE', which is part of this source code package.
 */

#include "scene.h"

#include "camera.h"
#include "drawer.h"
#include "MathLib.h"
#include "mesh.h"
#include "walker.h"

#include <stdlib.h>

static float global_time;
static float time_endgame;
static Maze* maze;
static Mesh *maze_mesh, *plane, *pyramid;
static Walker* walker;
static Texture wall_texture, ceiling_texture, floor_texture;
static Program textured_program, twister_program;

#define WALL_GROW_TIME 2.0

enum GameState { GAME_STARTING, GAME_RUNNING, GAME_ENDING };
static enum GameState game_state;

static void camera_update_pos(float pos[3]);
static void finish();
static void clean_up();
static void new_game();
static void draw_scene();
static void draw_models();
static void draw_ceiling();
static void draw_floor();
static void draw_walls();
static void draw_twisters();

void scene_init() {
  wall_texture = drawer_load_texture("wall.jpg");
  ceiling_texture = drawer_load_texture("ceiling.jpg");
  floor_texture = drawer_load_texture("floor.jpg");

  textured_program = drawer_create_program("textured.vert.glsl", "textured.frag.glsl");
  twister_program = drawer_create_program("twister.vert.glsl", "twister.frag.glsl");

  pyramid = mesh_create_pyramid(0.2);

  drawer_effect_init();

  new_game();
}

void scene_quit() {
  drawer_effect_shutdown();
}

void scene_update(float time_passed) {
  global_time += time_passed;

  if (global_time > WALL_GROW_TIME && game_state == GAME_STARTING) game_state = GAME_RUNNING;
  if ((global_time - time_endgame) > WALL_GROW_TIME && game_state == GAME_ENDING) new_game();

  if (game_state == GAME_RUNNING) walker_step(walker, time_passed);
}

void scene_draw() {
  draw_scene();
}

static void camera_update_pos(float pos[3]) {
  pos[1] = 0.5;
  camera_set_position(pos);
}

static void finish() {
  time_endgame = global_time;
  game_state = GAME_ENDING;
}

static void clean_up() {
  if (maze) maze_free(maze);
  if (maze_mesh) mesh_free(maze_mesh);
  if (walker) free(walker);
  if (plane) mesh_free(plane);
}

static void new_game() {
  clean_up();
  maze = maze_generate(10, 10);
  maze_print(maze);
  maze_mesh = mesh_create_maze(maze);
  plane = mesh_create_quad((float)maze->width, (float)maze->height);
  int start[2] = {0, 0};
  walker = walker_create(maze, start, DOWN, camera_update_pos, camera_set_rotation, finish);
  game_state = GAME_STARTING;
  time_endgame = global_time = 0.0;
}

static void draw_scene() {
  if (!drawer_effect_is_enabled()) {
    drawer_use_rendertarget(DRAWER_WINDOW_RENDERTARGET, 1);
    draw_models();
  } else {
    drawer_effect_begin_frame();
    draw_models();
    Texture effect_result = drawer_effect_apply();
    drawer_use_rendertarget(DRAWER_WINDOW_RENDERTARGET, 1);
    drawer_effect_render_to_screen(effect_result);
  }
}

static void draw_models() {
  float view[16];
  camera_get_matrix(view);
  drawer_modelview_set(view);
  
  drawer_effect_set_view_matrix(view);

  draw_floor();
  drawer_modelview_set(view);

  draw_ceiling();
  drawer_modelview_set(view);

  draw_walls();
  drawer_modelview_set(view);

  draw_twisters();
  drawer_modelview_set(view);
}

static void draw_ceiling() {
  float model[16];
  create_identity_m4(model);
  translate_m4(model, 0.0, 1.0, 0.0);
  
  float view[16], mv[16];
  drawer_modelview_get(view);
  copy_m4_m4(mv, view);
  mul_m4_m4(mv, model);
  drawer_modelview_set(mv);
  
  drawer_effect_store_model_matrix(model);

  drawer_use_program(textured_program);
  drawer_use_texture(ceiling_texture, 0, "Diffuse");
  drawer_draw_mesh(plane);
}

static void draw_floor() {
  float model[16];
  create_identity_m4(model);
  
  drawer_effect_store_model_matrix(model);

  drawer_use_program(textured_program);
  drawer_use_texture(floor_texture, 0, "Diffuse");
  drawer_draw_mesh(plane);
}

static void draw_walls() {
  float model[16];
  create_identity_m4(model);
  
  if (game_state == GAME_STARTING)
    scale_m4(model, 1.0, global_time / WALL_GROW_TIME, 1.0);
  else if (game_state == GAME_ENDING)
    scale_m4(model, 1.0, 1.0 - ((global_time - time_endgame) / WALL_GROW_TIME), 1.0);
  
  float view[16], mv[16];
  drawer_modelview_get(view);
  copy_m4_m4(mv, view);
  mul_m4_m4(mv, model);
  drawer_modelview_set(mv);
  
  drawer_effect_store_model_matrix(model);

  drawer_use_program(textured_program);
  drawer_use_texture(wall_texture, 0, "Diffuse");
  drawer_draw_mesh(maze_mesh);
}

static void draw_twisters() {
  float view[16];
  drawer_modelview_get(view);

  drawer_use_program(twister_program);
  
  unsigned int i;
  for (i = 0; i < maze->height * maze->width; i++) {
    Cell* cell = &maze->cells[i];
    if (cell->object == OBJ_TWISTER) {
      float model[16];
      create_identity_m4(model);
      translate_m4(model, cell->x + 0.5, 0.5, cell->y + 0.5);
      rotate_m4(model, global_time * 50.0, 0.0, 1.0, 0.0);
      rotate_m4(model, global_time * 35, 1.0, 0.0, 0.0);
      
      float mv[16];
      copy_m4_m4(mv, view);
      mul_m4_m4(mv, model);
      drawer_modelview_set(mv);
      
      drawer_effect_store_model_matrix(model);
      
      drawer_draw_mesh(pyramid);
    }
  }
}
