#include "engine/gl/Context.hpp"

namespace engine::gl {

void GlContext::Initialize() {
    extensions_.Initialize();
    capabilities_.Initialize();
    textureUnits_.Initialize(*this); // NOTE: capabilities must be initilized by now
    isInitialized_ = true;
}

} // namespace engine::gl