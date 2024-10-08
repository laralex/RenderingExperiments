#pragma once

#include "engine/LineRendererInput.hpp"
#include "engine/Precompiled.hpp"
#include "engine/gl/GpuBuffer.hpp"
#include "engine/gl/IGlDisposable.hpp"
#include "engine/gl/GpuProgram.hpp"
#include "engine/gl/Vao.hpp"
#include <glm/mat4x4.hpp>

#include <stack>

namespace engine::gl {

class LineRenderer final : public IGlDisposable {

public:
#define Self LineRenderer
    explicit Self() noexcept     = default;
    ~Self() override             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static auto Allocate [[nodiscard]] (GlContext& gl, size_t maxLines) -> LineRenderer;
    void Fill(std::vector<LineRendererInput::Line> const& lines, size_t numLines, size_t numLinesOffset) const;
    void Render(GlContext& gl, glm::mat4 const& camera) const;
    void Dispose(GlContext const& gl) override;

private:
    Vao vao_ = Vao{};
    GpuBuffer attributeBuffer_ = GpuBuffer{};
    std::shared_ptr<GpuProgram> program_ = {};
};

} // namespace engine::gl