#include "meshUtils.hpp"

#include <cmath>

using namespace meshUtils;

constexpr float Pi = 3.14159265359f;

static auto perpendicularBasis(glm::vec3 dir)
{
    // Compute two vectors that are perpendicular to dir and each other, used to make circles and ellipses
    // Those vectors are such that a x b is a vector parallel and in the same direction to dir
    auto perp = glm::cross(glm::vec3(1, 0, 0), dir);
    if (perp == glm::vec3()) perp = glm::cross(glm::vec3(0, 1, 0), dir);
    return std::make_pair(glm::normalize(perp), glm::normalize(glm::cross(perp, dir)));
}

gl::MeshBuilder planeX(float x, float ymin, float zmin, float ymax, float zmax, bool normalDir)
{
    gl::MeshBuilder mesh;

    // Generate the 4 vertices
    mesh.normals.resize(4, glm::vec3(normalDir ? 1 : -1, 0, 0));

    mesh.positions.resize(4);
    mesh.positions[0] = glm::vec3(x, ymin, zmin);
    mesh.positions[1] = glm::vec3(x, ymin, zmax);
    mesh.positions[2] = glm::vec3(x, ymax, zmax);
    mesh.positions[3] = glm::vec3(x, ymax, zmin);

    // Topology
    if (normalDir) mesh.indices = { 0, 2, 1, 0, 3, 2 };
    else mesh.indices = { 0, 1, 2, 0, 2, 3 };

    return mesh;
}

gl::MeshBuilder planeY(float y, float xmin, float zmin, float xmax, float zmax, bool normalDir)
{
    gl::MeshBuilder mesh;

    // Generate the 4 vertices
    mesh.normals.resize(4, glm::vec3(0, normalDir ? 1 : -1, 0));

    mesh.positions.resize(4);
    mesh.positions[0] = glm::vec3(xmin, y, zmin);
    mesh.positions[1] = glm::vec3(xmax, y, zmin);
    mesh.positions[2] = glm::vec3(xmax, y, zmax);
    mesh.positions[3] = glm::vec3(xmin, y, zmax);

    // Topology
    if (normalDir) mesh.indices = { 0, 2, 1, 0, 3, 2 };
    else mesh.indices = { 0, 1, 2, 0, 2, 3 };

    return mesh;
}

gl::MeshBuilder planeZ(float z, float xmin, float ymin, float xmax, float ymax, bool normalDir)
{
    gl::MeshBuilder mesh;

    // Generate the 4 vertices
    mesh.normals.resize(4, glm::vec3(0, 0, normalDir ? 1 : -1));

    mesh.positions.resize(4);
    mesh.positions[0] = glm::vec3(xmin, ymin, z);
    mesh.positions[1] = glm::vec3(xmin, ymax, z);
    mesh.positions[2] = glm::vec3(xmax, ymax, z);
    mesh.positions[3] = glm::vec3(xmax, ymin, z);

    // Topology
    if (normalDir) mesh.indices = { 0, 2, 1, 0, 3, 2 };
    else mesh.indices = { 0, 1, 2, 0, 2, 3 };

    return mesh;
}

gl::MeshBuilder meshUtils::planeRight(float x, float ymin, float zmin, float ymax, float zmax)
{
    return planeX(x, ymin, zmin, ymax, zmax, true);
}

gl::MeshBuilder meshUtils::planeLeft(float x, float ymin, float zmin, float ymax, float zmax)
{
    return planeX(x, ymin, zmin, ymax, zmax, false);
}

gl::MeshBuilder meshUtils::planeUp(float y, float xmin, float zmin, float xmax, float zmax)
{
    return planeY(y, xmin, zmin, xmax, zmax, true);
}

gl::MeshBuilder meshUtils::planeDown(float y, float xmin, float zmin, float xmax, float zmax)
{
    return planeY(y, xmin, zmin, xmax, zmax, false);
}

