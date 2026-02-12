#include "SampleApp.hpp"

#include <SDL.h>

#if __has_include(<glad/glad.h>)
#include <glad/glad.h>
#define ENGINE_GLAD_V1 1
#elif __has_include(<glad/gl.h>)
#include <glad/gl.h>
#define ENGINE_GLAD_V1 0
#else
#error "GLAD headers not found. Provide third_party/glad or a glad package."
#endif

#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl2.h>

#include <cstdint>
#include <exception>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

#include "CameraController.hpp"
#include "MeshRenderEngine.hpp"
#include "ObjLoader.hpp"
#include "SampleAssets.hpp"
#include "engine/modules/IModule.hpp"
#include "engine/modules/ModuleContract.hpp"
#include "engine/modules/ModuleManager.hpp"
#include "engine/platform/IPlatformBackend.hpp"
#include "engine/platform/IWindowSystem.hpp"
#include "engine/platform/PlatformBackendFactory.hpp"

namespace sample::app {
namespace {

class MeshDemoModule final : public engine::modules::IModule {
public:
  void onLoad() override { std::cout << "[module] mesh renderer loaded\n"; }
  void onStart() override { std::cout << "[module] mesh renderer started\n"; }
  void onStop() override { std::cout << "[module] mesh renderer stopped\n"; }
  void onUnload() override { std::cout << "[module] mesh renderer unloaded\n"; }
};

constexpr engine::modules::Version kEngineApiVersion{0, 1, 0};

[[nodiscard]] engine::modules::ModuleDescriptor makeDemoModuleDescriptor() {
  return engine::modules::ModuleDescriptor{
      .id = "sample.mesh_engine",
      .category = "sample",
      .moduleVersion = {0, 4, 0},
      .requiredApiVersion = kEngineApiVersion,
      .swapPolicy = engine::modules::SwapPolicy::RuntimeSwappable,
      .dependencies = {},
      .conflicts = {}};
}

void drawStaticLeftPanel(const float panelWidth,
                         const rendering::MeshRenderEngine& renderer,
                         const std::optional<std::uint32_t> hoveredMesh,
                         const std::optional<std::uint32_t> selectedMesh,
                         rendering::SceneLighting& lighting,
                         rendering::PbrMaterial& backpackMaterial,
                         float (&clearColor)[3]) {
  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->Pos);
  ImGui::SetNextWindowSize(ImVec2(panelWidth, viewport->Size.y));
  const ImGuiWindowFlags panelFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

  ImGui::Begin("Scene Explorer", nullptr, panelFlags);
  ImGui::Text("Renderer UI (Docked Left)");
  ImGui::Separator();
  ImGui::Text("Triangles: %u", renderer.totalTriangles());
  ImGui::Text("Hovered Mesh: %s", hoveredMesh.has_value() ? "yes" : "none");
  ImGui::Text("Selected Mesh: %s", selectedMesh.has_value() ? "yes" : "none");
  ImGui::Separator();

  if (ImGui::TreeNode("Scene")) {
    if (ImGui::TreeNode("Meshes")) {
      ImGui::BulletText("backpack.obj");
      ImGui::TreePop();
    }

    if (ImGui::TreeNode("Lighting")) {
      ImGui::SliderFloat3("Light Position", lighting.lightPosition, -10.0f, 10.0f);
      ImGui::ColorEdit3("Light Color", lighting.lightColor);
      ImGui::SliderFloat("Ambient", &lighting.ambientIntensity, 0.0f, 1.0f);
      ImGui::TreePop();
    }

    if (ImGui::TreeNode("Material")) {
      ImGui::ColorEdit3("Base Color", backpackMaterial.baseColor);
      ImGui::SliderFloat("Metallic", &backpackMaterial.metallic, 0.0f, 1.0f);
      ImGui::SliderFloat("Roughness", &backpackMaterial.roughness, 0.04f, 1.0f);
      ImGui::SliderFloat("AO", &backpackMaterial.ambientOcclusion, 0.0f, 1.0f);
      ImGui::ColorEdit3("Background", clearColor);
      ImGui::TreePop();
    }

    ImGui::TreePop();
  }

  ImGui::Separator();
  ImGui::TextWrapped("Controls: RMB + Mouse look, WASD move, LMB select looked-at mesh.");
  ImGui::End();
}

} // namespace

