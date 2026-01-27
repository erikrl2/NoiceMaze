/* Copyright (C) 2016 ultitech - All Rights Reserved
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE', which is part of this source code package.
 */

#include "drawer.h"

#include "file.h"
#include "MathLib.h"
#include "mesh.h"
#include "window.h"
#include "noice/effect_c.h"

#include <FreeImage.h>
#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static float mat_projection[16], mat_modelview[16];
static GLuint current_program;
static char vbo_bound = 0;
static float global_time = 0.0;
static int screen_size[2];
static int viewport_position[2], viewport_size[2];

static GLuint pp_vertex_shader, pp_fragment_shader, pp_program;
static Mesh* screen_square_mesh;

static struct {
  EffectC* effect;
  char enabled;
  
  GLuint fbo[2];
  GLuint depth_tex[2];
  GLuint id_tex[2];
  
  float proj_mats[2][16];
  float view_mats[2][16];
  
  float (*model_mats)[2][16];
  size_t model_mat_capacity;
  size_t model_mat_count;
  size_t prev_model_mat_count;
  
  int curr_frame_idx; // 0 or 1
  int current_object_id;
} effect_data = {0};

static void update_uniforms();
static GLuint create_shader(GLenum type, char* filename);
static GLuint create_program(GLuint vertex_shader, GLuint fragment_shader);
static int uniform_exists(char* name, GLint* location);
static GLuint create_texture(GLsizei width, GLsizei height, GLenum format, GLfloat* data);
static void set_viewport(int posx, int posy, int sizex, int sizey);
static void handle_keypress(SDL_Keycode key);
static void effect_create_framebuffers();
static void effect_destroy_framebuffers();
static void effect_ensure_model_mat_capacity(size_t required);

void drawer_init() {
  window_add_keypress_handler(handle_keypress);

  FreeImage_Initialise(0);

  glewInit();

  drawer_print_glinfo();

  glClearColor(0.0, 0.0, 0.0, 0.0);
  glEnable(GL_DEPTH_TEST);

  window_get_size(screen_size);
  create_perspective_m4(mat_projection, 90.0, (float)screen_size[0] / (float)screen_size[1], 0.1, 100.0);
  set_viewport(0, 0, screen_size[0], screen_size[1]);

  //glActiveTexture(GL_TEXTURE0);

  pp_vertex_shader = create_shader(GL_VERTEX_SHADER, "pp.vert.glsl");
  pp_fragment_shader = create_shader(GL_FRAGMENT_SHADER, "pp.frag.glsl");
  pp_program = create_program(pp_vertex_shader, pp_fragment_shader);

  screen_square_mesh = mesh_create_screen_square();
}

void drawer_quit() {
  FreeImage_DeInitialise();
}

void drawer_modelview_set(float matrix[16]) {
  copy_m4_m4(mat_modelview, matrix);
  update_uniforms();
}

void drawer_modelview_get(float matrix[16]) {
  copy_m4_m4(matrix, mat_modelview);
}

Program drawer_create_program(char* vertex_filename, char* fragment_filename) {
  GLuint vertex_shader, fragment_shader, program;

  vertex_shader = create_shader(GL_VERTEX_SHADER, vertex_filename);
  fragment_shader = create_shader(GL_FRAGMENT_SHADER, fragment_filename);
  program = create_program(vertex_shader, fragment_shader);

  return program;
}

void drawer_use_program(Program program) {
  glUseProgram(program);
  current_program = program;
  update_uniforms();
}

Texture drawer_load_texture(char* filename) {
  FIBITMAP* bmp = FreeImage_Load(FIF_JPEG, file_resource(filename, RESOURCE_TEXTURE), 0);

  int image_size[2];
  image_size[0] = FreeImage_GetWidth(bmp);
  image_size[1] = FreeImage_GetHeight(bmp);

  GLfloat* image_data = malloc(sizeof(GLfloat) * image_size[0] * image_size[1] * 3);
  int x, y;
  for (x = 0; x < image_size[0]; x++)
    for (y = 0; y < image_size[1]; y++) {
      RGBQUAD color;
      FreeImage_GetPixelColor(bmp, x, y, &color);
      GLfloat* pixel = &image_data[(x + y * image_size[0]) * 3];
      pixel[0] = color.rgbRed / 255.0;
      pixel[1] = color.rgbGreen / 255.0;
      pixel[2] = color.rgbBlue / 255.0;
    }

  GLuint texture = create_texture(image_size[0], image_size[1], GL_RGB, image_data);

  free(image_data);
  FreeImage_Unload(bmp);

  return texture;
}

