#include "Shader.hpp"

#include "wrappers/glException.hpp"

using namespace gl;

auto shaderTypeToString(ShaderType type)
{
    switch (type)
    {
    case ShaderType::VertexShader: return "vertex";
    case ShaderType::GeometryShader: return "geometry";
    case ShaderType::FragmentShader: return "fragment";
    }
    return "";
}

Shader::Shader(ShaderType type, const char* source)
{
    if (type == ShaderType::Unknown) throw ShaderException("Cannot create shader of Unknown type!");

    shader = gl::checkError(glCreateShader(static_cast<GLenum>(type)));
   

    glShaderSource(shader, 1, &source, nullptr); gl::checkError();
    glCompileShader(shader); gl::checkError();

    // Check if compilation was okay
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status); gl::checkError();
    if (!status) throw ShaderException(std::string("Failed to compile ") + shaderTypeToString(type) + " shader: " + getInfoLog());
}

void Shader::setName(const std::string& name) const
{
    glObjectLabel(GL_SHADER, shader, (GLsizei)name.size(), name.data()); gl::checkError();
}

std::string Shader::getInfoLog() const
{
    GLint length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length); gl::checkError();
    std::string infoLog(length, 0);
    glGetShaderInfoLog(shader, length, nullptr, &infoLog[0]); gl::checkError();
    return infoLog;
}

ShaderType Shader::getType() const
{
    GLint type;
    glGetShaderiv(shader, GL_SHADER_TYPE, &type); gl::checkError();
    return static_cast<ShaderType>(type);
}

Shader::~Shader()
{
    glDeleteShader(shader); gl::checkError();
}
