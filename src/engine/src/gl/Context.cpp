#include "engine/gl/Context.hpp"
#include "engine/gl/GpuProgramRegistry.hpp"
#include "engine_private/Prelude.hpp"

namespace engine::gl {

ENGINE_EXPORT void GlContext::Initialize() {
    extensions_.Initialize();
    capabilities_.Initialize();
    textureUnits_.Initialize(*this); // NOTE: capabilities must be initilized by now
    programsRegistry_ = std::make_shared<GpuProgramRegistry>();

    datalessTriangleVao_ = Vao::Allocate(*this, "Dataless Triangle VAO");
    std::ignore          = VaoMutableCtx{datalessTriangleVao_}.MakeUnindexed(3);

    datalessQuadVao_ = Vao::Allocate(*this, "Dataless Quad VAO");
    std::ignore      = VaoMutableCtx{datalessQuadVao_}.MakeUnindexed(4);

    isInitialized_ = true;
}

} // namespace engine::gl