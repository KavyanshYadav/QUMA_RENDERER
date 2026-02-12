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
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "engine/modules/IModule.hpp"
#include "engine/modules/ModuleContract.hpp"
#include "engine/modules/ModuleManager.hpp"
#include "engine/platform/IPlatformBackend.hpp"
#include "engine/platform/IWindowSystem.hpp"
#include "engine/platform/PlatformBackendFactory.hpp"

namespace {

struct Mat4 {
  std::array<float, 16> value{};
};

[[nodiscard]] Mat4 identity() {
  Mat4 matrix{};
  matrix.value = {1.0f, 0.0f, 0.0f, 0.0f,
                  0.0f, 1.0f, 0.0f, 0.0f,
                  0.0f, 0.0f, 1.0f, 0.0f,
                  0.0f, 0.0f, 0.0f, 1.0f};
  return matrix;
}

[[nodiscard]] Mat4 multiply(const Mat4& left, const Mat4& right) {
  Mat4 out{};
  for (int row = 0; row < 4; ++row) {
    for (int col = 0; col < 4; ++col) {
      float sum = 0.0f;
      for (int k = 0; k < 4; ++k) {
        sum += left.value[k * 4 + row] * right.value[col * 4 + k];
      }
      out.value[col * 4 + row] = sum;
    }
  }
  return out;
}

[[nodiscard]] Mat4 perspective(const float fovRadians, const float aspectRatio, const float nearPlane, const float farPlane) {
  Mat4 matrix{};
  const float tanHalf = std::tan(fovRadians * 0.5f);
  matrix.value = {1.0f / (aspectRatio * tanHalf), 0.0f, 0.0f, 0.0f,
                  0.0f, 1.0f / tanHalf, 0.0f, 0.0f,
                  0.0f, 0.0f, -(farPlane + nearPlane) / (farPlane - nearPlane), -1.0f,
                  0.0f, 0.0f, -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane), 0.0f};
  return matrix;
}

[[nodiscard]] Mat4 translate(const float x, const float y, const float z) {
  Mat4 matrix = identity();
  matrix.value[12] = x;
  matrix.value[13] = y;
  matrix.value[14] = z;
  return matrix;
}

[[nodiscard]] Mat4 rotateY(const float radians) {
  Mat4 matrix = identity();
  const float c = std::cos(radians);
  const float s = std::sin(radians);
  matrix.value[0] = c;
  matrix.value[2] = -s;
  matrix.value[8] = s;
  matrix.value[10] = c;
  return matrix;
}

[[nodiscard]] Mat4 rotateX(const float radians) {
  Mat4 matrix = identity();
  const float c = std::cos(radians);
  const float s = std::sin(radians);
  matrix.value[5] = c;
  matrix.value[6] = s;
  matrix.value[9] = -s;
  matrix.value[10] = c;
  return matrix;
}

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
      .moduleVersion = {0, 2, 0},
      .requiredApiVersion = kEngineApiVersion,
      .swapPolicy = engine::modules::SwapPolicy::RuntimeSwappable,
      .dependencies = {},
      .conflicts = {}};
}

[[nodiscard]] GLuint compileShader(const GLenum type, const char* source) {
  const GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  GLint success = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (success != GL_TRUE) {
    GLint logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    std::string log(static_cast<std::size_t>(logLength), '\0');
    glGetShaderInfoLog(shader, logLength, nullptr, log.data());
    glDeleteShader(shader);
    throw std::runtime_error("Shader compilation failed: " + log);
  }

  return shader;
}

