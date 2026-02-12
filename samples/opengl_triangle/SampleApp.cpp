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

#include <array>
#include <cstdint>
#include <exception>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

#include "CameraController.hpp"
#include "EngineInstanceManager.hpp"
#include "MeshRenderEngine.hpp"
#include "PrimitiveMeshFactory.hpp"
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
      .moduleVersion = {0, 6, 0},
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
                         float (&clearColor)[3]) {
  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->Pos);
  ImGui::SetNextWindowSize(ImVec2(panelWidth, viewport->Size.y));
  const ImGuiWindowFlags panelFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

  ImGui::Begin("Scene Explorer", nullptr, panelFlags);
  ImGui::Text("Scene / Renderer State");
  ImGui::Separator();
  ImGui::Text("Triangles: %u", renderer.totalTriangles());
  ImGui::Text("Hovered Mesh Id: %d", hoveredMesh.has_value() ? static_cast<int>(hoveredMesh.value()) : -1);
  ImGui::Text("Selected Mesh Id: %d", selectedMesh.has_value() ? static_cast<int>(selectedMesh.value()) : -1);
  ImGui::Separator();

  if (ImGui::TreeNode("Lighting")) {
    ImGui::SliderFloat3("Light Position", lighting.lightPosition, -10.0f, 10.0f);
    ImGui::ColorEdit3("Light Color", lighting.lightColor);
    ImGui::SliderFloat("Ambient", &lighting.ambientIntensity, 0.0f, 1.0f);
    ImGui::ColorEdit3("Background", clearColor);
    ImGui::TreePop();
  }

  ImGui::Separator();
  ImGui::TextWrapped("Controls: RMB + Mouse Look, WASD move, LMB pick/select.");
  ImGui::End();
}

void drawGizmoWindow(rendering::MeshRenderEngine& renderer, const std::optional<std::uint32_t> selectedMesh) {
  if (!selectedMesh.has_value()) {
    return;
  }

  auto transform = renderer.meshTransform(selectedMesh.value());
  if (!transform.has_value()) {
    return;
  }

  ImGui::Begin("Transform Gizmo");
  ImGui::Text("Selected Mesh: %u", selectedMesh.value());
  ImGui::Separator();

  bool changed = false;
  changed |= ImGui::DragFloat3("Location", transform->position, 0.03f);
  float rotationDegrees = transform->rotationYRadians * 57.2957795f;
  if (ImGui::DragFloat("Rotation Y", &rotationDegrees, 1.0f, -360.0f, 360.0f)) {
    transform->rotationYRadians = rotationDegrees * 0.0174532925f;
    changed = true;
  }
  changed |= ImGui::DragFloat("Scale", &transform->scale, 0.01f, 0.2f, 4.0f);

  ImGui::Separator();
  ImGui::Text("Quick Axis Move");
  if (ImGui::Button("X-")) {
    transform->position[0] -= 0.1f;
    changed = true;
  }
  ImGui::SameLine();
  if (ImGui::Button("X+")) {
    transform->position[0] += 0.1f;
    changed = true;
  }
  if (ImGui::Button("Y-")) {
    transform->position[1] -= 0.1f;
    changed = true;
  }
  ImGui::SameLine();
  if (ImGui::Button("Y+")) {
    transform->position[1] += 0.1f;
    changed = true;
  }
  if (ImGui::Button("Z-")) {
    transform->position[2] -= 0.1f;
    changed = true;
  }
  ImGui::SameLine();
  if (ImGui::Button("Z+")) {
    transform->position[2] += 0.1f;
    changed = true;
  }

  if (changed) {
    renderer.setMeshTransform(selectedMesh.value(), *transform);
  }

  ImGui::End();
}

