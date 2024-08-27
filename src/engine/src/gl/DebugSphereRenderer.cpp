#include "engine/gl/DebugSphereRenderer.hpp"

#include "engine/IcosphereMesh.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Uniform.hpp"

namespace {

constexpr GLint UNIFORM_MVP_LOCATION = 0;

} // namespace

namespace engine::gl {

auto DebugSphereRenderer::Allocate(size_t maxSpheres) -> DebugSphereRenderer {
    constexpr GLint ATTRIB_POSITION_LOCATION        = 0;
    constexpr GLint ATTRIB_UV_LOCATION              = 1;
    constexpr GLint ATTRIB_NORMAL_LOCATION          = 2;
    constexpr GLint ATTRIB_COLOR_LOCATION           = 3;
    constexpr GLint ATTRIB_INSTANCE_MATRIX_LOCATION = 4;

    using T = SphereRendererInput::Sphere;

    auto mesh = IcosphereMesh::Generate({
        .numSubdivisions    = 0,
        .duplicateSeam      = false,
        .clockwiseTriangles = false,
    });

    DebugSphereRenderer renderer;
    renderer.meshPositionsBuffer_ = gl::GpuBuffer::Allocate(
        GL_ARRAY_BUFFER, GL_STATIC_DRAW, mesh.vertexPositions.data(),
        std::size(mesh.vertexPositions) * sizeof(mesh.vertexPositions[0]), "DebugSphereRenderer/TemplatePositionsVBO");
    renderer.meshAttributesBuffer_ = gl::GpuBuffer::Allocate(
        GL_ARRAY_BUFFER, GL_STATIC_DRAW, mesh.vertexData.data(),
        std::size(mesh.vertexData) * sizeof(mesh.vertexData[0]), "DebugSphereRenderer/TemplateVBO");
    renderer.instancesBuffer_ = gl::GpuBuffer::Allocate(
        GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, nullptr, maxSpheres * sizeof(T), "DebugSphereRenderer/InstancesVBO");
    renderer.indexBuffer_ = gl::GpuBuffer::Allocate(
        GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, mesh.indices.data(), std::size(mesh.indices) * sizeof(mesh.indices[0]),
        "DebugSphereRenderer/TemplateEBO");

    renderer.vao_ = gl::Vao::Allocate("DebugSphereRenderer/VAO");

    (void)gl::VaoMutableCtx{renderer.vao_}
        .MakeVertexAttribute(
            renderer.meshPositionsBuffer_,
            {.location        = ATTRIB_POSITION_LOCATION,
             .valuesPerVertex = 3,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(mesh.vertexPositions[0]),
             .offset          = 0})
        .MakeVertexAttribute(
            renderer.meshAttributesBuffer_,
            {.location        = ATTRIB_UV_LOCATION,
             .valuesPerVertex = 2,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(IcosphereMesh::Vertex),
             .offset          = offsetof(IcosphereMesh::Vertex, uv)})
        .MakeVertexAttribute(
            renderer.meshAttributesBuffer_,
            {.location        = ATTRIB_NORMAL_LOCATION,
             .valuesPerVertex = 3,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(IcosphereMesh::Vertex),
             .offset          = offsetof(IcosphereMesh::Vertex, normal)})
        .MakeVertexAttribute(
            renderer.instancesBuffer_,
            {.location        = ATTRIB_COLOR_LOCATION,
             .valuesPerVertex = 1,
             .datatype        = GL_INT,
             .stride          = sizeof(T),
             .offset          = offsetof(T, colorIdx),
             .instanceDivisor = 1})
        .MakeVertexAttribute(
            renderer.instancesBuffer_,
            {.location        = ATTRIB_INSTANCE_MATRIX_LOCATION,
             .numLocations    = 4,
             .valuesPerVertex = 4,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(T),
             .offset          = offsetof(T, transform),
             .offsetAdvance   = sizeof(glm::vec4),
             .instanceDivisor = 1})
        .MakeIndexed(renderer.indexBuffer_, GL_UNSIGNED_SHORT);

    gl::ShaderDefine const defines[] = {
        {.name = "ATTRIB_POSITION", .value = ATTRIB_POSITION_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "ATTRIB_UV", .value = ATTRIB_UV_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "ATTRIB_NORMAL", .value = ATTRIB_NORMAL_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "ATTRIB_COLOR", .value = ATTRIB_COLOR_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "ATTRIB_INSTANCE_MATRIX", .value = ATTRIB_INSTANCE_MATRIX_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_MVP", .value = UNIFORM_MVP_LOCATION, .type = gl::ShaderDefine::INT32},
    };

    auto maybeProgram = gl::LinkProgramFromFiles(
        "data/engine/shaders/instanced_simple.vert", "data/engine/shaders/color_palette.frag",
        CpuView{defines, std::size(defines)}, "DebugSphereRenderer");
    assert(maybeProgram);
    renderer.program_ = std::move(*maybeProgram);

    return renderer;
}

void DebugSphereRenderer::Render(glm::mat4 const& camera) const {
    auto programGuard = UniformCtx{program_};
    programGuard.SetUniformMatrix4(UNIFORM_MVP_LOCATION, glm::value_ptr(camera));
    RenderVaoInstanced(vao_, numInstances_);
}

void DebugSphereRenderer::Fill(std::vector<SphereRendererInput::Sphere> const& spheres) {
    using T       = typename std::decay<decltype(*spheres.begin())>::type;
    auto numBytes = std::min(instancesBuffer_.SizeBytes(), static_cast<int32_t>(std::size(spheres) * sizeof(T)));
    instancesBuffer_.Fill(spheres.data(), numBytes);
    numInstances_ = std::size(spheres);
}

} // namespace engine::gl
