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
#include <stdexcept>
#include <string>

#include "MeshRenderEngine.hpp"
#include "ObjLoader.hpp"
#include "engine/modules/IModule.hpp"
#include "engine/modules/ModuleContract.hpp"
#include "engine/modules/ModuleManager.hpp"
#include "engine/platform/IPlatformBackend.hpp"
#include "engine/platform/IWindowSystem.hpp"
#include "engine/platform/PlatformBackendFactory.hpp"

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
      .moduleVersion = {0, 3, 0},
      .requiredApiVersion = kEngineApiVersion,
      .swapPolicy = engine::modules::SwapPolicy::RuntimeSwappable,
      .dependencies = {},
      .conflicts = {}};
}

[[nodiscard]] std::string backpackObjSource() {
  return R"OBJ(
# stylized backpack-like box mesh
v -0.8 -1.0 -0.4
v 0.8 -1.0 -0.4
v 0.8 1.0 -0.4
v -0.8 1.0 -0.4
v -0.8 -1.0 0.4
v 0.8 -1.0 0.4
v 0.8 1.0 0.4
v -0.8 1.0 0.4
vt 0.0 0.0
vt 1.0 0.0
vt 1.0 1.0
vt 0.0 1.0
vn 0 0 -1
vn 0 0 1
vn -1 0 0
vn 1 0 0
vn 0 -1 0
vn 0 1 0
f 1/1/1 2/2/1 3/3/1 4/4/1
f 5/1/2 8/4/2 7/3/2 6/2/2
f 1/1/3 4/2/3 8/3/3 5/4/3
f 2/1/4 6/2/4 7/3/4 3/4/4
f 1/1/5 5/2/5 6/3/5 2/4/5
f 4/1/6 3/2/6 7/3/6 8/4/6
)OBJ";
}

} // namespace

int main() {
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
        .title = "QumaRenderer - Mesh Engine (.obj + Lighting)", .size = {1280, 720}, .resizable = true, .highDpi = true});

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

    sample::rendering::MeshRenderEngine renderer{sdlWindow, glContext};
    auto mesh = sample::rendering::loadObjFromString(backpackObjSource());
    mesh.material.name = "backpack_material";
    mesh.material.baseColor[0] = 0.45f;
    mesh.material.baseColor[1] = 0.62f;
    mesh.material.baseColor[2] = 0.30f;
    mesh.material.metallic = 0.1f;
    mesh.material.roughness = 0.68f;
    renderer.setMesh(mesh);

    sample::rendering::CameraState camera{};
    sample::rendering::SceneLighting lighting{};

    float clearColor[3] = {0.07f, 0.08f, 0.11f};
    float rotationSpeed = 30.0f;
    float rotation = 0.0f;

    bool running = true;
    std::uint64_t currentTicks = SDL_GetPerformanceCounter();
    while (running && !windowSystem.shouldClose(windowId)) {
      const std::uint64_t newTicks = SDL_GetPerformanceCounter();
      const double deltaSeconds = static_cast<double>(newTicks - currentTicks) / static_cast<double>(SDL_GetPerformanceFrequency());
      currentTicks = newTicks;

      SDL_Event event;
      while (SDL_PollEvent(&event) == 1) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) {
          running = false;
        }
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) {
          running = false;
        }
      }

      rotation += rotationSpeed * static_cast<float>(deltaSeconds) * 0.0174532925f;

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplSDL2_NewFrame();
      ImGui::NewFrame();

      ImGui::Begin("Mesh Engine Controls");
      ImGui::Text("Production-oriented mesh pipeline skeleton");
      ImGui::Text("Mesh: backpack.obj (embedded sample)");
      ImGui::Text("Triangles: %u", renderer.triangleCount());
      ImGui::SliderFloat("Rotation Speed", &rotationSpeed, 0.0f, 90.0f);
      ImGui::SliderFloat3("Light Position", lighting.lightPosition, -8.0f, 8.0f);
      ImGui::ColorEdit3("Light Color", lighting.lightColor);
      ImGui::SliderFloat("Ambient", &lighting.ambientIntensity, 0.0f, 1.0f);
      ImGui::ColorEdit3("Base Color", mesh.material.baseColor);
      ImGui::SliderFloat("Metallic", &mesh.material.metallic, 0.0f, 1.0f);
      ImGui::SliderFloat("Roughness", &mesh.material.roughness, 0.04f, 1.0f);
      ImGui::ColorEdit3("Background", clearColor);
      ImGui::End();

      renderer.setMesh(mesh);
      int w = 0;
      int h = 0;
      SDL_GL_GetDrawableSize(sdlWindow, &w, &h);
      renderer.resize(w, h);
      renderer.beginFrame(clearColor[0], clearColor[1], clearColor[2]);
      renderer.renderMesh(camera, lighting, rotation);

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