void drawer_use_texture(Texture texture, unsigned int texture_unit, char* uniform_name) {
  glActiveTexture(GL_TEXTURE0 + texture_unit);
  glBindTexture(GL_TEXTURE_2D, texture);
  GLint location;
  if (uniform_exists(uniform_name, &location)) glUniform1i(location, texture_unit);
}

void drawer_use_rendertarget_texture(Rendertarget target, unsigned int texture_unit, char* uniform_name) {
  GLuint texture;
  GLint location;

  glBindFramebuffer(GL_READ_FRAMEBUFFER, target);
  glGetFramebufferAttachmentParameteriv(
      GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, (GLint*)&texture
  );

  drawer_use_texture(texture, texture_unit, uniform_name);
}

Rendertarget drawer_create_rendertarget() {
  GLuint target, image, depth;

  glGenFramebuffers(1, &target);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target);

  glGenTextures(1, &image);
  glBindTexture(GL_TEXTURE_2D, image);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, screen_size[0], screen_size[1], 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, image, 0);

  glGenRenderbuffers(1, &depth);
  glBindRenderbuffer(GL_RENDERBUFFER, depth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, screen_size[0], screen_size[1]);
  glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

  glDrawBuffer(GL_COLOR_ATTACHMENT0);

  return target;
}

void drawer_use_rendertarget(Rendertarget target, char clear) {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target);

  if (clear) glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void drawer_draw_mesh(Mesh* mesh) {
  if (mesh->vbo) {
    MeshVBO* vbo = mesh->vbo;
    glBindVertexArray(vbo->vao);
    glDrawElements(GL_TRIANGLES, mesh->indices_count, GL_UNSIGNED_INT, NULL);
  }
}

void drawer_begin_scene(float time_passed) {
  global_time += time_passed;
}

void drawer_end_scene() {
  window_swap_buffers();
}

void drawer_create_mesh_vbo(Mesh* mesh) {
  MeshVBO* vbo = malloc(sizeof(MeshVBO));

  glGenVertexArrays(1, &vbo->vao);
  glBindVertexArray(vbo->vao);

  glGenBuffers(1, &vbo->vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vbo->vertex_buffer);
  glBufferData(
      GL_ARRAY_BUFFER,
      sizeof(GLfloat) * mesh_get_vertex_size(mesh->vertex_format) * mesh->vertices_count,
      mesh->data->vertices,
      GL_STATIC_DRAW
  );

  glGenBuffers(1, &vbo->index_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo->index_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh->indices_count, mesh->data->indices, GL_STATIC_DRAW);

  GLsizei stride = 0;
  int position_offset = 0, normal_offset = 0, texcoord_offset = 0;
  if (mesh->vertex_format & VERTEX_POSITION) {
    position_offset = stride;
    stride += 3;
  }
  if (mesh->vertex_format & VERTEX_NORMAL) {
    normal_offset = stride;
    stride += 3;
  }
  if (mesh->vertex_format & VERTEX_TEXCOORD) {
    texcoord_offset = stride;
    stride += 2;
  }
  stride *= sizeof(GLfloat);

  if (mesh->vertex_format & VERTEX_POSITION) {
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(intptr_t)(position_offset * sizeof(GLfloat)));
  }

  if (mesh->vertex_format & VERTEX_NORMAL) {
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(intptr_t)(normal_offset * sizeof(GLfloat)));
  }

  if (mesh->vertex_format & VERTEX_TEXCOORD) {
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(intptr_t)(texcoord_offset * sizeof(GLfloat)));
  }

  glBindVertexArray(0);
  mesh->vbo = vbo;
}

void drawer_free_mesh_vbo(MeshVBO* vbo) {
  glDeleteBuffers(1, &vbo->vertex_buffer);
  glDeleteBuffers(1, &vbo->index_buffer);
  free(vbo);
}

void drawer_screenshot() {
  // TODO: use my cpp accum screenshot tool
}

void drawer_print_glinfo() {
  printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
  printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
}

