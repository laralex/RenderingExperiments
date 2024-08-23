#pragma once

#include "engine/Prelude.hpp"
#include "engine/gl/Program.hpp"

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

    void SetUbo(GLuint programBinding, GLuint bufferBindingIdx) const;
    void SetUniformMatrix2(GLint location, GLfloat const* values, GLsizei numMatrices = 1, GLboolean transpose = false);
    void SetUniformMatrix3(GLint location, GLfloat const* values, GLsizei numMatrices = 1, GLboolean transpose = false);
    void SetUniformMatrix4(GLint location, GLfloat const* values, GLsizei numMatrices = 1, GLboolean transpose = false);
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

    template <size_t NUM_COMPONENTS = 1> void SetUniformArray(GLint location, GLint const* values, GLsizei numValues) {
        PFNGLUNIFORM1IVPROC setter = nullptr;
        if constexpr (NUM_COMPONENTS == 1) {
            setter = glUniform1iv;
        } else if constexpr (NUM_COMPONENTS == 2) {
            setter = glUniform2iv;
        } else if constexpr (NUM_COMPONENTS == 3) {
            setter = glUniform3iv;
        } else if constexpr (NUM_COMPONENTS == 4) {
            setter = glUniform4iv;
        } else {
            std::terminate();
        }
        GLCALL(setter(location, numValues, values));
    }

    template <size_t NUM_COMPONENTS = 1> void SetUniformArray(GLint location, GLuint const* values, GLsizei numValues) {
        PFNGLUNIFORM1UIVPROC setter = nullptr;
        if constexpr (NUM_COMPONENTS == 1) {
            setter = glUniform1uiv;
        } else if constexpr (NUM_COMPONENTS == 2) {
            setter = glUniform2uiv;
        } else if constexpr (NUM_COMPONENTS == 3) {
            setter = glUniform3uiv;
        } else if constexpr (NUM_COMPONENTS == 4) {
            setter = glUniform4uiv;
        } else {
            std::terminate();
        }
        GLCALL(setter(location, numValues, values));
    }

    template <size_t NUM_COMPONENTS = 1>
    void SetUniformArray(GLint location, GLfloat const* values, GLsizei numValues) {
        PFNGLUNIFORM1FVPROC setter = nullptr;
        if constexpr (NUM_COMPONENTS == 1) {
            setter = glUniform1fv;
        } else if constexpr (NUM_COMPONENTS == 2) {
            setter = glUniform2fv;
        } else if constexpr (NUM_COMPONENTS == 3) {
            setter = glUniform3fv;
        } else if constexpr (NUM_COMPONENTS == 4) {
            setter = glUniform4fv;
        } else {
            std::terminate();
        }
        GLCALL(setter(location, numValues, values));
    }

private:
    static GlHandle contextProgram_;
    static bool hasInstances_;
};

} // namespace engine::gl
