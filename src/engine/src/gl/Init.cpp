#include "engine/Prelude.hpp"
#include "engine/gl/TextureUnits.hpp"
#include "engine_private/Prelude.hpp"

namespace engine::gl
{

ENGINE_EXPORT void InitializeOpenGl() {
    gl::GlExtensions::Initialize();
    gl::GlCapabilities::Initialize();
    gl::InitializeDebug();
    gl::GlTextureUnits::Initialize();
}

ENGINE_EXPORT void DisposeOpenGl() { }

} // namespace engine::gl
