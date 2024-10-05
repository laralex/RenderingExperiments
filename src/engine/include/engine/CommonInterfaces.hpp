#include "engine/Precompiled.hpp"

namespace engine {

struct IDirty {
#define Self IDirty
public:
    Self()                       = default;
    virtual ~Self()              = default;
    virtual void CommitChanges() = 0;

protected:
    Self(Self const&)            = default;
    Self& operator=(Self const&) = default;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self
};

struct IDisposable {
#define Self IDisposable
public:
    Self()                 = default;
    virtual ~Self()        = default;
    virtual void Dispose() = 0;

private:
    Self(Self const&)            = default;
    Self& operator=(Self const&) = default;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self
};

struct ITransform {
#define Self ITransform
public:
    Self()                                                              = default;
    virtual ~Self()                                                     = default;
    virtual auto Position [[nodiscard]] () const -> glm::vec3           = 0;
    virtual auto ComputeViewMatrix [[nodiscard]] () const -> glm::mat4  = 0;
    virtual auto ComputeModelMatrix [[nodiscard]] () const -> glm::mat4 = 0;

private:
    Self(Self const&)            = default;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self
};

} // namespace engine
