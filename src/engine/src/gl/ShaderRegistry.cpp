#include "engine/gl/ShaderRegistry.hpp"
#include "engine/Assets.hpp"

namespace engine::gl {

ShaderRegistry::ShaderRegistry() {
    auto insertV = [](std::unordered_map<VertexShaderId, std::string>& map, VertexShaderId id, std::string filepath) {
        std::string code = LoadTextFile(filepath);
        map.emplace(id, code);
    };
    insertV(vertexSrcCode_, VertexShaderId::AXES, "data/engine/shaders/axes.vert");
    insertV(vertexSrcCode_, VertexShaderId::BOX, "data/engine/shaders/box.vert");
    insertV(vertexSrcCode_, VertexShaderId::FRUSTUM, "data/engine/shaders/frustum.vert");
    insertV(vertexSrcCode_, VertexShaderId::TRIANGLE_FULLSCREEN, "data/engine/shaders/triangle_fullscreen.vert");

    auto insertF = [](std::unordered_map<FragmentShaderId, std::string>& map, FragmentShaderId id,
                      std::string filepath) {
        std::string code = LoadTextFile(filepath);
        map.emplace(id, code);
    };
    insertF(fragmentSrcCode_, FragmentShaderId::AXES, "data/engine/shaders/axes.frag");
    insertF(fragmentSrcCode_, FragmentShaderId::BLIT, "data/engine/shaders/blit.frag");
    insertF(fragmentSrcCode_, FragmentShaderId::CONSTANT, "data/engine/shaders/constant.frag");
}

ShaderRegistry::~ShaderRegistry() {
    for (auto& vs : vertexShaders_) {
        GLCALL(glDeleteShader(vs.second));
    }
    vertexShaders_.clear();

    for (auto& fs : fragmentShaders_) {
        GLCALL(glDeleteShader(fs.second));
    }
    fragmentShaders_.clear();
}

} // namespace engine::gl
