#pragma once

#include "Shader.hpp"
#include <filesystem>
#include <stdexcept>

namespace fileUtils
{
    class LoadException : public std::runtime_error
    {
    public:
        LoadException(std::string what) : std::runtime_error(what) {}
    };

    gl::Shader loadShader(std::filesystem::path path, gl::ShaderType type = gl::ShaderType::Unknown);

    void addDefaultLoaders();
}
