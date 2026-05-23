#include <cstdio>

#include <GL/gl.h>
#include <glm/gtc/type_ptr.hpp>

#include "engine/ShaderProgram.hpp"

ShaderProgram::~ShaderProgram()
{
    destroy();
}

void ShaderProgram::destroy() noexcept
{
    if (m_program)
    {
        glDeleteProgram(m_program);
        m_program = 0;
    }

    m_uniformLocations.clear();
}

bool ShaderProgram::init(const std::string& vertexSource, const std::string& fragmentSource)
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    if (!vs || !fs)
    {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return false;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);

    if (!linked)
    {
        GLint length = 0;

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

        if (length > 1)
        {
            std::string log(static_cast<std::size_t>(length), '\0');
            glGetProgramInfoLog(program, length, nullptr, log.data());
            std::fprintf(stderr, "Shader link error: %s\n", log.c_str());
        }

        glDeleteProgram(program);
        glDeleteShader(vs);
        glDeleteShader(fs);
        return false;
    }

    glDetachShader(program, vs);
    glDetachShader(program, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    m_program = program;
    m_uniformLocations.clear();
    return true;
}

void ShaderProgram::use() const noexcept
{
    glUseProgram(m_program);
}

void ShaderProgram::stop() const noexcept
{
    glUseProgram(0);
}

void ShaderProgram::setInt(const std::string& name, int value) const noexcept
{
    GLint location = uniformLocation(name);

    if (location >= 0)
        glUniform1i(location, value);
}

void ShaderProgram::setFloat(const std::string& name, float value) const noexcept
{
    GLint location = uniformLocation(name);

    if (location >= 0)
        glUniform1f(location, value);
}

void ShaderProgram::setMat4(const std::string& name, const glm::mat4& value) const noexcept
{
    GLint location = uniformLocation(name);

    if (location >= 0)
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void ShaderProgram::setVec3(const std::string& name, float x, float y, float z) const noexcept
{
    GLint location = uniformLocation(name);

    if (location >= 0)
        glUniform3f(location, x, y, z);
}

void ShaderProgram::setVec4(const std::string& name, float x, float y, float z, float w) const noexcept
{
    GLint location = uniformLocation(name);

    if (location >= 0)
        glUniform4f(location, x, y, z, w);
}

GLint ShaderProgram::uniformLocation(const std::string& name) const noexcept
{
    auto it = m_uniformLocations.find(name);

    if (it != m_uniformLocations.end())
        return it->second;

    GLint location = glGetUniformLocation(m_program, name.c_str());

    try
    {
        m_uniformLocations.emplace(name, location);
    }
    catch (...)
    {
    }

    return location;
}

GLuint ShaderProgram::compileShader(GLenum type, const std::string& source)
{
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();

    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

        if (length > 1)
        {
            std::string log(static_cast<std::size_t>(length), '\0');
            glGetShaderInfoLog(shader, length, nullptr, log.data());
            std::fprintf(stderr, "Shader compile error: %s\n", log.c_str());
        }

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}
