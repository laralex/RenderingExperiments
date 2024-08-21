#pragma once

#include "engine/Prelude.hpp"

namespace engine::gl {

class GpuProgram final {

public:
#define Self GpuProgram
    explicit Self() = default;
    ~Self() { Dispose(); };
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static auto Allocate [[nodiscard]] (GLuint vertexShader, GLuint fragmentShader, std::string_view name = {})
    -> std::optional<GpuProgram>;
    auto Id [[nodiscard]] () const -> GLuint { return programId_; }

private:
    void Dispose();
    GlHandle programId_{GL_NONE};

    friend class UniformCtx;
};

auto CompileShader [[nodiscard]] (GLenum shaderType, std::string_view code) -> GLuint;

} // namespace engine::gl