static GLuint create_shader(GLenum type, char* filename) {
  GLuint shader = glCreateShader(type);
  GLchar* shader_source = file_text(file_resource(filename, RESOURCE_SHADER));
  if (shader_source == NULL) {
    printf("Failed to load shader file: %s\n", filename);
    glDeleteShader(shader);
    return 0;
  }
  glShaderSource(shader, 1, (const GLchar**)&shader_source, NULL);
  free(shader_source);

  glCompileShader(shader);

  GLint compiled;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (compiled == GL_FALSE) {
    printf("Failed to compile %s:\n", filename);
    GLint log_length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
    GLchar* log = malloc(log_length + 1);
    glGetShaderInfoLog(shader, log_length, NULL, log);
    printf("%s\n", log);
    free(log);
  }
  return shader;
}

static GLuint create_program(GLuint vertex_shader, GLuint fragment_shader) {
  GLuint program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  GLint linked;
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  if (linked == GL_FALSE) {
    printf("Failed to link program:\n");
    GLint log_length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
    GLchar* log = malloc(log_length + 1);
    glGetProgramInfoLog(program, log_length, NULL, log);
    printf("%s\n", log);
    free(log);
  }
  return program;
}

static int uniform_exists(char* name, GLint* location) {
  *location = glGetUniformLocation(current_program, name);
  return *location == -1 ? 0 : 1;
}

static GLuint create_texture(GLsizei width, GLsizei height, GLenum format, GLfloat* data) {
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_FLOAT, data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  return texture;
}

static void update_uniforms() {
  if (current_program == 0) return;

  GLint location;
  if (uniform_exists("MVMatrix", &location)) glUniformMatrix4fv(location, 1, GL_FALSE, mat_modelview);
  if (uniform_exists("MVPMatrix", &location)) {
    float mvp[16];
    copy_m4_m4(mvp, mat_projection);
    mul_m4_m4(mvp, mat_modelview);
    glUniformMatrix4fv(location, 1, GL_FALSE, mvp);
  }
  
  if (effect_data.enabled && effect_data.effect) {
    if (uniform_exists("ObjectID", &location))
      glUniform1i(location, effect_data.current_object_id);
  }
}

static void set_viewport(int posx, int posy, int sizex, int sizey) {
  viewport_position[0] = posx;
  viewport_position[1] = posy;
  viewport_size[0] = sizex;
  viewport_size[1] = sizey;
  glViewport(posx, posy, sizex, sizey);
}

static void handle_keypress(SDL_Keycode key) {
  if (key == SDLK_e) {
    drawer_effect_toggle();
  }
}

