#pragma once

#include "engine/Precompiled.hpp"
#include "engine/gl/GpuProgram.hpp"

namespace engine::gl {

// Helper object, binds GL program in ctor, unbinds it in dtor
// No two instances of the class should exist in one scope (checked by assert)
class UniformCtx final {

public:
#define Self UniformCtx
    explicit Self(GpuProgram const& useProgram) noexcept;
    ~Self() noexcept;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

    static auto GetUboLocation [[nodiscard]] (GpuProgram const& program, std::string_view programUboName) -> GLint;
    auto GetUboLocation [[nodiscard]] (std::string_view programUboName) const -> GLint;
    void SetUbo(GLuint programBinding, GLuint bufferBindingIdx) const;

    // NOTE: No CpuView wrapper used, because data length is guaranteed by function user
    void SetUniformMatrix2x2(
        GLint location, GLfloat const* values, GLsizei numMatrices = 1, GLboolean transpose = false);
    // NOTE: No CpuView wrapper used, because data length is guaranteed by function user
    void SetUniformMatrix3x3(
        GLint location, GLfloat const* values, GLsizei numMatrices = 1, GLboolean transpose = false);
    // NOTE: No CpuView wrapper used, because data length is guaranteed by function user
    void SetUniformMatrix4x4(
        GLint location, GLfloat const* values, GLsizei numMatrices = 1, GLboolean transpose = false);

    void SetUniformTexture(GLint location, GLint textureSlot);

    template <typename T> void SetUniformValue1(GLint location, T const value) {
        if constexpr (std::is_same_v<T, GLint>) {
            GLCALL(glUniform1i(location, value));
        } else if constexpr (std::is_same_v<T, GLuint>) {
            GLCALL(glUniform1ui(location, value));
        } else if constexpr (std::is_same_v<T, GLfloat>) {
            GLCALL(glUniform1f(location, value));
        } else if constexpr (std::is_same_v<T, GLdouble>) {
            GLCALL(glUniform1d(location, value));
        } else {
            std::terminate();
        }
    }

    // NOTE: No CpuView wrapper used, because data length is guaranteed by function user
    template <typename T> void SetUniformValue2(GLint location, T const* values2) {
        SetUniformValue2(location, values2[0], values2[1]);
    }

    template <typename T> void SetUniformValue2(GLint location, T const value0, T const value1) {
        if constexpr (std::is_same_v<T, GLint>) {
            GLCALL(glUniform2i(location, value0, value1));
        } else if constexpr (std::is_same_v<T, GLuint>) {
            GLCALL(glUniform2ui(location, value0, value1));
        } else if constexpr (std::is_same_v<T, GLfloat>) {
            GLCALL(glUniform2f(location, value0, value1));
        } else if constexpr (std::is_same_v<T, GLdouble>) {
            GLCALL(glUniform2d(location, value0, value1));
        } else {
            std::terminate();
        }
    }

    // NOTE: No CpuView wrapper used, because data length is guaranteed by function user
    template <typename T> void SetUniformValue3(GLint location, T const* values3) {
        SetUniformValue3(location, values3[0], values3[1], values3[2]);
    }

    template <typename T> void SetUniformValue3(GLint location, T const value0, T const value1, T const value2) {
        if constexpr (std::is_same_v<T, GLint>) {
            GLCALL(glUniform3i(location, value0, value1, value2));
        } else if constexpr (std::is_same_v<T, GLuint>) {
            GLCALL(glUniform3ui(location, value0, value1, value2));
        } else if constexpr (std::is_same_v<T, GLfloat>) {
            GLCALL(glUniform3f(location, value0, value1, value2));
        } else if constexpr (std::is_same_v<T, GLdouble>) {
            GLCALL(glUniform3d(location, value0, value1, value2));
        } else {
            std::terminate();
        }
    }

    // NOTE: No CpuView wrapper used, because data length is guaranteed by function user
    template <typename T> void SetUniformValue4(GLint location, T const* values4) {
        SetUniformValue4(location, values4[0], values4[1], values4[2], values4[3]);
    }

    template <typename T>
    void SetUniformValue4(GLint location, T const value0, T const value1, T const value2, T const value3) {
        if constexpr (std::is_same_v<T, GLint>) {
            GLCALL(glUniform4i(location, value0, value1, value2, value3));
        } else if constexpr (std::is_same_v<T, GLuint>) {
            GLCALL(glUniform4ui(location, value0, value1, value2, value3));
        } else if constexpr (std::is_same_v<T, GLfloat>) {
            GLCALL(glUniform4f(location, value0, value1, value2, value3));
        } else if constexpr (std::is_same_v<T, GLdouble>) {
            GLCALL(glUniform4d(location, value0, value1, value2, value3));
        } else {
            std::terminate();
        }
    }

    // NOTE: No CpuView wrapper used, because data length is guaranteed by function user
    template <typename T> void SetUniformArrayOf1(GLint location, T const* values, GLsizei numValues) {
        if constexpr (std::is_same_v<T, GLint>) {
            GLCALL(glUniform1iv(location, numValues, values));
        } else if constexpr (std::is_same_v<T, GLuint>) {
            GLCALL(glUniform1uiv(location, numValues, values));
        } else if constexpr (std::is_same_v<T, GLfloat>) {
            GLCALL(glUniform1fv(location, numValues, values));
        } else {
            std::terminate();
        }
    }

    // NOTE: No CpuView wrapper used, because data length is guaranteed by function user
    template <typename T> void SetUniformArrayOf2(GLint location, T const* values, GLsizei numValues) {
        if constexpr (std::is_same_v<T, GLint>) {
            GLCALL(glUniform2iv(location, numValues, values));
        } else if constexpr (std::is_same_v<T, GLuint>) {
            GLCALL(glUniform2uiv(location, numValues, values));
        } else if constexpr (std::is_same_v<T, GLfloat>) {
            GLCALL(glUniform2fv(location, numValues, values));
        } else {
            std::terminate();
        }
    }

    // NOTE: No CpuView wrapper used, because data length is guaranteed by function user
    template <typename T> void SetUniformArrayOf3(GLint location, T const* values, GLsizei numValues) {
        if constexpr (std::is_same_v<T, GLint>) {
            GLCALL(glUniform3iv(location, numValues, values));
        } else if constexpr (std::is_same_v<T, GLuint>) {
            GLCALL(glUniform3uiv(location, numValues, values));
        } else if constexpr (std::is_same_v<T, GLfloat>) {
            GLCALL(glUniform3fv(location, numValues, values));
        } else {
            std::terminate();
        }
    }

    // NOTE: No CpuView wrapper used, because data length is guaranteed by function user
    template <typename T> void SetUniformArrayOf4(GLint location, T const* values, GLsizei numValues) {
        if constexpr (std::is_same_v<T, GLint>) {
            GLCALL(glUniform4iv(location, numValues, values));
        } else if constexpr (std::is_same_v<T, GLuint>) {
            GLCALL(glUniform4uiv(location, numValues, values));
        } else if constexpr (std::is_same_v<T, GLfloat>) {
            GLCALL(glUniform4fv(location, numValues, values));
        } else {
            std::terminate();
        }
    }

private:
    static GlHandle contextProgram_;
    static bool hasInstances_;
};

} // namespace engine::gl
