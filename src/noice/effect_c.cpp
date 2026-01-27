#include "effect_c.h"

#include "effect.hpp"
#include "image.hpp"

#include <glm/glm.hpp>

#include <cassert>
#include <span>

struct EffectC {
  Effect eff;
};

extern "C" {

EffectC* effect_create(void) {
  return new EffectC{};
}

void effect_destroy(EffectC* e) {
  delete e;
}

void effect_init(EffectC* e, int width, int height) {
  assert(e);
  e->eff.Init(width, height);
}

void effect_shutdown(EffectC* e) {
  assert(e);
  e->eff.Destroy();
}

void effect_on_resize(EffectC* e, int width, int height) {
  assert(e);
  e->eff.OnResize(width, height);
}

GLuint effect_apply(EffectC* e, const EffectInputDataC* in) {
  assert(e && in);

  assert(in->prev_curr_proj && in->prev_curr_view);
  assert(in->model_mats || in->model_mat_count == 0);

  EffectInputData cpp{};
  cpp.currIdTex = Texture{in->curr_id_tex};
  cpp.prevIdTex = Texture{in->prev_id_tex};
  cpp.prevDepthTex = Texture{in->prev_depth_tex};
  cpp.currInd = in->curr_ind;

  static_assert(alignof(glm::mat4) == alignof(float));

  cpp.prevCurrProj = (const glm::mat4*)(in->prev_curr_proj);
  cpp.prevCurrView = (const glm::mat4*)(in->prev_curr_view);

  auto mats = (const glm::mat4(*)[2])(in->model_mats);
  cpp.modelMats = std::span<const glm::mat4[2]>(mats, in->model_mat_count);

  return e->eff.Apply(cpp);
}

void effect_set_acc_reset_interval(EffectC* e, int interval) {
  assert(e);
  e->eff.accResetInterval = interval;
}

void effect_set_show_acc(EffectC* e, bool show) {
  assert(e);
  e->eff.showAcc = show;
}

} // extern "C"
