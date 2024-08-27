#pragma once

#include "engine/LineRendererInput.hpp"
#include "engine/Prelude.hpp"
#include "engine/gl/Buffer.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Vao.hpp"
#include <glm/mat4x4.hpp>

#include <stack>

namespace engine::gl {

class LineRenderer final {

public:
#define Self LineRenderer
    explicit Self() noexcept     = default;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static auto Allocate [[nodiscard]] (size_t maxLines) -> LineRenderer;
    void Fill(std::vector<LineRendererInput::Line> const& lines) const;
    void Render(glm::mat4 const& camera) const;

private:
    struct ColorCtx final {
        glm::vec3 color;
        int32_t colorIdx;
        int32_t oldColorLastLineIdx;
    };

    void ClearQueue();

    Vao vao_{};
    GpuBuffer attributeBuffer_{};
    GpuProgram program_{};
};

} // namespace engine::gl