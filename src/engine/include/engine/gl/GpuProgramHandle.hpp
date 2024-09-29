#pragma once

#include "engine/Log.hpp"
#include <cstdint>

namespace engine::gl {
    class GpuProgramHandle final {

public:
    using InnerType = int32_t;

#define Self GpuProgramHandle
    explicit Self() : inner_(0) {}
    explicit Self(InnerType id)
        : inner_(id) { }
    ~Self() noexcept {
        if (inner_ != 0) {
            XLOGW("GpuProgramHandle leaked: 0x{:08X}", inner_); 
        }
        inner_ = 0;
    };
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&& other)
        : inner_(other.inner_) {
        other.UnsafeReset();
    }
    Self& operator=(Self&& other) {
        if (this == &other) { return *this; }
        if (inner_ != 0) { XLOGW("GpuProgramHandle leaked: 0x{:08X}", inner_); }
        inner_ = other.inner_;
        other.UnsafeReset();
        return *this;
    }
#undef Self
    void UnsafeReset() { inner_ = 0; }
    [[nodiscard]] operator InnerType() const { return inner_; }
    auto Inner[[nodiscard]]() const -> InnerType { return inner_; }

private:
    InnerType inner_;
};

const GpuProgramHandle NULL_GPU_PROGRAM_HANDLE = GpuProgramHandle{0};

}