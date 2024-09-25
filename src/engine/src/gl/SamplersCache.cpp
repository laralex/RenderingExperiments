#include "engine/gl/SamplersCache.hpp"

#include "engine_private/Prelude.hpp"

namespace engine::gl {

ENGINE_STATIC GpuSampler const SamplersCache::nullSampler_{};

ENGINE_EXPORT auto SamplersCache::FindSampler(std::string_view name) const -> GpuSampler const& {
    auto findId = nameToId_.find(name);
    if (findId == nameToId_.end()) {
        XLOGE("Can't find key={} in SamplersCache, returning NULL sampler", name)
        return nullSampler_;
    };
    return idToSampler_[findId->second];
}

ENGINE_EXPORT auto SamplersCache::GetSampler(CacheKey id) const -> GpuSampler const& {
    if (id < 0 && id >= std::size(idToSampler_)) {
        XLOGE("Can't find key={} in SamplersCache, returning NULL sampler", id);
        return nullSampler_;
    };
    return idToSampler_[id];
}

ENGINE_EXPORT auto SamplersCache::Store(std::string_view name, GpuSampler&& sampler) -> CacheKey {
    idToSampler_.emplace_back(std::move(sampler));
    size_t id = std::size(idToSampler_) - 1;
    nameToId_.emplace(name, id);
    return id;
}

ENGINE_EXPORT void SamplersCache::Clear() {
    XLOGE("SamplersCache::Clear(), dropped {} samplers", std::size(idToSampler_));
    nameToId_.clear();
    idToSampler_.clear();
}

} // namespace engine::gl