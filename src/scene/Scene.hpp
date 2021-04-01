#pragma once

#include "wrappers/glfw.hpp"
#include "resources/Mesh.hpp"
#include "resources/Program.hpp"
#include "GBuffer.hpp"
#include "SSR.hpp"
#include "Camera.hpp"
#include "Lighting.hpp"
#include "resources/Query.hpp"

#include <queue>

namespace scene
{
    class Scene final
    {
        glfw::Window& window;
        Camera camera;
        Lighting lighting;
        GBuffer gbuffer;
        gl::Mesh floorMesh;
        gl::Mesh boxMeshes;

        gl::Texture2D resolveTexture;
        gl::Framebuffer resolveFramebuffer;
        std::shared_ptr<gl::Program> resolveProgram;

        std::shared_ptr<gl::Program> objectProgram;
        std::shared_ptr<gl::Program> shadowProgram;
        std::shared_ptr<gl::Program> ssrDrawProgram;

        SSR ssr;

        bool enableSSR;
        bool lastPressedSSR;

        bool showCounters;
        bool lastPressedCounters;

        bool lastPressedRegen;

        struct Queries 
        { 
            gl::Query gbuffer, shadow, resolve, ssr, finalStep;
            Queries() : gbuffer(gl::QueryType::TimeElapsed), shadow(gl::QueryType::TimeElapsed), resolve(gl::QueryType::TimeElapsed), 
                ssr(gl::QueryType::TimeElapsed), finalStep(gl::QueryType::TimeElapsed) {}
        };
        struct Results { GLuint64 gbuffer, shadow, resolve, ssr, finalStep; };

        std::queue<Queries> queries;
        Results lastResults;

    public:
        Scene(glfw::Window& window);
        ~Scene();

        void generateBoxMesh();

        void update(float delta);

        void getQueryResults();
        void draw();
        void drawScene(const glm::mat4& projection, const glm::mat4& view, gl::Program& program);
        void resolveGBuffer(const glm::mat4& view);
        void finalStep();
        void drawGui();

        static void drawFullScreenQuad();
    };
}
