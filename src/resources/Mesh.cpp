#include "Mesh.hpp"

#include "wrappers/glException.hpp"
#include "bufferUtils.hpp"
#include <algorithm>
#include <numeric>

namespace LayoutIndices
{
    enum : GLuint
    {
        Position, Normal, Color, Texcoord, Shininess, Model0, Model1, Model2, Model3
    };
}

using namespace gl;

template <typename T>
static void concat(std::vector<T>& out, const std::vector<T>& in1, std::size_t expSize1, const std::vector<T>& in2, std::size_t expSize2)
{
    if (in1.empty() && in2.empty()) out.clear();
    else
    {
        out.resize(expSize1 + expSize2);
        auto it = std::copy(in1.begin(), in1.end(), out.begin());
        std::copy(in2.begin(), in2.end(), out.begin() + expSize1);
    }
}

MeshBuilder gl::operator+(const MeshBuilder& mb1, const MeshBuilder& mb2)
{
    MeshBuilder mesh;

    auto expSize1 = mb1.validateAndGetNumberOfVertices();
    auto expSize2 = mb2.validateAndGetNumberOfVertices();

    bool hasHomogeneous1 = mb1.positions.empty() && !mb1.positionsH.empty();
    bool hasHomogeneous2 = mb2.positions.empty() && !mb2.positionsH.empty();

    // Convert homogeneous whether necessary
    if (!hasHomogeneous1 && !hasHomogeneous2)
        concat(mesh.positions, mb1.positions, expSize1, mb2.positions, expSize2);
    else if (hasHomogeneous1 && !hasHomogeneous2)
    {
        auto toVec4 = [](const glm::vec3& vec) { return glm::vec4(vec, 1); };
        std::vector<glm::vec4> homogeneous2(mb2.positions.size());
        std::transform(mb2.positions.begin(), mb2.positions.end(), homogeneous2.begin(), toVec4);
        concat(mesh.positionsH, mb1.positionsH, expSize1, homogeneous2, expSize2);
    }
    else if (!hasHomogeneous1 && hasHomogeneous2)
    {
        auto toVec4 = [](const glm::vec3& vec) { return glm::vec4(vec, 1); };
        std::vector<glm::vec4> homogeneous1(mb1.positions.size());
        std::transform(mb1.positions.begin(), mb1.positions.end(), homogeneous1.begin(), toVec4);
        concat(mesh.positionsH, homogeneous1, expSize1, mb2.positionsH, expSize2);
    }
    else concat(mesh.positionsH, mb1.positionsH, expSize1, mb2.positionsH, expSize2);

    // Concat the remaining of the attributes
    concat(mesh.normals, mb1.normals, expSize1, mb2.normals, expSize2);
    concat(mesh.colors, mb1.colors, expSize1, mb2.colors, expSize2);
    concat(mesh.texcoords, mb1.texcoords, expSize1, mb2.texcoords, expSize2);
    concat(mesh.shininesses, mb1.shininesses, expSize1, mb2.shininesses, expSize2);

    if (!mb1.indices.empty() || !mb2.indices.empty())
    {
        auto numElements1 = mb1.indices.empty() ? expSize1 : mb1.indices.size();
        auto numElements2 = mb2.indices.empty() ? expSize2 : mb2.indices.size();
        concat(mesh.indices, mb1.indices, numElements1, mb2.indices, numElements2);

        if (mb1.indices.empty()) std::iota(mesh.indices.begin(), mesh.indices.begin() + numElements1, 0);

        if (mb2.indices.empty()) std::iota(mesh.indices.begin() + numElements1, mesh.indices.end(), (unsigned int)expSize1);
        else std::for_each(mesh.indices.begin() + numElements1, mesh.indices.end(), [=](auto& idx) { idx += (unsigned int)expSize1; });
    }

    return mesh;
}

std::size_t MeshBuilder::validateAndGetNumberOfVertices() const
{
    // Either you define normal coordinates or homogeneous coordinates, not both
    if (!positions.empty() && !positionsH.empty())
        throw MeshException("Cannot define positions and homogeneous positions at the same time!");

    auto sizes = { positions.size(), positionsH.size(), normals.size(), colors.size(), texcoords.size(), shininesses.size() };

    // First, ensure the consistency of the meshBuilder
    auto expectedSize = std::max(sizes);
    for (auto size : sizes)
        if (size != 0 && size != expectedSize)
            throw MeshException("Inconsistent MeshBuilder size when building!");

    return expectedSize;
}

