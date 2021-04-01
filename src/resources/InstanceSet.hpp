#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <cstddef>
#include "wrappers/glException.hpp"

namespace gl
{
    class InstanceSet final
    {
        GLuint matrixBuffer;
        GLsizei numInstances;

    public:
        InstanceSet() : numInstances(0) { glGenBuffers(1, &matrixBuffer); gl::checkError(); }
        ~InstanceSet() { glDeleteBuffers(1, &matrixBuffer); gl::checkError(); }

        // Disallow copying
        InstanceSet(const InstanceSet&) = delete;
        InstanceSet& operator=(const InstanceSet&) = delete;

        // Enable moving
        InstanceSet(InstanceSet&& o) noexcept : numInstances(o.numInstances), matrixBuffer(o.matrixBuffer) { o.matrixBuffer = 0; }
        InstanceSet& operator=(InstanceSet&& o) noexcept
        {
            numInstances = o.numInstances;
            std::swap(matrixBuffer, o.matrixBuffer);
            return *this;
        }

        // Upload the instances
        void setInstances(const std::vector<glm::mat4>& matrices)
        {
            glBindBuffer(GL_ARRAY_BUFFER, matrixBuffer); gl::checkError();
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * matrices.size(), matrices.data(), GL_STREAM_DRAW); gl::checkError();
            numInstances = (GLsizei)matrices.size();
        }

        // Use them
        void useInstances(GLuint modelAttributeIndex) const
        {
            glBindBuffer(GL_ARRAY_BUFFER, matrixBuffer); gl::checkError();

            for (int i = 0; i < 4; i++)
            {
                glEnableVertexAttribArray(modelAttributeIndex + i); gl::checkError();
                glVertexAttribPointer(modelAttributeIndex + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i)); gl::checkError();
                glVertexAttribDivisor(modelAttributeIndex + i, 1); gl::checkError(); // This is what sets it instanced
            }
        }

        friend class Mesh;
    };
}
