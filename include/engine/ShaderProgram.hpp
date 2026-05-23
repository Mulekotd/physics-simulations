#pragma once

#include <string>
#include <unordered_map>

#include <GL/gl.h>
#include <glm/mat4x4.hpp>

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
    void setFloat(const std::string& name, float value) const noexcept;
    void setMat4(const std::string& name, const glm::mat4& value) const noexcept;
    void setVec3(const std::string& name, float x, float y, float z) const noexcept;
    void setVec4(const std::string& name, float x, float y, float z, float w) const noexcept;

private:
    GLuint m_program = 0;
    mutable std::unordered_map<std::string, GLint> m_uniformLocations;

    GLuint compileShader(GLenum type, const std::string& source);
    [[nodiscard]] GLint uniformLocation(const std::string& name) const noexcept;
};
