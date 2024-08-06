#include "engine/GlTextureUnits.hpp"
#include "engine/GlCapabilities.hpp"

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

constexpr auto TextureTypeToOffset(GLenum textureType) -> size_t {
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

bool GlTextureUnits::isInitialized       = false;
bool GlTextureUnits::isRecordingSnapshot = false;
std::vector<GLuint> GlTextureUnits::currentBindings{};
std::vector<GLuint> GlTextureUnits::currentSamplerBindings{};
std::stack<GlTextureUnits::TextureUnitSnapshot> GlTextureUnits::stateSnapshot{};

void GlTextureUnits::Initialize() {
    if (isInitialized) { return; }
    isRecordingSnapshot = false;

    assert(GlCapabilities::IsInitialized());
    currentBindings.resize(GlCapabilities::maxTextureUnits * NUM_TEXTURE_TYPES);
    std::fill(currentBindings.begin(), currentBindings.end(), GL_NONE);
    currentSamplerBindings.resize(GlCapabilities::maxTextureUnits);
    std::fill(currentSamplerBindings.begin(), currentSamplerBindings.end(), GL_NONE);

    DiscardSnapshot();

    isInitialized = true;
}

auto GlTextureUnits::MaxSlot() -> size_t {
    assert(GlCapabilities::IsInitialized());
    return GlCapabilities::maxTextureUnits;
}

void GlTextureUnits::BindTexture(size_t slotIdx, GLenum textureType, GLuint texture) {
    size_t bindingOffset = slotIdx * NUM_TEXTURE_TYPES + TextureTypeToOffset(textureType);
    if (isRecordingSnapshot) {
        stateSnapshot.push(TextureUnitSnapshot{
            .slotIdx     = slotIdx,
            .textureType = textureType,
            .objectType  = SnapshotObjectType::TEXTURE,
            .oldObject   = currentBindings[bindingOffset]});
    }
    GLCALL(glActiveTexture(GL_TEXTURE0 + slotIdx));
    GLCALL(glBindTexture(textureType, texture));
    currentBindings[bindingOffset] = texture;
}

void GlTextureUnits::BindSampler(size_t slotIdx, GLuint sampler) {
    if (isRecordingSnapshot) {
        stateSnapshot.push(TextureUnitSnapshot{
            .slotIdx     = slotIdx,
            .textureType = GL_NONE,
            .objectType  = SnapshotObjectType::SAMPLER,
            .oldObject   = currentSamplerBindings[slotIdx]});
    }
    GLCALL(glBindSampler(GL_TEXTURE0 + slotIdx, sampler));
    currentSamplerBindings[slotIdx] = sampler;
}

void GlTextureUnits::Bind2D(size_t slotIdx, GLuint texture) { BindTexture(slotIdx, GL_TEXTURE_2D, texture); }

void GlTextureUnits::Bind2DArray(size_t slotIdx, GLuint texture) { BindTexture(slotIdx, GL_TEXTURE_2D_ARRAY, texture); }

void GlTextureUnits::BindCubemap(size_t slotIdx, GLuint texture) { BindTexture(slotIdx, GL_TEXTURE_CUBE_MAP, texture); }

void GlTextureUnits::BindCubemapArray(size_t slotIdx, GLuint texture) {
    BindTexture(slotIdx, GL_TEXTURE_CUBE_MAP_ARRAY, texture);
}

void GlTextureUnits::BeginStateSnapshot() {
    assert(!isRecordingSnapshot);
    DiscardSnapshot();
    isRecordingSnapshot = true;
}

void GlTextureUnits::EndStateSnapshot() {
    assert(isRecordingSnapshot);
    isRecordingSnapshot = false;
}

void GlTextureUnits::RestoreState() {
    assert(!isRecordingSnapshot);
    isRecordingSnapshot = false;

    while (!stateSnapshot.empty()) {
        auto const& unitState = stateSnapshot.top();
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
        stateSnapshot.pop();
    }
}

void GlTextureUnits::DiscardSnapshot() {
    while (!stateSnapshot.empty()) {
        stateSnapshot.pop();
    }
}

} // namespace engine::gl