MeshBuilder& MeshBuilder::operator+=(const MeshBuilder& other)
{
    *this = *this + other;
    return *this;
}

Mesh::Mesh(const MeshBuilder& meshBuilder, PrimitiveType primitiveType) : primitiveType(primitiveType)
{
    auto numVertices = meshBuilder.validateAndGetNumberOfVertices();
   
    // Generate the vertex array and bind the necessary indices
    glGenVertexArrays(1, &vertexArray); gl::checkError(); 

    // And bind the vertex array
    glBindVertexArray(vertexArray); gl::checkError(); 

    // Generate and configure the attributes
    if (meshBuilder.positionsH.empty())
        positionBuffer = createAndConfigureVertexArray(meshBuilder.positions, LayoutIndices::Position);
    else positionBuffer = createAndConfigureVertexArray(meshBuilder.positionsH, LayoutIndices::Position);
    normalBuffer = createAndConfigureVertexArray(meshBuilder.normals, LayoutIndices::Normal);
    colorBuffer = createAndConfigureVertexArray(meshBuilder.colors, LayoutIndices::Color, true);
    texcoordBuffer = createAndConfigureVertexArray(meshBuilder.texcoords, LayoutIndices::Texcoord);
    shininessBuffer = createAndConfigureVertexArray(meshBuilder.shininesses, LayoutIndices::Shininess);

    // Build the index list
    elementBuffer = createAndFillBuffer(meshBuilder.indices, GL_ELEMENT_ARRAY_BUFFER);
    numElements = (unsigned int)(meshBuilder.indices.empty() ? numVertices : meshBuilder.indices.size());

    // Unbind the vertex array
    glBindVertexArray(0); gl::checkError();
}

Mesh Mesh::empty()
{
    // Create an empty (but valid) mesh
    Mesh mesh;
    glGenVertexArrays(1, &mesh.vertexArray); gl::checkError();
    mesh.primitiveType = PrimitiveType::Triangles;
    return mesh;
}

Mesh& Mesh::operator=(Mesh&& mesh) noexcept
{
    std::swap(vertexArray, mesh.vertexArray);
    std::swap(numElements, mesh.numElements);
    std::swap(primitiveType, mesh.primitiveType);
    std::swap(elementBuffer, mesh.elementBuffer);
    std::swap(positionBuffer, mesh.positionBuffer);
    std::swap(normalBuffer, mesh.normalBuffer);
    std::swap(colorBuffer, mesh.colorBuffer);
    std::swap(texcoordBuffer, mesh.texcoordBuffer);
    std::swap(shininessBuffer, mesh.shininessBuffer);
    return *this;
}

void Mesh::setBufferName(GLuint buffer, std::string name)
{
    if (buffer) { glObjectLabel(GL_BUFFER, buffer, (GLsizei)name.size(), name.data()); gl::checkError(); }
}

void Mesh::setName(const std::string& name)
{
    glObjectLabel(GL_VERTEX_ARRAY, vertexArray, (GLsizei)name.size(), name.data()); gl::checkError();
    setBufferName(positionBuffer, name + " - position");
    setBufferName(normalBuffer, name + " - normal");
    setBufferName(colorBuffer, name + " - color");
    setBufferName(texcoordBuffer, name + " - texcoord");
    setBufferName(shininessBuffer, name + " - metalness");
    setBufferName(elementBuffer, name + " - elements");
}

template <typename T>
void refillBufferStream(GLuint& buffer, const std::vector<T>& data, GLenum target = GL_ARRAY_BUFFER)
{
    if (buffer == 0 && !data.empty())
    {
        glGenBuffers(1, &buffer); gl::checkError();
    }
    else if (buffer != 0 && data.empty())
    {
        glDeleteBuffers(1, &buffer); gl::checkError();
        buffer = 0;
    }

    if (data.empty()) return;
    glBindBuffer(target, buffer); gl::checkError();
    glBufferData(target, data.size() * sizeof(T), data.data(), GL_STREAM_DRAW); gl::checkError();
}

