#include "Scene.hpp"

#include <random>
#include <iostream>
#include <queue>
#include <array>
#include <thread>
#include <optional>
#include "meshUtils.hpp"
#include "colors.hpp"
#include "util/grid.hpp"

#include "ImGuiS.hpp"
#include "resources/Cache.hpp"


using namespace scene;

const glm::vec3 LightDirection = glm::normalize(glm::vec3(0.5, -1, -0.5));

constexpr float Bounds = 1.0f;
constexpr float BottomY = -3.0f;
constexpr std::size_t BoxGridWidth = 8;
constexpr std::size_t BoxGridHeight = 6;
constexpr std::size_t MaxStackedBoxes = 4;
constexpr float AverageNumSeeds = 3.7f;
constexpr float BoxGenProb = 0.16f;
constexpr float MeanShininess = 6.5f;

constexpr std::array BoxColors
{ 
    colors::Green, colors::Yellow, colors::Blue, colors::Orange, colors::AirForceBlue, colors::DarkGreen, colors::Purple,
    colors::DarkSlateBlue, colors::AliceBlue, colors::Gold, colors::Ruby, colors::Maroon, colors::Fuchsia, colors::PastelPink,
    colors::Gray, colors::White, colors::AndroidGreen, colors::Brown, colors::LightYellow, colors::DarkSeaGreen, colors::Cyan,
    colors::Teal, colors::Wine, colors::LightSlateGray
};

constexpr glm::vec3 InitialPos = glm::vec3(BoxGridWidth / 2.0f, MaxStackedBoxes + 3.0f, BoxGridHeight + 6.0f);
constexpr glm::vec3 ViewPos = glm::vec3(BoxGridWidth / 2.0f, 0.0f, BoxGridHeight / 2.0f);

constexpr glm::u8vec4 withSpecular(glm::u8vec4 color, float specular)
{
    return glm::u8vec4(color.x, color.y, color.z, specular * 255);
}

constexpr auto FloorColor = withSpecular(colors::Red, 0.75);
constexpr auto WallColor = withSpecular(colors::Red, 0.5);

static std::optional<gl::Mesh> fullScreenQuad;

Scene::Scene(glfw::Window& window) : window(window), camera(window, 1000.0f), gbuffer(window.getFramebufferSize()), ssr(window.getFramebufferSize()),
    lighting(-Bounds, BottomY, -Bounds, Bounds + BoxGridWidth, (float)MaxStackedBoxes + 1, Bounds + BoxGridHeight, 1.0f/256.0f, LightDirection),
    enableSSR(true), lastPressedSSR(false), lastPressedRegen(false), showCounters(false), lastPressedCounters(false)
{
    camera.position = InitialPos;
    
    constexpr auto viewDir = ViewPos - InitialPos;
    camera.angles.y = std::atan2(viewDir.y, -viewDir.z);

    // Generate the floor mesh
    floorMesh = meshUtils::addParameters(meshUtils::planeUp(0.0f, -Bounds, -Bounds, Bounds + BoxGridWidth, Bounds + BoxGridHeight), FloorColor, 40.0f)
        + meshUtils::addParameters(meshUtils::planeFront(Bounds + BoxGridHeight, -Bounds, BottomY, Bounds + BoxGridWidth, 0.0f), WallColor, 40.0f)
        + meshUtils::addParameters(meshUtils::planeBack(-Bounds, -Bounds, BottomY, Bounds + BoxGridWidth, 0.0f), WallColor, 40.0f)
        + meshUtils::addParameters(meshUtils::planeRight(Bounds + BoxGridWidth, BottomY, -Bounds, 0.0f, Bounds + BoxGridHeight), WallColor, 40.0f)
        + meshUtils::addParameters(meshUtils::planeLeft(-Bounds, BottomY, -Bounds, 0.0f, Bounds + BoxGridHeight), WallColor, 40.0f);

    // Generate the boxes
    generateBoxMesh();

    // Load the program
    objectProgram = cache::loadProgram({ "resources/shaders/commonObjects.vert", "resources/shaders/commonObjects.frag" });
    shadowProgram = cache::loadProgram({ "resources/shaders/gbuffer.vert", "resources/shaders/depthWrite.frag" });
    resolveProgram = cache::loadProgram({ "resources/shaders/fullScreenQuad.vert", "resources/shaders/resolve.frag" });
    ssrDrawProgram = cache::loadProgram({ "resources/shaders/fullScreenQuad.vert", "resources/shaders/ssrDraw.frag" });

    // Build the full screen quad
    gl::MeshBuilder meshBuilder;
    meshBuilder.positions = { glm::vec3(-1, -1, 0), glm::vec3(1, -1, 0), glm::vec3(-1, 1, 0), glm::vec3(1, 1, 0) };
    fullScreenQuad = gl::Mesh(meshBuilder, gl::PrimitiveType::TriangleStrip);

    // Build the resolution framebuffer
    const auto& size = window.getFramebufferSize();
    resolveTexture.assign(0, gl::InternalFormat::RGBA8, size.width, size.height);
    resolveTexture.setMagFilter(gl::MagFilter::Linear);
    resolveTexture.setMinFilter(gl::MinFilter::Linear);
    resolveTexture.setName("G-Buffer Resolution Texture");
    resolveFramebuffer.attach(gl::ColorAttachment(0), resolveTexture);
    resolveFramebuffer.setName("G-Buffer Resolution Framebuffer");
}

