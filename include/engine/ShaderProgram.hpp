#pragma once

#include <string>
#include <unordered_map>

#include <GL/gl.h>
#include <glm/mat4x4.hpp>

#include "app/Types.hpp"

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

    void setInt(const std::string& name, std::int32_t value) const noexcept;
    void setFloat(const std::string& name, float32_t value) const noexcept;
    void setMat4(const std::string& name, const glm::mat4& value) const noexcept;
    void setVec3(const std::string& name, float32_t x, float32_t y, float32_t z) const noexcept;
    void setVec4(const std::string& name, float32_t x, float32_t y, float32_t z, float32_t w) const noexcept;

private:
    GLuint m_program = 0;
    mutable std::unordered_map<std::string, GLint> m_uniformLocations;

    GLuint compileShader(GLenum type, const std::string& source);
    [[nodiscard]] GLint uniformLocation(const std::string& name) const noexcept;
};
