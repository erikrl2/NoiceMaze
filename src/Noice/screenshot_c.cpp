#include "screenshot_c.h"

#include "screenshot.hpp"

#include <cassert>
#include <cstring>

struct ScreenshotC {
  Screenshot ss;
};

extern "C" {

ScreenshotC* screenshot_create(void) {
  return new ScreenshotC{};
}

void screenshot_destroy(ScreenshotC* s) {
  delete s;
}

void screenshot_init(ScreenshotC* s, int width, int height) {
  assert(s);
  s->ss.Init(width, height);
}

void screenshot_shutdown(ScreenshotC* s) {
  assert(s);
  s->ss.Destroy();
}

void screenshot_update(ScreenshotC* s, GLuint source) {
  assert(s);
  s->ss.Update(Texture{source});
}

void screenshot_begin(ScreenshotC* s) {
  assert(s);
  s->ss.Begin();
}

void screenshot_reset(ScreenshotC* s) {
  assert(s);
  s->ss.Reset();
}

void screenshot_save_png(ScreenshotC* s) {
  assert(s);
  s->ss.SavePNG();
}

bool screenshot_is_active(ScreenshotC* s) {
  assert(s);
  return s->ss.IsActive();
}

bool screenshot_has_result(ScreenshotC* s) {
  assert(s);
  return s->ss.HasResult();
}

GLuint screenshot_get_result(ScreenshotC* s) {
  assert(s);
  return (GLuint)s->ss.GetResult();
}

void screenshot_set_method(ScreenshotC* s, int method) {
  assert(s);
  if (method < 0 || method >= (int)Screenshot::Method::Count) return;
  s->ss.options.method = (Screenshot::Method)method;
}

void screenshot_set_target_frames(ScreenshotC* s, int targetFrames) {
  assert(s);
  s->ss.options.targetFrames = targetFrames;
}

void screenshot_set_gain(ScreenshotC* s, float gain) {
  assert(s);
  s->ss.options.gain = gain;
}

void screenshot_set_gamma(ScreenshotC* s, float gamma) {
  assert(s);
  s->ss.options.gamma = gamma;
}

void screenshot_set_basename(ScreenshotC* s, const char* baseName) {
  assert(s);
  s->ss.options.baseName = baseName ? baseName : "";
}

} // extern "C"
