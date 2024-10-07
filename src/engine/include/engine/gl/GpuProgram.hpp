#pragma once

#include "engine/Precompiled.hpp"
#include "engine/ShaderDefine.hpp"

namespace engine::gl {

class GpuProgram final {

public:
#define Self GpuProgram
    explicit Self() noexcept = default;
    ~Self() noexcept { Dispose(); };
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static auto Allocate
        [[nodiscard]] (GlContext& gl, GLuint vertexShader, GLuint fragmentShader, std::string_view name = {})
        -> std::optional<GpuProgram>;
    auto LinkGraphical [[nodiscard]] (GLuint vertexShader, GLuint fragmentShader, bool isRecompile = false) const
        -> bool;
    auto Id [[nodiscard]] () const -> GLuint { return programId_; }
    auto Type [[nodiscard]] () const -> GpuProgramType { return type_; }

private:
    void Dispose();
    GlHandle programId_ = GlHandle{GL_NONE};
    enum GpuProgramType type_     = GpuProgramType::GRAPHICAL;

    friend class UniformCtx;
};

} // namespace engine::gl
