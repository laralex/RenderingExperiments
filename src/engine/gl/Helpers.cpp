#include "engine/Prelude.hpp"
#include "engine/gl/TextureUnits.hpp"
#include "engine_private/Prelude.hpp"

namespace engine::gl {

ENGINE_EXPORT void InitializeOpenGl() {
    gl::GlExtensions::Initialize();
    gl::GlCapabilities::Initialize();
    gl::InitializeDebug();
    gl::GlTextureUnits::Initialize();
    gl::CommonRenderers::Initialize();
}

void BlitFramebuffers(
    GLuint srcFramebuffer, GLuint dstFramebuffer, GLbitfield colorDepthStencilMask, bool filterLinear) {
    GLCALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFramebuffer));
    GLCALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFramebuffer));
    GLCALL(glBlitFramebuffer(
        /* srcX0 */ 0,
        /* srcY0 */ 0,
        /* srcX1 */ 0,
        /* srcY1 */ 0,
        /* dstX0 */ 0,
        /* dstY0 */ 0,
        /* dstX1 */ 0,
        /* dstY1 */ 0, colorDepthStencilMask, filterLinear ? GL_LINEAR : GL_NEAREST));
}

} // namespace engine::gl