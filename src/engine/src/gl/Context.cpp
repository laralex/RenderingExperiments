#include "engine/gl/Context.hpp"

#include "engine_private/Prelude.hpp"

namespace engine::gl {

ENGINE_EXPORT void GlContext::Initialize() {
    extensions_.Initialize();
    capabilities_.Initialize();
    textureUnits_.Initialize(*this); // NOTE: capabilities must be initilized by now
    isInitialized_ = true;
}

} // namespace engine::gl