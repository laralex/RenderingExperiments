#pragma once

#include <glad/gl.h>
#include <stack>
#include <vector>

namespace engine::gl {

class GlContext;

class GlTextureUnits final {

public:
#define Self GlTextureUnits
    explicit Self() noexcept     = default;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    void Initialize(GlContext const& gl);
    auto IsInitialized [[nodiscard]] () const -> bool { return isInitialized_; };

    void BindSampler(size_t slotIdx, GLuint sampler);
    void Bind2D(size_t slotIdx, GLuint texture);
    void Bind2DArray(size_t slotIdx, GLuint texture);
    void BindCubemap(size_t slotIdx, GLuint texture);
    void BindCubemapArray(size_t slotIdx, GLuint texture);
    void BeginStateSnapshot();
    void EndStateSnapshot();
    void RestoreState();

private:
    enum class SnapshotObjectType { TEXTURE, SAMPLER };
    struct TextureUnitSnapshot final {
        size_t slotIdx                = 0xDEADDEAD;
        GLenum textureType            = 0xDEADDEAD;
        SnapshotObjectType objectType = SnapshotObjectType::TEXTURE;
        GLuint oldObject              = 0xDEADDEAD;
    };

    void BindTexture(size_t slotIdx, GLenum textureType, GLuint texture);
    void DiscardSnapshot();

    bool isInitialized_;
    bool isRecordingSnapshot_;
    std::vector<GLuint> currentBindings_;
    std::vector<GLuint> currentSamplerBindings_;
    std::stack<TextureUnitSnapshot> stateSnapshot_;
};

} // namespace engine::gl