gl::MeshBuilder meshUtils::planeFront(float z, float xmin, float ymin, float xmax, float ymax)
{
    return planeZ(z, xmin, ymin, xmax, ymax, true);
}

gl::MeshBuilder meshUtils::planeBack(float z, float xmin, float ymin, float xmax, float ymax)
{
    return planeZ(z, xmin, ymin, xmax, ymax, false);
}

gl::MeshBuilder meshUtils::box(glm::vec3 min, glm::vec3 max)
{
    gl::MeshBuilder mesh;

    // Generate the vertices: we need 24 vertices in order to get all the correct places
    mesh.positions.reserve(24);
    mesh.normals.reserve(24);

    // For each one of the 6 faces, we insert the normal 4 times
    for (auto normal : { glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0),
        glm::vec3(0, -1, 0), glm::vec3(0, 0, 1), glm::vec3(0, 0, -1) })
    {
        mesh.normals.push_back(normal);
        mesh.normals.push_back(normal);
        mesh.normals.push_back(normal);
        mesh.normals.push_back(normal);
    }

    // Now, we do the positions manually - right face
    mesh.positions.emplace_back(max.x, min.y, min.z);
    mesh.positions.emplace_back(max.x, max.y, min.z);
    mesh.positions.emplace_back(max.x, max.y, max.z);
    mesh.positions.emplace_back(max.x, min.y, max.z);

    // left face
    mesh.positions.emplace_back(min.x, min.y, min.z);
    mesh.positions.emplace_back(min.x, min.y, max.z);
    mesh.positions.emplace_back(min.x, max.y, max.z);
    mesh.positions.emplace_back(min.x, max.y, min.z);

    // top face
    mesh.positions.emplace_back(min.x, max.y, min.z);
    mesh.positions.emplace_back(min.x, max.y, max.z);
    mesh.positions.emplace_back(max.x, max.y, max.z);
    mesh.positions.emplace_back(max.x, max.y, min.z);

    // bottom face
    mesh.positions.emplace_back(min.x, min.y, min.z);
    mesh.positions.emplace_back(max.x, min.y, min.z);
    mesh.positions.emplace_back(max.x, min.y, max.z);
    mesh.positions.emplace_back(min.x, min.y, max.z);

    // front face
    mesh.positions.emplace_back(min.x, min.y, max.z);
    mesh.positions.emplace_back(max.x, min.y, max.z);
    mesh.positions.emplace_back(max.x, max.y, max.z);
    mesh.positions.emplace_back(min.x, max.y, max.z);

    // back face
    mesh.positions.emplace_back(min.x, min.y, min.z);
    mesh.positions.emplace_back(min.x, max.y, min.z);
    mesh.positions.emplace_back(max.x, max.y, min.z);
    mesh.positions.emplace_back(max.x, min.y, min.z);

    // Topology
    mesh.indices.reserve(36);
    for (std::size_t i = 0; i < 6; i++)
    {
        mesh.indices.push_back((unsigned int)(4 * i + 0));
        mesh.indices.push_back((unsigned int)(4 * i + 1));
        mesh.indices.push_back((unsigned int)(4 * i + 2));
        mesh.indices.push_back((unsigned int)(4 * i + 0));
        mesh.indices.push_back((unsigned int)(4 * i + 2));
        mesh.indices.push_back((unsigned int)(4 * i + 3));
    }

    return mesh;
}

gl::MeshBuilder meshUtils::addParameters(gl::MeshBuilder&& mesh, glm::u8vec4 color, float shininess)
{
    auto size = mesh.validateAndGetNumberOfVertices();
    mesh.colors.resize(size, color);
    mesh.shininesses.resize(size, shininess);
    return mesh;
}

void meshUtils::swapWinding(gl::MeshBuilder& mesh)
{
    auto numTris = mesh.indices.size() / 3;
    for (std::size_t i = 0; i < numTris; i++)
        std::swap(mesh.indices[3 * i + 1], mesh.indices[3 * i + 2]);
}
