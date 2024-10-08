#pragma once

// #include "engine/Precompiled.hpp"
#include "engine/gl/Common.hpp"
#include "engine/CpuView.hpp"
#include <glad/gl.h>

namespace engine::gl {

class GlContext;

class GpuBuffer final {

public:
#define Self GpuBuffer
    explicit Self() noexcept = default;
    ~Self() noexcept { Dispose(); };
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    enum Access : uint32_t {
        NONE          = 0x0,
        CLIENT_READ   = 0x1,
        CLIENT_UPDATE = 0x2,
    };

    static auto Allocate(
        GlContext& gl, GLenum targetType, Access access, CpuMemory<GLvoid const> data, std::string_view name = {})
        -> GpuBuffer;
    auto Id [[nodiscard]] () const -> GLuint { return bufferId_; }
    void Fill(CpuMemory<GLvoid const> cpuData, GLintptr gpuByteOffset = 0) const;
    auto SizeBytes() const -> int32_t { return sizeBytes_; };

private:
    void Dispose();
    GlHandle bufferId_ = GlHandle{GL_NONE};
    GLenum targetType_ = 0xDEAD;
    Access accessMask_ = Access::NONE;
    int32_t sizeBytes_ = 0xDEAD;
};

} // namespace engine::gl
