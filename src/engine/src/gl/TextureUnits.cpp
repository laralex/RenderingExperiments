#include "engine/gl/TextureUnits.hpp"
#include "engine/Precompiled.hpp"

#include "engine_private/Prelude.hpp"

#include <utility>

namespace {

enum TextureTypeOffset : size_t {
    TEXTURE_2D = 0,
    TEXTURE_2D_ARRAY,
    TEXTURE_CUBEMAP,
    TEXTURE_CUBEMAP_ARRAY,
    TEXTURE_1D,
    TEXTURE_1D_ARRAY,
    TEXTURE_2D_MULTISAMLE,
    TEXTURE_2D_MULTISAMLE_ARRAY,
    TEXTURE_3D,
    TEXTURE_BUFFER,
    NUM_TEXTURE_TYPES,
};

constexpr auto TextureTypeToOffset [[nodiscard]] (GLenum textureType) -> size_t {
    // clang-format off
    switch (textureType) {
    case GL_TEXTURE_2D: return TEXTURE_2D;
    case GL_TEXTURE_2D_ARRAY: return TEXTURE_2D_ARRAY;
    case GL_TEXTURE_CUBE_MAP: return TEXTURE_CUBEMAP;
    case GL_TEXTURE_CUBE_MAP_ARRAY: return TEXTURE_CUBEMAP_ARRAY;
    case GL_TEXTURE_1D: return TEXTURE_1D;
    case GL_TEXTURE_1D_ARRAY: return TEXTURE_1D_ARRAY;
    case GL_TEXTURE_2D_MULTISAMPLE: return TEXTURE_2D_MULTISAMLE;
    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY: return TEXTURE_2D_MULTISAMLE_ARRAY;
    case GL_TEXTURE_3D: return TEXTURE_3D;
    case GL_TEXTURE_BUFFER: return TEXTURE_BUFFER;
    }
    // clang-format on
    XLOGE("Bug, TextureTypeToOffset accepts valid GL_TEXTURE enum, given={}", textureType);
    std::terminate();
    return 0xDEADDEAD;
}

} // namespace

namespace engine::gl {

ENGINE_EXPORT void GlTextureUnits::Initialize(GlContext const& gl) {
    if (isInitialized_) { return; }
    isRecordingSnapshot_ = false;

    currentBindings_.resize(gl.Capabilities().maxTextureUnits * NUM_TEXTURE_TYPES);
    std::fill(currentBindings_.begin(), currentBindings_.end(), GL_NONE);
    currentSamplerBindings_.resize(gl.Capabilities().maxTextureUnits);
    std::fill(currentSamplerBindings_.begin(), currentSamplerBindings_.end(), GL_NONE);

    DiscardSnapshot();

    isInitialized_ = true;
}

ENGINE_EXPORT void GlTextureUnits::BindTexture(size_t slotIdx, GLenum textureType, GLuint texture) {
    size_t bindingOffset = slotIdx * NUM_TEXTURE_TYPES + TextureTypeToOffset(textureType);
    if (isRecordingSnapshot_) {
        stateSnapshot_.push(TextureUnitSnapshot{
            .slotIdx     = slotIdx,
            .textureType = textureType,
            .objectType  = SnapshotObjectType::TEXTURE,
            .oldObject   = currentBindings_[bindingOffset]});
    }
    GLCALL(glActiveTexture(GL_TEXTURE0 + slotIdx));
    GLCALL(glBindTexture(textureType, texture));
    currentBindings_[bindingOffset] = texture;
}

ENGINE_EXPORT void GlTextureUnits::BindSampler(size_t slotIdx, GLuint sampler) {
    if (isRecordingSnapshot_) {
        stateSnapshot_.push(TextureUnitSnapshot{
            .slotIdx     = slotIdx,
            .textureType = GL_NONE,
            .objectType  = SnapshotObjectType::SAMPLER,
            .oldObject   = currentSamplerBindings_[slotIdx]});
    }
    GLCALL(glBindSampler(slotIdx, sampler));
    currentSamplerBindings_[slotIdx] = sampler;
}

ENGINE_EXPORT void GlTextureUnits::Bind2D(size_t slotIdx, GLuint texture) { BindTexture(slotIdx, GL_TEXTURE_2D, texture); }

ENGINE_EXPORT void GlTextureUnits::Bind2DArray(size_t slotIdx, GLuint texture) { BindTexture(slotIdx, GL_TEXTURE_2D_ARRAY, texture); }

ENGINE_EXPORT void GlTextureUnits::BindCubemap(size_t slotIdx, GLuint texture) { BindTexture(slotIdx, GL_TEXTURE_CUBE_MAP, texture); }

ENGINE_EXPORT void GlTextureUnits::BindCubemapArray(size_t slotIdx, GLuint texture) {
    BindTexture(slotIdx, GL_TEXTURE_CUBE_MAP_ARRAY, texture);
}

ENGINE_EXPORT void GlTextureUnits::BeginStateSnapshot() {
    assert(!isRecordingSnapshot_);
    DiscardSnapshot();
    isRecordingSnapshot_ = true;
}

ENGINE_EXPORT void GlTextureUnits::EndStateSnapshot() {
    assert(isRecordingSnapshot_);
    isRecordingSnapshot_ = false;
}

ENGINE_EXPORT void GlTextureUnits::RestoreState() {
    assert(!isRecordingSnapshot_);
    isRecordingSnapshot_ = false;

    while (!stateSnapshot_.empty()) {
        auto const& unitState = stateSnapshot_.top();
        switch (unitState.objectType) {
        case GlTextureUnits::SnapshotObjectType::SAMPLER:
            // XLOG("GlTextureUnits::RestoreState slot={} sampler={}", unitState.slotIdx, unitState.oldObject);
            BindSampler(unitState.slotIdx, unitState.oldObject);
            break;
        case GlTextureUnits::SnapshotObjectType::TEXTURE:
            // XLOG("GlTextureUnits::RestoreState slot={} texture={}", unitState.slotIdx, unitState.oldObject);
            BindTexture(unitState.slotIdx, unitState.textureType, unitState.oldObject);
            break;
        }
        stateSnapshot_.pop();
    }
}

ENGINE_EXPORT void GlTextureUnits::DiscardSnapshot() {
    while (!stateSnapshot_.empty()) {
        stateSnapshot_.pop();
    }
}

} // namespace engine::gl