template <typename T>
void reconfigureVertexArrayStream(GLuint& buffer, const std::vector<T>& data, GLuint index, bool normalized = false)
{
    refillBufferStream(buffer, data);

    if (buffer != 0)
    {
        glEnableVertexAttribArray(index); gl::checkError();
        glVertexAttribPointer(index, vector_traits<T>::size, ParamFromType<typename vector_traits<T>::type>, normalized, sizeof(T), nullptr); gl::checkError();
    }
    else glDisableVertexAttribArray(index); gl::checkError();
}

void Mesh::streamMesh(const MeshBuilder& meshBuilder, PrimitiveType newPrimitiveType)
{
    auto numVertices = meshBuilder.validateAndGetNumberOfVertices();

    // Bind the vertex array
    glBindVertexArray(vertexArray); gl::checkError();

    // Recreate all buffers
    if (meshBuilder.positionsH.empty())
        reconfigureVertexArrayStream(positionBuffer, meshBuilder.positions, LayoutIndices::Position);
    else reconfigureVertexArrayStream(positionBuffer, meshBuilder.positionsH, LayoutIndices::Position);
    reconfigureVertexArrayStream(normalBuffer, meshBuilder.normals, LayoutIndices::Normal);
    reconfigureVertexArrayStream(colorBuffer, meshBuilder.colors, LayoutIndices::Color, true);
    reconfigureVertexArrayStream(texcoordBuffer, meshBuilder.texcoords, LayoutIndices::Texcoord);
    reconfigureVertexArrayStream(texcoordBuffer, meshBuilder.shininesses, LayoutIndices::Shininess);

    // Rebuild the index list
    refillBufferStream(elementBuffer, meshBuilder.indices, GL_ELEMENT_ARRAY_BUFFER);
    numElements = (unsigned int)(meshBuilder.indices.empty() ? numVertices : meshBuilder.indices.size());
    primitiveType = newPrimitiveType;

    // Unbind it in order to avoid outside changes
    glBindVertexArray(0); gl::checkError();
}

void Mesh::draw(const glm::mat4& model) const
{
    if (numElements == 0) return;

    // Bind the vertex array
    glBindVertexArray(vertexArray); gl::checkError();

    // Bind the vertex attribute
    glVertexAttrib4fv(LayoutIndices::Model0, glm::value_ptr(model[0])); gl::checkError();
    glVertexAttrib4fv(LayoutIndices::Model1, glm::value_ptr(model[1])); gl::checkError();
    glVertexAttrib4fv(LayoutIndices::Model2, glm::value_ptr(model[2])); gl::checkError();
    glVertexAttrib4fv(LayoutIndices::Model3, glm::value_ptr(model[3])); gl::checkError();

    // Use the appropriate draw function
    auto mode = static_cast<GLenum>(primitiveType);
    if (elementBuffer) { glDrawElements(mode, numElements, GL_UNSIGNED_SHORT, nullptr); gl::checkError(); }
    else { glDrawArrays(mode, 0, numElements); gl::checkError(); }
}

void Mesh::draw(const InstanceSet& instances) const
{
    if (numElements == 0) return;

    // Bind the vertex array
    glBindVertexArray(vertexArray); gl::checkError();

    // Bind the vertex attribute
    instances.useInstances(LayoutIndices::Model0);

    // Use the appropriate draw function
    auto mode = static_cast<GLenum>(primitiveType);
    if (elementBuffer) { glDrawElementsInstanced(mode, numElements, GL_UNSIGNED_SHORT, nullptr, instances.numInstances); gl::checkError(); }
    else { glDrawArraysInstanced(mode, 0, numElements, instances.numInstances); gl::checkError(); }
}


Mesh::~Mesh()
{
    // Delete the vertex array
    glDeleteVertexArrays(1, &vertexArray); gl::checkError();

    glDeleteBuffers(1, &elementBuffer); gl::checkError();
    glDeleteBuffers(1, &positionBuffer); gl::checkError();
    glDeleteBuffers(1, &normalBuffer); gl::checkError();
    glDeleteBuffers(1, &colorBuffer); gl::checkError();
    glDeleteBuffers(1, &texcoordBuffer); gl::checkError();
    glDeleteBuffers(1, &shininessBuffer); gl::checkError();
}
