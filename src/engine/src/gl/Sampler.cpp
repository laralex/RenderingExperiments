#include "engine/gl/Sampler.hpp"
#include "engine/gl/TextureUnits.hpp"
#include "engine_private/Prelude.hpp"

namespace engine::gl {

void Sampler::Dispose() {
    if (samplerId_ == GL_NONE) { return; }
    LogDebugLabel(*this, "Sampler object was disposed");
    GLCALL(glDeleteSamplers(1, &samplerId_));
    samplerId_.UnsafeReset();
}

auto Sampler::Allocate(std::string_view name) -> Sampler {
    Sampler sampler{};
    GLCALL(glGenSamplers(1, &sampler.samplerId_));
    if (!name.empty()) {
        // assert(GlCapabilities::IsInitialized());
        assert(GlTextureUnits::IsInitialized());
        GlTextureUnits::BeginStateSnapshot();
        GlTextureUnits::BindSampler(/*slot*/ 0U, sampler.samplerId_);
        GlTextureUnits::EndStateSnapshot();
        GlTextureUnits::RestoreState();
        // NOTE: crutch, sampler state is not created in the driver untill first bind
        // so we have to change some current sampler binding, it's NOT restored back
        // GLCALL(glBindSampler(GlCapabilities::maxTextureUnits - 1, sampler.samplerId_));
        DebugLabel(sampler, name);
        LogDebugLabel(sampler, "Sampler was allocated");
    }

    return sampler;
}

auto Sampler::WithDepthCompare(bool enable, GLenum compareFunc) -> Sampler&& {
    {
        GLenum f = compareFunc;
        assert(
            f == GL_LEQUAL | f == GL_GEQUAL | f == GL_LESS | f == GL_GREATER | f == GL_EQUAL | f == GL_NOTEQUAL
            | f == GL_ALWAYS | f == GL_NEVER);
    }
    GLCALL(glSamplerParameteri(samplerId_, GL_TEXTURE_COMPARE_MODE, enable ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE));
    GLCALL(glSamplerParameteri(samplerId_, GL_TEXTURE_COMPARE_FUNC, compareFunc));
    return std::move(*this);
}

auto Sampler::WithBorderColor(glm::vec4 color) -> Sampler&& {
    GLfloat colorArr[] = {color.r, color.g, color.b, color.a};
    GLCALL(glSamplerParameterfv(samplerId_, GL_TEXTURE_BORDER_COLOR, colorArr));
    return std::move(*this);
}

auto Sampler::WithLinearMagnify(bool filterLinear) -> Sampler&& {
    magnificationFilter_ = filterLinear ? GL_LINEAR : GL_NEAREST;
    GLCALL(glSamplerParameteri(samplerId_, GL_TEXTURE_MAG_FILTER, magnificationFilter_));
    return std::move(*this);
}

auto Sampler::SetMinificationFilter(bool minifyLinear, bool useMips, bool mipsLinear) -> Sampler&& {
    if (useMips) {
        if (minifyLinear) {
            minificationFilter_ = mipsLinear ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST;
        } else {
            minificationFilter_ = mipsLinear ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST;
        }
    } else {
        minificationFilter_ = minifyLinear ? GL_LINEAR : GL_NEAREST;
    }
    GLCALL(glSamplerParameteri(samplerId_, GL_TEXTURE_MIN_FILTER, minificationFilter_));
    return std::move(*this);
}

auto Sampler::WithLinearMinify(bool filterLinear) -> Sampler&& {
    filterLinearMinify_ = filterLinear;
    return SetMinificationFilter(filterLinearMinify_, filterUsingMips_, filterLinearMips_);
}

auto Sampler::WithLinearMinifyOverMips(bool filterUsingMips, bool filterLinear) -> Sampler&& {
    filterUsingMips_  = filterUsingMips;
    filterLinearMips_ = filterLinear;
    return SetMinificationFilter(filterLinearMinify_, filterUsingMips_, filterLinearMips_);
}

auto Sampler::WithMipConfig(GLfloat minMip, GLfloat maxMip, GLfloat bias) -> Sampler&& {
    GLCALL(glSamplerParameterf(samplerId_, GL_TEXTURE_MIN_LOD, minMip));
    GLCALL(glSamplerParameterf(samplerId_, GL_TEXTURE_MAX_LOD, maxMip));
    GLCALL(glSamplerParameterf(samplerId_, GL_TEXTURE_LOD_BIAS, bias));
    return std::move(*this);
}

auto Sampler::WithWrap(GLenum wrapX, GLenum wrapY, GLenum wrapZ) -> Sampler&& {
    auto assertInput = [](GLenum wrap) {
        assert(
            wrap == GL_CLAMP_TO_EDGE | wrap == GL_MIRRORED_REPEAT | wrap == GL_REPEAT
            | wrap == GL_MIRROR_CLAMP_TO_EDGE);
    };
    assertInput(wrapX);
    assertInput(wrapY);
    assertInput(wrapZ);
    GLCALL(glSamplerParameteri(samplerId_, GL_TEXTURE_WRAP_S, wrapX));
    GLCALL(glSamplerParameteri(samplerId_, GL_TEXTURE_WRAP_T, wrapY));
    GLCALL(glSamplerParameteri(samplerId_, GL_TEXTURE_WRAP_R, wrapZ));
    return std::move(*this);
}

auto Sampler::WithWrap(GLenum wrapXYZ) -> Sampler&& { return WithWrap(wrapXYZ, wrapXYZ, wrapXYZ); }

auto Sampler::WithAnisotropicFilter(GLfloat maxAnisotropy) -> Sampler&& {
    assert(GlExtensions::IsInitialized());
    if (GlExtensions::Supports(GlExtensions::EXT_texture_filter_anisotropic)) {
        GLCALL(glSamplerParameterf(samplerId_, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy));
    }
    return std::move(*this);
}

} // namespace engine::gl