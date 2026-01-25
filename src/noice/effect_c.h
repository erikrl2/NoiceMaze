#pragma once

// C API wrapper for Effect (effect.hpp)

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    // Opaque handle to the underlying C++ Effect instance.
    typedef struct effect_handle_t effect_handle_t;

    // OpenGL texture name (GLuint) represented in C.
    typedef uint32_t effect_texture_t;

    // Column-major 4x4 matrix, compatible with glm::mat4 memory layout.
    typedef struct effect_mat4 {
        float m[16];
    } effect_mat4;

    // C representation of EffectInputData.
    //
    // Notes:
    // - prev_curr_proj / prev_curr_view are optional pointers to matrices (can be NULL).
    // - model_mats points to an array of (model_mats_count * 2) matrices.
    //   Each logical entry i is two consecutive mat4s:
    //     model_mats[i*2 + 0], model_mats[i*2 + 1]
    typedef struct effect_input_data_t {
        effect_texture_t curr_id_tex;

        int reproject; // bool (0/1)
        effect_texture_t prev_id_tex;
        effect_texture_t prev_depth_tex;
        const effect_mat4* prev_curr_proj; // optional (NULL allowed)
        const effect_mat4* prev_curr_view; // optional (NULL allowed)

        const effect_mat4* model_mats; // array of length model_mats_count * 2
        int model_mats_count;          // number of [2]-mat blocks
        int curr_ind;

        int flow; // bool (0/1)
        effect_texture_t prev_flow_tex;
    } effect_input_data_t;

    // Lifecycle
    effect_handle_t* effect_create(void);
    void effect_destroy(effect_handle_t* handle);

    // Mirrors Effect::Init / Destroy (Destroy does not free the handle; effect_destroy does)
    void effect_init(effect_handle_t* handle, int width, int height);
    void effect_shutdown(effect_handle_t* handle);

    // Main
    effect_texture_t effect_apply(effect_handle_t* handle, const effect_input_data_t* in, float dt);

    // Utilities
    void effect_clear_buffers(effect_handle_t* handle);
    void effect_clear_acc(effect_handle_t* handle);
    void effect_on_resize(effect_handle_t* handle, int width, int height);

    // Queries
    int effect_get_width(const effect_handle_t* handle);
    int effect_get_height(const effect_handle_t* handle);

    // Parameters (mirrors public members)
    float effect_get_scroll_speed_factor(const effect_handle_t* handle);
    void effect_set_scroll_speed_factor(effect_handle_t* handle, float v);

    int effect_get_acc_reset_interval(const effect_handle_t* handle);
    void effect_set_acc_reset_interval(effect_handle_t* handle, int v);

    int effect_get_downscale_factor(const effect_handle_t* handle);
    void effect_set_downscale_factor(effect_handle_t* handle, int v);

    int effect_get_paused(const effect_handle_t* handle);
    void effect_set_paused(effect_handle_t* handle, int v);

    int effect_get_show_acc(const effect_handle_t* handle);
    void effect_set_show_acc(effect_handle_t* handle, int v);

    // Optional: access the singleton (Effect::Get()) if your code relies on it.
    // Returns NULL if the singleton hasn't been constructed.
    effect_handle_t* effect_get_singleton(void);

#ifdef __cplusplus
} // extern "C"
#endif