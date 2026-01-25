#include "effect_c.h"

#include "effect.hpp"   // your Effect / EffectInputData
#include "image.hpp"    // Texture definition (GLuint wrapper)

#include <new>          // std::nothrow
#include <cstring>      // std::memcpy

// Opaque handle definition (C-visible as an incomplete type in the header)
struct effect_handle_t {
    Effect* ptr = nullptr;
};

// Helpers: convert between C matrix representation and glm::mat4.
// glm::mat4 is column-major and contiguous; effect_mat4 is defined to match that.
static inline const glm::mat4* as_glm_mat4_ptr(const effect_mat4* m) {
    return reinterpret_cast<const glm::mat4*>(m);
}

static inline glm::mat4* as_glm_mat4_ptr_mut(effect_mat4* m) {
    return reinterpret_cast<glm::mat4*>(m);
}

static inline Texture make_tex(effect_texture_t id) {
    Texture t;
    t.id = static_cast<GLuint>(id);
    return t;
}

static inline effect_texture_t tex_id(Texture t) {
    return static_cast<effect_texture_t>(static_cast<GLuint>(t));
}

effect_handle_t* effect_create(void) {
    effect_handle_t* h = new (std::nothrow) effect_handle_t();
    if (!h) return nullptr;

    h->ptr = new (std::nothrow) Effect();
    if (!h->ptr) {
        delete h;
        return nullptr;
    }
    return h;
}

void effect_destroy(effect_handle_t* handle) {
    if (!handle) return;
    delete handle->ptr;
    handle->ptr = nullptr;
    delete handle;
}

void effect_init(effect_handle_t* handle, int width, int height) {
    if (!handle || !handle->ptr) return;
    handle->ptr->Init(width, height);
}

void effect_shutdown(effect_handle_t* handle) {
    if (!handle || !handle->ptr) return;
    handle->ptr->Destroy();
}

effect_texture_t effect_apply(effect_handle_t* handle, const effect_input_data_t* in, float dt) {
    if (!handle || !handle->ptr || !in) return 0;

    EffectInputData cpp{};
    cpp.currIdTex = make_tex(in->curr_id_tex);

    cpp.reproject = (in->reproject != 0);
    cpp.prevIdTex = make_tex(in->prev_id_tex);
    cpp.prevDepthTex = make_tex(in->prev_depth_tex);

    // EffectInputData expects glm::mat4* (non-const pointers), but the effect should treat them as read-only.
    // We safely cast away const for API compatibility.
    cpp.prevCurrProj = in->prev_curr_proj ? const_cast<glm::mat4*>(as_glm_mat4_ptr(in->prev_curr_proj)) : nullptr;
    cpp.prevCurrView = in->prev_curr_view ? const_cast<glm::mat4*>(as_glm_mat4_ptr(in->prev_curr_view)) : nullptr;

    cpp.currInd = in->curr_ind;

    cpp.flow = (in->flow != 0);
    cpp.prevFlowTex = make_tex(in->prev_flow_tex);

    // modelMats is std::span<glm::mat4[2]>
    // In C we pass model_mats as an array of (count * 2) matrices.
    // We create a span over glm::mat4[2] by reinterpreting the pointer.
    if (in->model_mats && in->model_mats_count > 0) {
        auto mats2 = reinterpret_cast<glm::mat4(*)[2]>(const_cast<glm::mat4*>(as_glm_mat4_ptr(in->model_mats)));
        cpp.modelMats = std::span<glm::mat4[2]>(mats2, static_cast<size_t>(in->model_mats_count));
    }
    else {
        cpp.modelMats = std::span<glm::mat4[2]>();
    }

    Texture out = handle->ptr->Apply(cpp, dt);
    return tex_id(out);
}

void effect_clear_buffers(effect_handle_t* handle) {
    if (!handle || !handle->ptr) return;
    handle->ptr->ClearBuffers();
}

void effect_clear_acc(effect_handle_t* handle) {
    if (!handle || !handle->ptr) return;
    handle->ptr->ClearAcc();
}

void effect_on_resize(effect_handle_t* handle, int width, int height) {
    if (!handle || !handle->ptr) return;
    handle->ptr->OnResize(width, height);
}

int effect_get_width(const effect_handle_t* handle) {
    if (!handle || !handle->ptr) return 0;
    return handle->ptr->GetWidth();
}

int effect_get_height(const effect_handle_t* handle) {
    if (!handle || !handle->ptr) return 0;
    return handle->ptr->GetHeight();
}

// Parameters

float effect_get_scroll_speed_factor(const effect_handle_t* handle) {
    if (!handle || !handle->ptr) return 0.0f;
    return handle->ptr->scrollSpeedFactor;
}

void effect_set_scroll_speed_factor(effect_handle_t* handle, float v) {
    if (!handle || !handle->ptr) return;
    handle->ptr->scrollSpeedFactor = v;
}

int effect_get_acc_reset_interval(const effect_handle_t* handle) {
    if (!handle || !handle->ptr) return 0;
    return handle->ptr->accResetInterval;
}

void effect_set_acc_reset_interval(effect_handle_t* handle, int v) {
    if (!handle || !handle->ptr) return;
    handle->ptr->accResetInterval = v;
}

int effect_get_downscale_factor(const effect_handle_t* handle) {
    if (!handle || !handle->ptr) return 0;
    return handle->ptr->downscaleFactor;
}

void effect_set_downscale_factor(effect_handle_t* handle, int v) {
    if (!handle || !handle->ptr) return;
    handle->ptr->downscaleFactor = v;
}

int effect_get_paused(const effect_handle_t* handle) {
    if (!handle || !handle->ptr) return 0;
    return handle->ptr->paused ? 1 : 0;
}

void effect_set_paused(effect_handle_t* handle, int v) {
    if (!handle || !handle->ptr) return;
    handle->ptr->paused = (v != 0);
}

int effect_get_show_acc(const effect_handle_t* handle) {
    if (!handle || !handle->ptr) return 0;
    return handle->ptr->showAcc ? 1 : 0;
}

void effect_set_show_acc(effect_handle_t* handle, int v) {
    if (!handle || !handle->ptr) return;
    handle->ptr->showAcc = (v != 0);
}

effect_handle_t* effect_get_singleton(void) {
    Effect* s = Effect::Get();
    if (!s) return nullptr;

    // Wrap the singleton pointer without taking ownership.
    // Important: caller must NOT call effect_destroy() on this handle, or it will delete the singleton.
    // To keep the API safe, we return a static handle that is never freed.
    static effect_handle_t singleton_handle;
    singleton_handle.ptr = s;
    return &singleton_handle;
}