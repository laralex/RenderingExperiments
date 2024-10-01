#include "engine/gl/Context.hpp"
#include "engine/gl/GpuProgramRegistry.hpp"
#include "engine_private/Prelude.hpp"

namespace engine::gl {

ENGINE_EXPORT void GlContext::Initialize() {
    extensions_.Initialize();
    capabilities_.Initialize();
    textureUnits_.Initialize(*this); // NOTE: capabilities must be initilized by now
    programOwner_  = std::make_shared<GpuProgramRegistry>();
    isInitialized_ = true;
}

ENGINE_EXPORT auto GlContext::GetProgram(GpuProgramHandle const& handle) const -> GpuProgram const& {
    return programOwner_->ViewProgram(handle);
}

} // namespace engine::gl