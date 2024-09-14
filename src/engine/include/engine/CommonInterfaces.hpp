#include "engine/Precompiled.hpp"

namespace engine {

struct IDirty {
    virtual ~IDirty() noexcept { }
    virtual void CommitChanges() = 0;
};

struct IDisposable {
    virtual ~IDisposable() noexcept = 0;
    virtual void Dispose()          = 0;
};

struct ITransform {
    virtual ~ITransform() noexcept = 0;

    virtual auto Position [[nodiscard]] () const -> glm::vec3;
    virtual auto ComputeViewMatrix [[nodiscard]] () const -> glm::mat4;
    virtual auto ComputeModelMatrix [[nodiscard]] () const -> glm::mat4;
};

} // namespace engine