void Scene::generateBoxMesh()
{
    // The random structure
    std::mt19937 engine(std::random_device{}());
    std::uniform_int_distribution<std::size_t> boxSize(1, 4);
    std::uniform_int_distribution boxStackSize(std::size_t(1), MaxStackedBoxes);
    std::uniform_int_distribution colorChoice(std::size_t(0), BoxColors.size() - 1);
    std::exponential_distribution shininess(1.0f / MeanShininess);

    util::grid<std::size_t> stackedBoxes(BoxGridWidth, BoxGridHeight);
    std::ranges::fill(stackedBoxes, std::size_t(0));

    // Now, create the seeds
    auto numSeeds = std::poisson_distribution<std::size_t>(AverageNumSeeds)(engine);
    for (std::size_t i = 0; i < numSeeds; i++)
    {
        // Grab a size for the seed
        auto width = boxSize(engine), height = boxSize(engine);

        // Grab a position
        auto x = std::uniform_int_distribution(std::size_t(0), BoxGridWidth - width)(engine);
        auto y = std::uniform_int_distribution(std::size_t(0), BoxGridHeight - height)(engine);
        auto boxHeight = boxStackSize(engine);

        // And paste the height
        std::ranges::fill(stackedBoxes.make_view(x, y, width, height), boxHeight);
    }

    // Now, we are going to "propagate" the box values
    for (std::size_t j = 0; j < BoxGridHeight; j++)
        for (std::size_t i = 0; i < BoxGridWidth; i++)
        {
            auto val1 = i == 0 || stackedBoxes(i - 1, j) == 0 ? 0 : stackedBoxes(i - 1, j) - 1;
            auto val2 = j == 0 || stackedBoxes(i, j - 1) == 0 ? 0 : stackedBoxes(i, j - 1) - 1;
            stackedBoxes(i, j) = std::max({ stackedBoxes(i, j), val1, val2 });
        }

    for (std::size_t j = BoxGridHeight; j > 0; j--)
        for (std::size_t i = BoxGridWidth; i > 0; i--)
        {
            auto val1 = i == BoxGridWidth || stackedBoxes(i, j - 1) == 0 ? 0 : stackedBoxes(i, j - 1) - 1;
            auto val2 = j == BoxGridHeight || stackedBoxes(i - 1, j) == 0 ? 0 : stackedBoxes(i - 1, j) - 1;
            stackedBoxes(i - 1, j - 1) = std::max({ stackedBoxes(i - 1, j - 1), val1, val2 });
        }

    // Finally, build the meshes
    gl::MeshBuilder mesh;

    for (std::size_t j = 0; j < BoxGridHeight; j++)
        for (std::size_t i = 0; i < BoxGridWidth; i++)
        {
            if (stackedBoxes(i, j) == 0) continue;

            auto h = stackedBoxes(i, j);
            auto k = h - 1;
            if (i == 0 || j == 0 || i == BoxGridWidth - 1 || j == BoxGridHeight - 1) k = 0;

            for (; k < h; k++)
            {
                auto min = glm::vec3(i, k, j);
                auto max = min + glm::vec3(1, 1, 1);
                mesh += meshUtils::addParameters(meshUtils::box(min, max), 
                    withSpecular(BoxColors[colorChoice(engine)], 0.125), shininess(engine));
            }
        }

    boxMeshes = mesh;
}

Scene::~Scene()
{
    while (!queries.empty()) queries.pop();
    fullScreenQuad = std::nullopt;
}

// Utility
static bool stateChange(bool& lastPress, bool press)
{
    bool val = !lastPress && press;
    lastPress = press;
    return val;
}

