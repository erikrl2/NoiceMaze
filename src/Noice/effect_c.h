#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <GL/glew.h>
#include <stdbool.h>

typedef struct EffectC EffectC;

typedef struct EffectInputDataC {
  GLuint curr_id_tex;
  GLuint prev_id_tex;
  GLuint prev_flow_tex;
  GLuint prev_depth_tex;

  const float (*prev_curr_proj)[16];
  const float (*prev_curr_view)[16];

  const float (*model_mats)[2][16];
  size_t model_mat_count;

  int curr_ind;
} EffectInputDataC;

EffectC* effect_create(void);
void effect_destroy(EffectC* e);

void effect_init(EffectC* e, int width, int height);
void effect_shutdown(EffectC* e);
void effect_on_resize(EffectC* e, int width, int height);
GLuint effect_apply(EffectC* e, const EffectInputDataC* in, float dt);

void effect_set_acc_reset_interval(EffectC* e, int interval);
void effect_set_show_acc(EffectC* e, bool show);

#ifdef __cplusplus
} // extern "C"
#endif
