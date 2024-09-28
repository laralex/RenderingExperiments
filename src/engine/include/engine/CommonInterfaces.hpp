#include "engine/Precompiled.hpp"

namespace engine {

struct IDirty {
#define Self IDirty
    Self()                       = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
    virtual ~Self()              = default;
#undef Self
    virtual void CommitChanges() = 0;
};

struct IDisposable {
#define Self IDisposable
    Self()                       = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
    virtual ~Self()              = default;
#undef Self
    virtual void Dispose()       = 0;
};

struct ITransform {
#define Self ITransform
    Self()                       = default;
    Self(Self const&)            = default;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
    virtual ~Self()              = default;
#undef Self
    virtual auto Position [[nodiscard]] () const -> glm::vec3 = 0;
    virtual auto ComputeViewMatrix [[nodiscard]] () const -> glm::mat4 = 0;
    virtual auto ComputeModelMatrix [[nodiscard]] () const -> glm::mat4 = 0;
};

} // namespace engine
