#pragma once

#include <string>

namespace sample::app {

[[nodiscard]] inline std::string backpackObjSource() {
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

} // namespace sample::app
