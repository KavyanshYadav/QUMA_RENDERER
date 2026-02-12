#include "ObjLoader.hpp"

#include <array>
#include <charconv>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace sample::rendering {
namespace {

struct FaceIndex {
  int position = 0;
  int uv = 0;
  int normal = 0;
};

[[nodiscard]] float parseFloat(const std::string_view token) {
  float value = 0.0f;
  const auto* begin = token.data();
  const auto* end = token.data() + token.size();
  const auto result = std::from_chars(begin, end, value);
  if (result.ec != std::errc{}) {
    throw std::runtime_error("Invalid OBJ float token: " + std::string(token));
  }
  return value;
}

[[nodiscard]] int parseInt(const std::string_view token) {
  int value = 0;
  const auto* begin = token.data();
  const auto* end = token.data() + token.size();
  const auto result = std::from_chars(begin, end, value);
  if (result.ec != std::errc{}) {
    throw std::runtime_error("Invalid OBJ index token: " + std::string(token));
  }
  return value;
}

[[nodiscard]] std::vector<std::string_view> splitBySpace(const std::string_view line) {
  std::vector<std::string_view> tokens;
  std::size_t cursor = 0;
  while (cursor < line.size()) {
    while (cursor < line.size() && line[cursor] == ' ') {
      ++cursor;
    }
    if (cursor >= line.size()) {
      break;
    }

    std::size_t end = cursor;
    while (end < line.size() && line[end] != ' ') {
      ++end;
    }

    tokens.emplace_back(line.substr(cursor, end - cursor));
    cursor = end;
  }
  return tokens;
}

[[nodiscard]] FaceIndex parseFaceIndex(std::string_view token) {
  FaceIndex out{};

  const auto slashA = token.find('/');
  if (slashA == std::string_view::npos) {
    out.position = parseInt(token);
    return out;
  }

  out.position = parseInt(token.substr(0, slashA));
  const auto slashB = token.find('/', slashA + 1);
  if (slashB == std::string_view::npos) {
    out.uv = parseInt(token.substr(slashA + 1));
    return out;
  }

  if (slashB > slashA + 1) {
    out.uv = parseInt(token.substr(slashA + 1, slashB - slashA - 1));
  }
  if (slashB + 1 < token.size()) {
    out.normal = parseInt(token.substr(slashB + 1));
  }

  return out;
}

} // namespace

MeshData loadObjFromString(const std::string_view objSource) {
  std::vector<std::array<float, 3>> positions;
  std::vector<std::array<float, 3>> normals;
  std::vector<std::array<float, 2>> uvs;

  MeshData mesh{};
  std::unordered_map<std::string, std::uint32_t> vertexCache;

  std::size_t lineStart = 0;
  while (lineStart < objSource.size()) {
    auto lineEnd = objSource.find('\n', lineStart);
    if (lineEnd == std::string_view::npos) {
      lineEnd = objSource.size();
    }

    std::string_view line = objSource.substr(lineStart, lineEnd - lineStart);
    if (!line.empty() && line.back() == '\r') {
      line.remove_suffix(1);
    }

    if (!line.empty() && line[0] != '#') {
      auto tokens = splitBySpace(line);
      if (!tokens.empty()) {
        if (tokens[0] == "v" && tokens.size() >= 4) {
          positions.push_back({parseFloat(tokens[1]), parseFloat(tokens[2]), parseFloat(tokens[3])});
        } else if (tokens[0] == "vn" && tokens.size() >= 4) {
          normals.push_back({parseFloat(tokens[1]), parseFloat(tokens[2]), parseFloat(tokens[3])});
        } else if (tokens[0] == "vt" && tokens.size() >= 3) {
          uvs.push_back({parseFloat(tokens[1]), parseFloat(tokens[2])});
        } else if (tokens[0] == "f" && tokens.size() >= 4) {
          std::vector<std::uint32_t> faceVertices;
          for (std::size_t i = 1; i < tokens.size(); ++i) {
            const auto key = std::string(tokens[i]);
            const auto found = vertexCache.find(key);
            if (found != vertexCache.end()) {
              faceVertices.push_back(found->second);
              continue;
            }

            const auto face = parseFaceIndex(tokens[i]);
            Vertex vertex{};
            if (face.position > 0 && static_cast<std::size_t>(face.position - 1) < positions.size()) {
              const auto& position = positions[static_cast<std::size_t>(face.position - 1)];
              vertex.position[0] = position[0];
              vertex.position[1] = position[1];
              vertex.position[2] = position[2];
            }
            if (face.normal > 0 && static_cast<std::size_t>(face.normal - 1) < normals.size()) {
              const auto& normal = normals[static_cast<std::size_t>(face.normal - 1)];
              vertex.normal[0] = normal[0];
              vertex.normal[1] = normal[1];
              vertex.normal[2] = normal[2];
            }
            if (face.uv > 0 && static_cast<std::size_t>(face.uv - 1) < uvs.size()) {
              const auto& uv = uvs[static_cast<std::size_t>(face.uv - 1)];
              vertex.uv[0] = uv[0];
              vertex.uv[1] = uv[1];
            }

            const auto newIndex = static_cast<std::uint32_t>(mesh.vertices.size());
            mesh.vertices.push_back(vertex);
            vertexCache.emplace(key, newIndex);
            faceVertices.push_back(newIndex);
          }

          for (std::size_t i = 1; i + 1 < faceVertices.size(); ++i) {
            mesh.indices.push_back(faceVertices[0]);
            mesh.indices.push_back(faceVertices[i]);
            mesh.indices.push_back(faceVertices[i + 1]);
          }
        }
      }
    }

    lineStart = lineEnd + 1;
  }

  return mesh;
}

} // namespace sample::rendering
