#pragma once

#include "engine/Prelude.hpp"
#include <stack>
#include <vector>

namespace engine::gl {

class GlTextureUnits final {

public:
#define Self GlTextureUnits
    explicit Self() noexcept     = delete;
    ~Self() noexcept             = delete;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

    static void Initialize();
    static auto IsInitialized [[nodiscard]] () -> bool { return isInitialized; };

    static auto MaxSlot [[nodiscard]] () -> size_t;
    static void BindSampler(size_t slotIdx, GLuint sampler);
    static void Bind2D(size_t slotIdx, GLuint texture);
    static void Bind2DArray(size_t slotIdx, GLuint texture);
    static void BindCubemap(size_t slotIdx, GLuint texture);
    static void BindCubemapArray(size_t slotIdx, GLuint texture);
    static void BeginStateSnapshot();
    static void EndStateSnapshot();
    static void RestoreState();

private:
    enum class SnapshotObjectType { TEXTURE, SAMPLER };
    struct TextureUnitSnapshot final {
        size_t slotIdx                = 0xDEADDEAD;
        GLenum textureType            = 0xDEADDEAD;
        SnapshotObjectType objectType = SnapshotObjectType::TEXTURE;
        GLuint oldObject              = 0xDEADDEAD;
    };

    static void BindTexture(size_t slotIdx, GLenum textureType, GLuint texture);
    static void DiscardSnapshot();

    static bool isInitialized;
    static bool isRecordingSnapshot;
    static std::vector<GLuint> currentBindings;
    static std::vector<GLuint> currentSamplerBindings;
    static std::stack<TextureUnitSnapshot> stateSnapshot;
};

} // namespace engine::gl