static void effect_create_framebuffers() {
  for (int i = 0; i < 2; i++) {
    glGenFramebuffers(1, &effect_data.fbo[i]);
    glBindFramebuffer(GL_FRAMEBUFFER, effect_data.fbo[i]);
    
    glGenTextures(1, &effect_data.id_tex[i]);
    glBindTexture(GL_TEXTURE_2D, effect_data.id_tex[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, screen_size[0], screen_size[1], 0, GL_RED_INTEGER, GL_INT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, effect_data.id_tex[i], 0);
    
    glGenTextures(1, &effect_data.depth_tex[i]);
    glBindTexture(GL_TEXTURE_2D, effect_data.depth_tex[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, screen_size[0], screen_size[1], 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, effect_data.depth_tex[i], 0);
    
    GLenum draw_buffers[] = {GL_NONE, GL_COLOR_ATTACHMENT0};
    glDrawBuffers(2, draw_buffers);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
      printf("Effect framebuffer %d incomplete: 0x%x\n", i, status);
    }
  }
  
  //glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void effect_destroy_framebuffers() {
  for (int i = 0; i < 2; i++) {
    if (effect_data.fbo[i]) glDeleteFramebuffers(1, &effect_data.fbo[i]);
    if (effect_data.depth_tex[i]) glDeleteTextures(1, &effect_data.depth_tex[i]);
    if (effect_data.id_tex[i]) glDeleteTextures(1, &effect_data.id_tex[i]);
  }
  memset(&effect_data.fbo, 0, sizeof(effect_data.fbo));
  memset(&effect_data.depth_tex, 0, sizeof(effect_data.depth_tex));
  memset(&effect_data.id_tex, 0, sizeof(effect_data.id_tex));
}

static void effect_ensure_model_mat_capacity(size_t required) {
  if (effect_data.model_mat_capacity >= required) return;
  
  size_t new_capacity = effect_data.model_mat_capacity == 0 ? 16 : effect_data.model_mat_capacity * 2;
  while (new_capacity < required) new_capacity *= 2;
  
  effect_data.model_mats = realloc(effect_data.model_mats, sizeof(float[2][16]) * new_capacity);
  effect_data.model_mat_capacity = new_capacity;
}

void drawer_effect_init() {
  if (effect_data.effect) return;
  
  effect_data.effect = effect_create();
  effect_data.enabled = 1;
  effect_data.curr_frame_idx = 0;
  effect_data.current_object_id = 0;
  
  effect_create_framebuffers();
  effect_init(effect_data.effect, screen_size[0], screen_size[1]);
  
  effect_data.model_mat_count = 0;
  effect_data.model_mat_capacity = 16;
  effect_data.model_mats = malloc(sizeof(float[2][16]) * effect_data.model_mat_capacity);
  
  printf("Noice effect initialized (toggle with 'E' key)\n");
}

void drawer_effect_shutdown() {
  if (!effect_data.effect) return;
  
  effect_shutdown(effect_data.effect);
  effect_destroy(effect_data.effect);
  effect_data.effect = NULL;
  
  effect_destroy_framebuffers();
  
  if (effect_data.model_mats) {
    free(effect_data.model_mats);
    effect_data.model_mats = NULL;
  }
  effect_data.model_mat_capacity = 0;
}

void drawer_effect_toggle() {
  if (!effect_data.effect) return;
  
  effect_data.enabled = !effect_data.enabled;
}

char drawer_effect_is_enabled() {
  return effect_data.effect && effect_data.enabled;
}

void drawer_effect_begin_frame() {
  if (!drawer_effect_is_enabled()) return;
  
  effect_data.curr_frame_idx = 1 - effect_data.curr_frame_idx;
  
  copy_m4_m4(effect_data.proj_mats[effect_data.curr_frame_idx], mat_projection);
  
  effect_data.prev_model_mat_count = effect_data.model_mat_count;
  effect_data.model_mat_count = 0;
  effect_data.current_object_id = -1;
  
  glBindFramebuffer(GL_FRAMEBUFFER, effect_data.fbo[effect_data.curr_frame_idx]);
  
  GLint clear_id = -1;
  glClearBufferiv(GL_COLOR, 1, &clear_id);

  glClear(GL_DEPTH_BUFFER_BIT);
}

void drawer_effect_set_view_matrix(float view[16]) {
  if (!drawer_effect_is_enabled()) return;
  
  int curr_idx = effect_data.curr_frame_idx;
  copy_m4_m4(effect_data.view_mats[curr_idx], view);
}

int drawer_effect_store_model_matrix(float model[16]) {
  if (!drawer_effect_is_enabled()) return -1;
  
  size_t idx = effect_data.model_mat_count;
  effect_ensure_model_mat_capacity(idx + 1);
  
  int curr_idx = effect_data.curr_frame_idx;
  int prev_idx = 1 - curr_idx;
  
  copy_m4_m4(effect_data.model_mats[idx][curr_idx], model);
  
  if (idx >= effect_data.prev_model_mat_count) {
    copy_m4_m4(effect_data.model_mats[idx][prev_idx], model);
  }
  
  effect_data.model_mat_count++;
  
  effect_data.current_object_id = (int)idx;
  update_uniforms();
  
  return (int)idx;
}

Texture drawer_effect_apply() {
  if (!drawer_effect_is_enabled()) return 0;
  
  int curr_idx = effect_data.curr_frame_idx;
  int prev_idx = 1 - curr_idx;
  
  EffectInputDataC input = {0};
  input.curr_id_tex = effect_data.id_tex[curr_idx];
  input.prev_id_tex = effect_data.id_tex[prev_idx];
  input.prev_depth_tex = effect_data.depth_tex[prev_idx];
  input.prev_curr_proj = (const float(*)[16])effect_data.proj_mats;
  input.prev_curr_view = (const float(*)[16])effect_data.view_mats;
  input.model_mats = (const float(*)[2][16])effect_data.model_mats;
  input.model_mat_count = effect_data.model_mat_count;
  input.curr_ind = curr_idx;
  
  GLuint result_tex = effect_apply(effect_data.effect, &input);
  
  //glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  return result_tex;
}

void drawer_effect_render_to_screen(Texture tex) {
  if (!tex) return;
  
  glDisable(GL_DEPTH_TEST);
  
  drawer_use_program(pp_program);
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);
  GLint location;
  if (uniform_exists("Image", &location)) glUniform1i(location, 0);
  
  drawer_draw_mesh(screen_square_mesh);
  
  glEnable(GL_DEPTH_TEST);
}
