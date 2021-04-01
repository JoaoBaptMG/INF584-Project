#pragma once

#include "resources/Mesh.hpp"
#include <glm/glm.hpp>

namespace meshUtils
{
    gl::MeshBuilder planeRight(float x, float ymin, float zmin, float ymax, float zmax);
    gl::MeshBuilder planeLeft(float x, float ymin, float zmin, float ymax, float zmax);
    gl::MeshBuilder planeUp(float y, float xmin, float zmin, float xmax, float zmax);
    gl::MeshBuilder planeDown(float y, float xmin, float zmin, float xmax, float zmax);
    gl::MeshBuilder planeFront(float z, float xmin, float ymin, float xmax, float ymax);
    gl::MeshBuilder planeBack(float z, float xmin, float ymin, float xmax, float ymax);
    gl::MeshBuilder box(glm::vec3 min, glm::vec3 max);
    gl::MeshBuilder addParameters(gl::MeshBuilder&& mesh, glm::u8vec4 color, float shininess);

    void swapWinding(gl::MeshBuilder& mesh);
}