void drawWindowManager(EngineInstanceManager& instanceManager,
                       engine::modules::ModuleManager& moduleManager,
                       const std::string& backpackObjSourceText,
                       std::optional<std::uint32_t>& selectedMesh,
                       rendering::MeshRenderEngine& renderer) {
  ImGui::Begin("Window Manager");
  ImGui::Text("Engine Instance Manager");
  ImGui::Text("Running Instances: %u", instanceManager.totalRunningInstances());
  ImGui::Separator();

  static int selectedConfig = 0;
  static int selectedProfile = 0;
  static int selectedPrimitive = 0;
  static int instanceNameCounter = 2;
  static constexpr std::array<const char*, 3> configs{"Debug", "Release", "Custom"};
  static constexpr std::array<const char*, 3> profiles{"Editor", "Game", "Tools"};
  static constexpr std::array<PrimitiveMeshType, 3> primitiveValues{
      PrimitiveMeshType::Backpack,
      PrimitiveMeshType::Sphere,
      PrimitiveMeshType::Cone};
  static constexpr std::array<const char*, 3> primitiveNames{"Backpack", "Sphere", "Cone"};

  ImGui::Combo("New Instance Config", &selectedConfig, configs.data(), static_cast<int>(configs.size()));
  ImGui::Combo("New Instance Profile", &selectedProfile, profiles.data(), static_cast<int>(profiles.size()));
  ImGui::Combo("Mesh Type", &selectedPrimitive, primitiveNames.data(), static_cast<int>(primitiveNames.size()));

  if (ImGui::Button("Create Instance")) {
    auto mesh = createPrimitiveMesh(primitiveValues[static_cast<std::size_t>(selectedPrimitive)], backpackObjSourceText);
    instanceManager.createInstanceWithMesh("instance_" + std::to_string(instanceNameCounter++),
                                           configs[static_cast<std::size_t>(selectedConfig)],
                                           profiles[static_cast<std::size_t>(selectedProfile)],
                                           mesh);
  }

  ImGui::Separator();
  if (ImGui::TreeNode("Instances")) {
    for (auto& instance : instanceManager.instances()) {
      const std::string nodeTitle = instance.summary.name + "##" + std::to_string(instance.summary.instanceId);
      if (ImGui::TreeNode(nodeTitle.c_str())) {
        ImGui::Text("Id: %u", instance.summary.instanceId);
        ImGui::Text("Config: %s", instance.summary.config.c_str());
        ImGui::Text("Profile: %s", instance.summary.profile.c_str());
        ImGui::Text("Mesh Id: %u", instance.meshId);
        ImGui::Text("Status: %s", instance.summary.running ? "running" : "stopped");
        if (ImGui::SmallButton(("Select##" + std::to_string(instance.meshId)).c_str())) {
          selectedMesh = instance.meshId;
          renderer.setSelectedMesh(selectedMesh);
        }

        if (ImGui::TreeNode("Modules")) {
          std::vector<engine::modules::ModuleDescriptor> descriptors;
          descriptors.reserve(instance.modules.size());
          for (const auto& module : instance.modules) {
            descriptors.push_back(module.descriptor);
          }
          const auto validation = moduleManager.validate(descriptors);
          if (!validation.ok) {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Validation has %d issue(s)", static_cast<int>(validation.errors.size()));
          }

          for (auto& module : instance.modules) {
            const std::string moduleNode = module.descriptor.id + "##module_" + std::to_string(instance.summary.instanceId);
            if (ImGui::TreeNode(moduleNode.c_str())) {
              ImGui::Text("Category: %s", module.descriptor.category.c_str());
              ImGui::Text("Version: %u.%u.%u",
                          module.descriptor.moduleVersion.major,
                          module.descriptor.moduleVersion.minor,
                          module.descriptor.moduleVersion.patch);
              ImGui::Text("Swap: %s", moduleManager.canHotSwap(module.descriptor) ? "runtime swappable" : "locked");
              ImGui::Text("HotSwap Generation: %u", module.hotSwapGeneration);
              ImGui::Checkbox(("Enabled##" + moduleNode).c_str(), &module.enabled);

              if (moduleManager.canHotSwap(module.descriptor) && ImGui::Button(("Hot Replace##" + moduleNode).c_str())) {
                ++module.hotSwapGeneration;
                ++module.descriptor.moduleVersion.patch;
              }
              ImGui::TreePop();
            }
          }
          ImGui::TreePop();
        }
        ImGui::TreePop();
      }
    }
    ImGui::TreePop();
  }

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
        .title = "QumaRenderer - Engine Window Manager", .size = {1440, 840}, .resizable = true, .highDpi = true});

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
    const std::string backpackObjText = backpackObjSource();
    auto baseMesh = createPrimitiveMesh(PrimitiveMeshType::Backpack, backpackObjText);

    EngineInstanceManager instanceManager{renderer, baseMesh, kEngineApiVersion};
    const auto initialMeshId = instanceManager.createInstanceWithMesh("instance_1", "Debug", "Editor", baseMesh);

    rendering::SceneLighting lighting{};
    CameraController cameraController{};

    float clearColor[3] = {0.07f, 0.08f, 0.11f};
    std::optional<std::uint32_t> selectedMesh{initialMeshId};
    renderer.setSelectedMesh(selectedMesh);

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
      std::optional<std::uint32_t> lookedAtInFrame;

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
          const auto pick = renderer.pickMeshFromScreen(event.button.x, event.button.y, cameraController.camera());
          if (pick.has_value()) {
            selectedMesh = pick;
          } else if (lookedAtInFrame.has_value()) {
            selectedMesh = lookedAtInFrame;
          }
          renderer.setSelectedMesh(selectedMesh);
        }
      }

      cameraController.updateFromInput(deltaSeconds, SDL_GetKeyboardState(nullptr), !ImGui::GetIO().WantCaptureKeyboard);

      lookedAtInFrame = renderer.findLookedAtMesh(cameraController.camera());
      renderer.setHoveredMesh(lookedAtInFrame);

      int w = 0;
      int h = 0;
      SDL_GL_GetDrawableSize(sdlWindow, &w, &h);
      renderer.resize(w, h);
      renderer.beginFrame(clearColor[0], clearColor[1], clearColor[2]);
      renderer.renderScene(cameraController.camera(), lighting);

      drawStaticLeftPanel(340.0f, renderer, lookedAtInFrame, selectedMesh, lighting, clearColor);
      drawWindowManager(instanceManager, moduleManager, backpackObjText, selectedMesh, renderer);
      drawGizmoWindow(renderer, selectedMesh);

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
