#pragma once

#include <string>

#include <GL/gl.h>

class ShaderProgram {
public:
    ShaderProgram() = default;
    ~ShaderProgram();

    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    bool init(const std::string& vertexSource, const std::string& fragmentSource);
    void destroy() noexcept;

    void use() const noexcept;
    void stop() const noexcept;

    GLuint id() const noexcept { return m_program; }

    void setInt(const std::string& name, int value) const noexcept;
    void setVec4(const std::string& name, float x, float y, float z, float w) const noexcept;

private:
    GLuint m_program = 0;

    GLuint compileShader(GLenum type, const std::string& source);
};
