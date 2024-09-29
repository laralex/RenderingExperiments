#include "engine/gl/Sampler.hpp"
#include "engine/gl/TextureUnits.hpp"

#include "engine_private/Prelude.hpp"

namespace engine::gl {

ENGINE_EXPORT void GpuSampler::Dispose() {
    if (samplerId_ == GL_NONE) { return; }
    // LogDebugLabel(*this, "Sampler object was disposed");
    XLOG("Sampler object was disposed: 0x{:08X}", GLuint(samplerId_));
    GLCALL(glDeleteSamplers(1, samplerId_.Ptr()));
    samplerId_.UnsafeReset();
}

ENGINE_EXPORT auto GpuSampler::Allocate(GlContext& gl, std::string_view name) -> GpuSampler {
    GpuSampler sampler{};
    GLCALL(glGenSamplers(1, sampler.samplerId_.Ptr()));
    if (!name.empty()) {
        // assert(GlCapabilities::IsInitialized());
        auto& textureUnits = gl.TextureUnits();
        textureUnits.BeginStateSnapshot();
        textureUnits.BindSampler(/*slot*/ 0U, sampler.samplerId_);
        textureUnits.EndStateSnapshot();
        textureUnits.RestoreState();
        // NOTE: crutch, sampler state is not created in the driver untill first bind
        // so we have to change some current sampler binding, it's NOT restored back
        // GLCALL(glBindSampler(GlCapabilities::maxTextureUnits - 1, sampler.samplerId_));
        DebugLabel(gl, sampler, name);
        LogDebugLabel(gl, sampler, "Sampler was allocated");
    }

    return sampler;
}

ENGINE_EXPORT auto GpuSampler::WithDepthCompare(bool enable, GLenum compareFunc) && -> GpuSampler&& {
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

ENGINE_EXPORT auto GpuSampler::WithBorderColor(glm::vec4 color) && -> GpuSampler&& {
    GLfloat colorArr[] = {color.r, color.g, color.b, color.a};
    GLCALL(glSamplerParameterfv(samplerId_, GL_TEXTURE_BORDER_COLOR, colorArr));
    return std::move(*this);
}

ENGINE_EXPORT auto GpuSampler::WithLinearMagnify(bool filterLinear) && -> GpuSampler&& {
    magnificationFilter_ = filterLinear ? GL_LINEAR : GL_NEAREST;
    GLCALL(glSamplerParameteri(samplerId_, GL_TEXTURE_MAG_FILTER, magnificationFilter_));
    return std::move(*this);
}

ENGINE_EXPORT auto GpuSampler::SetMinificationFilter(
    bool minifyLinear, bool useMips, bool mipsLinear) && -> GpuSampler&& {
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

ENGINE_EXPORT auto GpuSampler::WithLinearMinify(bool filterLinear) && -> GpuSampler&& {
    filterLinearMinify_ = filterLinear;
    return std::move(*this).SetMinificationFilter(filterLinearMinify_, filterUsingMips_, filterLinearMips_);
}

ENGINE_EXPORT auto GpuSampler::WithLinearMinifyOverMips(bool filterUsingMips, bool filterLinear) && -> GpuSampler&& {
    filterUsingMips_  = filterUsingMips;
    filterLinearMips_ = filterLinear;
    return std::move(*this).SetMinificationFilter(filterLinearMinify_, filterUsingMips_, filterLinearMips_);
}

ENGINE_EXPORT auto GpuSampler::WithMipConfig(GLfloat minMip, GLfloat maxMip, GLfloat bias) && -> GpuSampler&& {
    GLCALL(glSamplerParameterf(samplerId_, GL_TEXTURE_MIN_LOD, minMip));
    GLCALL(glSamplerParameterf(samplerId_, GL_TEXTURE_MAX_LOD, maxMip));
    GLCALL(glSamplerParameterf(samplerId_, GL_TEXTURE_LOD_BIAS, bias));
    return std::move(*this);
}

ENGINE_EXPORT auto GpuSampler::WithWrap(GLenum wrapX, GLenum wrapY, GLenum wrapZ) && -> GpuSampler&& {
    auto assertInput = [](GLenum wrap) {
        assert(
            wrap == GL_CLAMP_TO_EDGE | wrap == GL_MIRRORED_REPEAT | wrap == GL_REPEAT
            /*| wrap == GL_MIRROR_CLAMP_TO_EDGE*/);
    };
    assertInput(wrapX);
    assertInput(wrapY);
    assertInput(wrapZ);
    GLCALL(glSamplerParameteri(samplerId_, GL_TEXTURE_WRAP_S, wrapX));
    GLCALL(glSamplerParameteri(samplerId_, GL_TEXTURE_WRAP_T, wrapY));
    GLCALL(glSamplerParameteri(samplerId_, GL_TEXTURE_WRAP_R, wrapZ));
    return std::move(*this);
}

ENGINE_EXPORT auto GpuSampler::WithWrap(GLenum wrapXYZ) && -> GpuSampler&& {
    return std::move(*this).WithWrap(wrapXYZ, wrapXYZ, wrapXYZ);
}

ENGINE_EXPORT auto GpuSampler::WithAnisotropicFilter(GlContext const& gl, GLfloat maxAnisotropy) && -> GpuSampler&& {
    if (gl.Extensions().Supports(GlExtensions::ARB_texture_filter_anisotropic)) {
        GLCALL(glSamplerParameterf(samplerId_, GL_TEXTURE_MAX_ANISOTROPY, maxAnisotropy));
    }
    return std::move(*this);
}

} // namespace engine::gl