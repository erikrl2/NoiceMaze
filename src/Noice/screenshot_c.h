#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <GL/glew.h>
#include <stdbool.h>

typedef struct ScreenshotC ScreenshotC;

enum ScreenshotMethodC {
  SCREENSHOT_METHOD_AVERAGE = 0,
  SCREENSHOT_METHOD_ABSDIFFSUM = 1,
  SCREENSHOT_METHOD_COUNT = 2,
};

ScreenshotC* screenshot_create(void);
void screenshot_destroy(ScreenshotC* s);

void screenshot_init(ScreenshotC* s, int width, int height);
void screenshot_shutdown(ScreenshotC* s);

void screenshot_update(ScreenshotC* s, GLuint source);
void screenshot_begin(ScreenshotC* s);
void screenshot_reset(ScreenshotC* s);
void screenshot_save_png(ScreenshotC* s);

bool screenshot_is_active(ScreenshotC* s);
bool screenshot_has_result(ScreenshotC* s);
GLuint screenshot_get_result(ScreenshotC* s);

void screenshot_set_method(ScreenshotC* s, int method);
void screenshot_set_target_frames(ScreenshotC* s, int targetFrames);
void screenshot_set_gain(ScreenshotC* s, float gain);
void screenshot_set_gamma(ScreenshotC* s, float gamma);
void screenshot_set_basename(ScreenshotC* s, const char* baseName);

#ifdef __cplusplus
} // extern "C"
#endif