void Scene::update(float delta)
{
    camera.update(window, delta);

    if (stateChange(lastPressedSSR, window.getKey('Q')))
        enableSSR = !enableSSR;

    if (stateChange(lastPressedRegen, window.getKey('E')))
        generateBoxMesh();

    if (stateChange(lastPressedCounters, window.getKey('R')))
        showCounters = !showCounters;
}

void Scene::getQueryResults()
{
    while (!queries.empty())
    {
        auto& q = queries.front();
        if (q.gbuffer.available() && q.shadow.available() && q.resolve.available() && q.ssr.available() && q.finalStep.available())
        {
            lastResults.gbuffer = q.gbuffer.result();
            lastResults.shadow = q.shadow.result();
            lastResults.resolve = q.resolve.result();
            lastResults.ssr = q.ssr.result();
            lastResults.finalStep = q.finalStep.result();
            queries.pop();
        }
        else break;
    }
}

void Scene::draw()
{
    getQueryResults();

    const auto& view = camera.getViewMatrix();

    auto& q = queries.emplace();

    // Draw scene to g-buffer
    q.gbuffer.begin();
    gbuffer.begin();
    drawScene(camera.projection, view, gbuffer.getDrawProgram());
    gbuffer.end();
    q.gbuffer.end();

    // Draw scene with shadow
    q.shadow.begin();
    lighting.beginShadow();
    drawScene(lighting.getShadowProjection(), glm::mat4(1.0f), *shadowProgram);
    lighting.endShadow();
    q.shadow.end();

    // Resolve the lighting
    glDisable(GL_DEPTH_TEST); gl::checkError();
    q.resolve.begin();
    resolveGBuffer(view);
    q.resolve.end();

    // Compute the screen-space reflections
    q.ssr.begin();
    if (enableSSR) ssr.drawSSR(gbuffer, camera.projection);
    else ssr.clearSSR();
    q.ssr.end();

    // The final step
    q.finalStep.begin();
    finalStep();
    q.finalStep.end();

    glEnable(GL_DEPTH_TEST); gl::checkError();

    drawGui();
}

void scene::Scene::drawScene(const glm::mat4& projection, const glm::mat4& view, gl::Program& program)
{
    program.use();
    program.setUniform("Projection", projection);
    program.setUniform("View", view);

    // Draw the floor and the cubes
    floorMesh.draw(glm::mat4(1.0f));
    boxMeshes.draw(glm::mat4(1.0f));
}

void Scene::resolveGBuffer(const glm::mat4& view)
{
    resolveFramebuffer.bind();
    window.setViewport();
    glClearColor(0.0, 0.0, 0.0, 0.0); gl::checkError();
    glClear(GL_COLOR_BUFFER_BIT); gl::checkError();

    resolveProgram->use();
    gbuffer.setParams(*resolveProgram, camera.projection);
    resolveProgram->setUniform("InvViewShadowViewProjection", lighting.getShadowProjection() * glm::inverse(view));

    lighting.setLightParams(*resolveProgram, view);
    lighting.setShadowMapTexture(*resolveProgram);

    // Draw the fullscreen quad
    drawFullScreenQuad();
}

void Scene::finalStep()
{
    gl::Framebuffer::bindDefault();
    window.setViewport();
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT); gl::checkError();

    ssrDrawProgram->use();
    resolveTexture.bindTo(0);
    ssrDrawProgram->setUniform("ResolveTexture", 0);
    ssr.setTextureParam(*ssrDrawProgram);
    drawFullScreenQuad();
}

#include <imgui/imgui.h>

void Scene::drawGui()
{
    ImGui::Begin("Details Window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("WASD to move around, move mouse to move camera");
    ImGui::Text("Q to %s screen space reflections", enableSSR ? "disable" : "enable");
    ImGui::Text("E to regenerate the crates");
    ImGui::Text("R to %s the performance counters", showCounters ? "hide" : "show");
    ImGui::End();

    if (showCounters)
    {
        ImGui::Begin("Counters", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("G-Buffer Construction: %.3lfms", lastResults.gbuffer / 1000000.0);
        ImGui::Text("Shadow Map Generation: %.3lfms", lastResults.shadow / 1000000.0);
        ImGui::Text("Lighting Resolution: %.3lfms", lastResults.resolve / 1000000.0);
        ImGui::Text("SSR Buffers Constuction: %.3lfms", lastResults.ssr / 1000000.0);
        ImGui::Text("Final Combine Step: %.3lfms", lastResults.finalStep / 1000000.0);
        ImGui::End();
    }
}

void Scene::drawFullScreenQuad()
{
    if (fullScreenQuad)
        fullScreenQuad->draw(glm::mat4(1.0));
}

