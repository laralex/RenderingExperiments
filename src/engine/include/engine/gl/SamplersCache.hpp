#pragma once

#include "engine/Prelude.hpp"
#include "engine/gl/Sampler.hpp"

#include <map>

namespace engine::gl {

class SamplersCache final {

public:
#define Self SamplersCache
    explicit Self() noexcept     = default;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

    using CacheKey = size_t;

    auto FindSampler(std::string_view name) const -> Sampler const&;

    auto GetSampler [[nodiscard]] (CacheKey id) const -> Sampler const&;

    auto Cache [[nodiscard]] (std::string_view name, Sampler&& sampler) -> CacheKey;

    void Clear();

private:
    // TODO(a.larionov): std::unordered_map doens't support std::string_view lookup
    std::map<std::string, CacheKey, std::less<>> nameToId_;
    std::vector<Sampler> idToSampler_;

    static const Sampler nullSampler_;
};

} // namespace engine::gl