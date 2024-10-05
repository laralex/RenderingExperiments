#pragma once

#include "engine/gl/Context.hpp"

namespace engine::gl {

struct IGlDisposable {
#define Self IGlDisposable
public:
    Self()                                    = default;
    virtual ~Self()                           = default;
    virtual void Dispose(GlContext const& gl) = 0;

protected:
    Self(Self const&)            = default;
    Self& operator=(Self const&) = default;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self
};

} // namespace engine::gl