int runSampleApp() {
  try {
    engine::modules::ModuleManager moduleManager{kEngineApiVersion};
    auto moduleDescriptor = makeDemoModuleDescriptor();
    auto validation = moduleManager.validate({moduleDescriptor});
    if (!validation.ok) {
      for (const auto& error : validation.errors) {
        std::cerr << "Module validation error: " << error << '\n';
      }
      return 1;
    }

    MeshDemoModule demoModule;
    demoModule.onLoad();
    demoModule.onStart();

    auto platformBackend = engine::platform::createPlatformBackend();
    auto& windowSystem = platformBackend->windowSystem();

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    const auto windowId = windowSystem.createWindow(engine::platform::WindowCreateInfo{
        .title = "QumaRenderer - Mesh Engine (WASD + Pick + Static UI)", .size = {1280, 720}, .resizable = true, .highDpi = true});

    auto* sdlWindow = static_cast<SDL_Window*>(windowSystem.nativeWindowHandle(windowId));
    if (sdlWindow == nullptr) {
      throw std::runtime_error("Failed to retrieve native SDL window handle");
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(sdlWindow);
    if (glContext == nullptr) {
      throw std::runtime_error(std::string{"SDL_GL_CreateContext failed: "} + SDL_GetError());
    }

    if (SDL_GL_MakeCurrent(sdlWindow, glContext) != 0) {
      throw std::runtime_error(std::string{"SDL_GL_MakeCurrent failed: "} + SDL_GetError());
    }

#if ENGINE_GLAD_V1
    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress)) == 0) {
      throw std::runtime_error("gladLoadGLLoader failed");
    }
#else
    if (gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress)) == 0) {
      throw std::runtime_error("gladLoadGL failed");
    }
#endif

    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(sdlWindow, glContext);
    ImGui_ImplOpenGL3_Init("#version 330");

    rendering::MeshRenderEngine renderer{sdlWindow};
    auto backpackMesh = rendering::loadObjFromString(backpackObjSource());
    backpackMesh.material.name = "backpack_material";
    backpackMesh.material.baseColor[0] = 0.45f;
    backpackMesh.material.baseColor[1] = 0.62f;
    backpackMesh.material.baseColor[2] = 0.30f;
    backpackMesh.material.metallic = 0.1f;
    backpackMesh.material.roughness = 0.68f;

    const std::uint32_t backpackMeshId = renderer.addMeshInstance(rendering::MeshRenderEngine::MeshInstanceCreateInfo{
        .mesh = backpackMesh,
        .position = {0.0f, 0.0f, 0.0f},
        .rotationYRadians = 0.0f,
        .scale = 1.0f});

    rendering::SceneLighting lighting{};
    CameraController cameraController{};

    float clearColor[3] = {0.07f, 0.08f, 0.11f};
    std::optional<std::uint32_t> selectedMesh{};

    bool running = true;
    std::uint64_t currentTicks = SDL_GetPerformanceCounter();
    while (running && !windowSystem.shouldClose(windowId)) {
      const std::uint64_t newTicks = SDL_GetPerformanceCounter();
      const float deltaSeconds = static_cast<float>(static_cast<double>(newTicks - currentTicks) / static_cast<double>(SDL_GetPerformanceFrequency()));
      currentTicks = newTicks;

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplSDL2_NewFrame();
      ImGui::NewFrame();

      const bool allowMouseLook = !ImGui::GetIO().WantCaptureMouse;

      SDL_Event event;
      while (SDL_PollEvent(&event) == 1) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) {
          running = false;
        }
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) {
          running = false;
        }
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) {
          cameraController.setMouseLookActive(true);
        }
        if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_RIGHT) {
          cameraController.setMouseLookActive(false);
        }
        if (event.type == SDL_MOUSEMOTION) {
          cameraController.handleMouseMotion(event.motion, allowMouseLook);
        }
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && allowMouseLook) {
          selectedMesh = renderer.pickMeshFromScreen(event.button.x, event.button.y, cameraController.camera());
          renderer.setSelectedMesh(selectedMesh);
        }
      }

      cameraController.updateFromInput(deltaSeconds, SDL_GetKeyboardState(nullptr), !ImGui::GetIO().WantCaptureKeyboard);

      const auto lookedAt = renderer.findLookedAtMesh(cameraController.camera());
      renderer.setHoveredMesh(lookedAt);
      renderer.updateMeshMaterial(backpackMeshId, backpackMesh.material);

      int w = 0;
      int h = 0;
      SDL_GL_GetDrawableSize(sdlWindow, &w, &h);
      renderer.resize(w, h);
      renderer.beginFrame(clearColor[0], clearColor[1], clearColor[2]);
      renderer.renderScene(cameraController.camera(), lighting);

      drawStaticLeftPanel(340.0f, renderer, lookedAt, selectedMesh, lighting, backpackMesh.material, clearColor);

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
      renderer.endFrame();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(glContext);
    windowSystem.destroyWindow(windowId);

    demoModule.onStop();
    demoModule.onUnload();

    return 0;
  } catch (const std::exception& exception) {
    std::cerr << "Fatal error: " << exception.what() << '\n';
    return 1;
  }
}

} // namespace sample::app