[[nodiscard]] GLuint createProgram(const char* vertexSource, const char* fragmentSource) {
  const GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexSource);
  const GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

  const GLuint program = glCreateProgram();
  glAttachShader(program, vertex);
  glAttachShader(program, fragment);
  glLinkProgram(program);

  GLint success = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  glDeleteShader(vertex);
  glDeleteShader(fragment);

  if (success != GL_TRUE) {
    GLint logLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    std::string log(static_cast<std::size_t>(logLength), '\0');
    glGetProgramInfoLog(program, logLength, nullptr, log.data());
    glDeleteProgram(program);
    throw std::runtime_error("Program link failed: " + log);
  }

  return program;
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

    TriangleDemoModule demoModule;
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
        .title = "QumaRenderer - Rotating Lit Cube (SDL + OpenGL + ImGui)", .size = {1280, 720}, .resizable = true, .highDpi = true});

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
    glEnable(GL_DEPTH_TEST);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(sdlWindow, glContext);
    ImGui_ImplOpenGL3_Init("#version 330");

    static constexpr float cubeVertices[] = {
        -1.0f, -1.0f, -1.0f, 0.0f,  0.0f,  -1.0f,
        1.0f,  -1.0f, -1.0f, 0.0f,  0.0f,  -1.0f,
        1.0f,  1.0f,  -1.0f, 0.0f,  0.0f,  -1.0f,
        -1.0f, 1.0f,  -1.0f, 0.0f,  0.0f,  -1.0f,

        -1.0f, -1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
        1.0f,  -1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
        1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        -1.0f, 1.0f,  1.0f,  0.0f,  0.0f,  1.0f,

        -1.0f, -1.0f, -1.0f, -1.0f, 0.0f,  0.0f,
        -1.0f, 1.0f,  -1.0f, -1.0f, 0.0f,  0.0f,
        -1.0f, 1.0f,  1.0f,  -1.0f, 0.0f,  0.0f,
        -1.0f, -1.0f, 1.0f,  -1.0f, 0.0f,  0.0f,

        1.0f,  -1.0f, -1.0f, 1.0f,  0.0f,  0.0f,
        1.0f,  1.0f,  -1.0f, 1.0f,  0.0f,  0.0f,
        1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,
        1.0f,  -1.0f, 1.0f,  1.0f,  0.0f,  0.0f,

        -1.0f, -1.0f, -1.0f, 0.0f,  -1.0f, 0.0f,
        -1.0f, -1.0f, 1.0f,  0.0f,  -1.0f, 0.0f,
        1.0f,  -1.0f, 1.0f,  0.0f,  -1.0f, 0.0f,
        1.0f,  -1.0f, -1.0f, 0.0f,  -1.0f, 0.0f,

        -1.0f, 1.0f,  -1.0f, 0.0f,  1.0f,  0.0f,
        -1.0f, 1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        1.0f,  1.0f,  -1.0f, 0.0f,  1.0f,  0.0f,
    };

    static constexpr unsigned int cubeIndices[] = {
        0,  1,  2,  2,  3,  0,
        4,  5,  6,  6,  7,  4,
        8,  9,  10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20,
    };

    static constexpr char vertexShader[] = R"(
#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vNormal;
out vec3 vWorldPos;

void main() {
  vec4 worldPos = uModel * vec4(aPosition, 1.0);
  vWorldPos = worldPos.xyz;
  vNormal = mat3(transpose(inverse(uModel))) * aNormal;
  gl_Position = uProjection * uView * worldPos;
}
)";

    static constexpr char fragmentShader[] = R"(
#version 330 core
in vec3 vNormal;
in vec3 vWorldPos;
out vec4 outColor;

uniform vec3 uObjectColor;
uniform vec3 uLightColor;
uniform vec3 uLightPos;
uniform vec3 uCameraPos;
uniform float uAmbientStrength;
uniform float uDiffuseStrength;
uniform float uSpecularStrength;
uniform float uShininess;

