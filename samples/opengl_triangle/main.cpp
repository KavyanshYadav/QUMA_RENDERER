#include <SDL.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "engine/modules/IModule.hpp"
#include "engine/modules/ModuleContract.hpp"
#include "engine/modules/ModuleManager.hpp"
#include "engine/platform/IPlatformBackend.hpp"
#include "engine/platform/IWindowSystem.hpp"
#include "engine/platform/PlatformBackendFactory.hpp"
#include "engine/render/ICommandContext.hpp"
#include "engine/render/IRenderBackend.hpp"
#include "engine/render/IRenderDevice.hpp"
#include "engine/render/RenderBackendFactory.hpp"
#include "engine/render/RenderTypes.hpp"

namespace {

class TriangleDemoModule final : public engine::modules::IModule {
public:
  void onLoad() override { std::cout << "[module] loaded\n"; }
  void onStart() override { std::cout << "[module] started\n"; }
  void onStop() override { std::cout << "[module] stopped\n"; }
  void onUnload() override { std::cout << "[module] unloaded\n"; }
};

constexpr engine::modules::Version kEngineApiVersion{0, 1, 0};

[[nodiscard]] engine::modules::ModuleDescriptor makeDemoModuleDescriptor() {
  return engine::modules::ModuleDescriptor{
      .id = "sample.opengl_triangle",
      .category = "sample",
      .moduleVersion = {0, 1, 0},
      .requiredApiVersion = kEngineApiVersion,
      .swapPolicy = engine::modules::SwapPolicy::RuntimeSwappable,
      .dependencies = {},
      .conflicts = {}};
}

[[nodiscard]] engine::render::ShaderCreateInfo makeShader(engine::render::ShaderStage stage, const char* source) {
  return engine::render::ShaderCreateInfo{
      .stage = stage,
      .byteCode = reinterpret_cast<const std::byte*>(source),
      .byteCodeSize = std::char_traits<char>::length(source)};
}

[[nodiscard]] std::uint64_t parseMaxFrames(int argc, char** argv) {
  constexpr std::string_view prefix = "--frames=";
  for (int index = 1; index < argc; ++index) {
    const std::string_view argument{argv[index]};
    if (!argument.starts_with(prefix)) {
      continue;
    }

    const auto valueView = argument.substr(prefix.size());
    try {
      return static_cast<std::uint64_t>(std::stoull(std::string{valueView}));
    } catch (const std::exception&) {
      return 600;
    }
  }

  return 600;
}

} // namespace

int main(int argc, char** argv) {
  try {
    const std::uint64_t maxFrames = parseMaxFrames(argc, argv);
    engine::modules::ModuleManager moduleManager{kEngineApiVersion};
    auto moduleDescriptor = makeDemoModuleDescriptor();
    auto validation = moduleManager.validate({moduleDescriptor});
    if (!validation.ok) {
      for (const auto& error : validation.errors) {
        std::cerr << "Module validation error: " << error << '\n';
      }
      return 1;
    }

    TriangleDemoModule demoModule;
    demoModule.onLoad();
    demoModule.onStart();

    auto platformBackend = engine::platform::createPlatformBackend();
    auto& windowSystem = platformBackend->windowSystem();

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetSwapInterval(1);

    const auto windowId = windowSystem.createWindow(
        engine::platform::WindowCreateInfo{.title = "QumaRenderer OpenGL Triangle", .size = {1280, 720}, .resizable = true, .highDpi = true});

    void* nativeWindow = windowSystem.nativeWindowHandle(windowId);
    if (nativeWindow == nullptr) {
      throw std::runtime_error("Failed to retrieve native window handle");
    }

    auto* sdlWindow = static_cast<SDL_Window*>(nativeWindow);
    SDL_GLContext glContext = SDL_GL_CreateContext(sdlWindow);
    if (glContext == nullptr) {
      throw std::runtime_error(std::string{"SDL_GL_CreateContext failed: "} + SDL_GetError());
    }

    if (SDL_GL_MakeCurrent(sdlWindow, glContext) != 0) {
      throw std::runtime_error(std::string{"SDL_GL_MakeCurrent failed: "} + SDL_GetError());
    }

    auto renderBackend = engine::render::createRenderBackend(engine::render::RenderBackendType::OpenGL);
    auto renderDevice = renderBackend->createDevice();
    auto commandContext = renderDevice->createCommandContext();

    static constexpr char kVertexShader[] = R"(
#version 330 core

const vec2 kPositions[3] = vec2[3](
  vec2(-0.8, -0.8),
  vec2(0.8, -0.8),
  vec2(0.0, 0.8)
);

void main() {
  gl_Position = vec4(kPositions[gl_VertexID], 0.0, 1.0);
}
)";

    static constexpr char kFragmentShader[] = R"(
#version 330 core

out vec4 outColor;

void main() {
  outColor = vec4(0.10, 0.70, 0.95, 1.0);
}
)";

    const auto vertexShader = renderDevice->createShader(makeShader(engine::render::ShaderStage::Vertex, kVertexShader));
    const auto fragmentShader = renderDevice->createShader(makeShader(engine::render::ShaderStage::Fragment, kFragmentShader));
    const auto pipeline = renderDevice->createGraphicsPipeline(
        engine::render::GraphicsPipelineCreateInfo{.vertexShader = vertexShader,
                                                    .fragmentShader = fragmentShader,
                                                    .topology = engine::render::PrimitiveTopology::TriangleList});

    std::uint64_t frameIndex = 0;
    bool running = true;
    while (running && !windowSystem.shouldClose(windowId) && frameIndex < maxFrames) {
      engine::platform::PlatformEventQueue events;
      windowSystem.pollEvents(events);

      for (const auto& event : events) {
        if (event.type == engine::platform::PlatformEventType::QuitRequested ||
            (event.type == engine::platform::PlatformEventType::WindowClosed && event.windowId == windowId)) {
          running = false;
          break;
        }
      }

      auto extent = windowSystem.framebufferExtent(windowId).value_or(engine::platform::Extent2D{1280, 720});
      commandContext->beginFrame(engine::render::FrameGraphFrameInfo{.frameIndex = frameIndex++, .renderExtent = extent});
      commandContext->bindPipeline(pipeline);
      commandContext->draw(3);
      commandContext->endFrame();

      SDL_GL_SwapWindow(sdlWindow);
    }

    renderDevice->destroyPipeline(pipeline);
    renderDevice->destroyShader(fragmentShader);
    renderDevice->destroyShader(vertexShader);

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