void main() {
  vec3 norm = normalize(vNormal);
  vec3 lightDir = normalize(uLightPos - vWorldPos);
  float diff = max(dot(norm, lightDir), 0.0);

  vec3 viewDir = normalize(uCameraPos - vWorldPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), uShininess);

  vec3 ambient = uAmbientStrength * uLightColor;
  vec3 diffuse = uDiffuseStrength * diff * uLightColor;
  vec3 specular = uSpecularStrength * spec * uLightColor;
  vec3 lighting = (ambient + diffuse + specular) * uObjectColor;

  outColor = vec4(lighting, 1.0);
}
)";

    const GLuint program = createProgram(vertexShader, fragmentShader);

    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    float objectColor[3] = {0.25f, 0.75f, 0.95f};
    float lightColor[3] = {1.0f, 1.0f, 1.0f};
    float lightPos[3] = {2.8f, 2.0f, 2.5f};
    float clearColor[3] = {0.07f, 0.08f, 0.11f};
    float rotationSpeed = 45.0f;
    float ambientStrength = 0.20f;
    float diffuseStrength = 0.90f;
    float specularStrength = 0.35f;
    float shininess = 32.0f;

    bool running = true;
    std::uint64_t currentTicks = SDL_GetPerformanceCounter();
    float angle = 0.0f;

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

      angle += rotationSpeed * static_cast<float>(deltaSeconds) * 0.0174532925f;

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplSDL2_NewFrame();
      ImGui::NewFrame();

      ImGui::Begin("Render Controls");
      ImGui::Text("SDL + OpenGL + ImGui + GLAD");
      ImGui::SliderFloat("Rotation Speed (deg/s)", &rotationSpeed, 0.0f, 180.0f);
      ImGui::ColorEdit3("Object Color", objectColor);
      ImGui::ColorEdit3("Light Color", lightColor);
      ImGui::SliderFloat3("Light Position", lightPos, -5.0f, 5.0f);
      ImGui::SliderFloat("Ambient", &ambientStrength, 0.0f, 1.0f);
      ImGui::SliderFloat("Diffuse", &diffuseStrength, 0.0f, 2.0f);
      ImGui::SliderFloat("Specular", &specularStrength, 0.0f, 1.5f);
      ImGui::SliderFloat("Shininess", &shininess, 2.0f, 128.0f);
      ImGui::ColorEdit3("Background", clearColor);
      ImGui::Text("Frame time: %.3f ms", static_cast<float>(deltaSeconds * 1000.0));
      ImGui::End();

      int drawableWidth = 0;
      int drawableHeight = 0;
      SDL_GL_GetDrawableSize(sdlWindow, &drawableWidth, &drawableHeight);
      glViewport(0, 0, drawableWidth, drawableHeight);
      glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      const float aspect = drawableHeight > 0 ? static_cast<float>(drawableWidth) / static_cast<float>(drawableHeight) : 16.0f / 9.0f;
      const Mat4 model = multiply(rotateY(angle), rotateX(angle * 0.5f));
      const Mat4 view = translate(0.0f, 0.0f, -6.0f);
      const Mat4 projection = perspective(45.0f * 0.0174532925f, aspect, 0.1f, 100.0f);

      glUseProgram(program);
      glUniformMatrix4fv(glGetUniformLocation(program, "uModel"), 1, GL_FALSE, model.value.data());
      glUniformMatrix4fv(glGetUniformLocation(program, "uView"), 1, GL_FALSE, view.value.data());
      glUniformMatrix4fv(glGetUniformLocation(program, "uProjection"), 1, GL_FALSE, projection.value.data());
      glUniform3f(glGetUniformLocation(program, "uObjectColor"), objectColor[0], objectColor[1], objectColor[2]);
      glUniform3f(glGetUniformLocation(program, "uLightColor"), lightColor[0], lightColor[1], lightColor[2]);
      glUniform3f(glGetUniformLocation(program, "uLightPos"), lightPos[0], lightPos[1], lightPos[2]);
      glUniform3f(glGetUniformLocation(program, "uCameraPos"), 0.0f, 0.0f, 6.0f);
      glUniform1f(glGetUniformLocation(program, "uAmbientStrength"), ambientStrength);
      glUniform1f(glGetUniformLocation(program, "uDiffuseStrength"), diffuseStrength);
      glUniform1f(glGetUniformLocation(program, "uSpecularStrength"), specularStrength);
      glUniform1f(glGetUniformLocation(program, "uShininess"), shininess);

      glBindVertexArray(vao);
      glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      SDL_GL_SwapWindow(sdlWindow);
    }

    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);

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
