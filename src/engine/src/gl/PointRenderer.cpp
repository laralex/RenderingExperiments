#include "engine/gl/PointRenderer.hpp"

// #include "engine/IcosphereMesh.hpp"
#include "engine/BoxMesh.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Uniform.hpp"

namespace {

constexpr GLint UNIFORM_MVP_LOCATION = 0;

} // namespace

namespace engine::gl {

auto PointRenderer::Allocate(size_t maxPoints) -> PointRenderer {
    constexpr GLint ATTRIB_POSITION_LOCATION        = 0;
    constexpr GLint ATTRIB_UV_LOCATION              = 1;
    constexpr GLint ATTRIB_NORMAL_LOCATION          = 2;
    constexpr GLint ATTRIB_INSTANCE_COLOR_LOCATION  = 3;
    constexpr GLint ATTRIB_INSTANCE_MATRIX_LOCATION = 4;

    using T = PointRendererInput::Point;

    // auto mesh = IcosphereMesh::Generate({.numSubdivisions = 0, .duplicateSeam = true, .clockwiseTriangles = false});
    auto mesh = BoxMesh::Generate();

    PointRenderer renderer;
    size_t numPositionsBytes = std::size(mesh.vertexPositions) * sizeof(mesh.vertexPositions[0]);
    renderer.meshPositionsBuffer_ = gl::GpuBuffer::Allocate(
        GL_ARRAY_BUFFER, GL_STATIC_DRAW,
        CpuMemory<const void>{mesh.vertexPositions.data(), numPositionsBytes}, "PointRenderer/TemplatePositionsVBO");
    size_t numDataBytes = std::size(mesh.vertexPositions) * sizeof(mesh.vertexPositions[0]);
    renderer.meshAttributesBuffer_ = gl::GpuBuffer::Allocate(
        GL_ARRAY_BUFFER, GL_STATIC_DRAW,
        CpuMemory<const void>{mesh.vertexData.data(), numDataBytes}, "PointRenderer/TemplateVBO");
    renderer.instancesBuffer_ = gl::GpuBuffer::Allocate(
        GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW,
        CpuMemory<void const>{nullptr, maxPoints * sizeof(T)}, "PointRenderer/InstancesVBO");
    renderer.indexBuffer_ = gl::GpuBuffer::Allocate(
        GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW,
        CpuMemory<void const>{mesh.indices.data(), std::size(mesh.indices) * sizeof(mesh.indices[0])},
        "PointRenderer/TemplateEBO");

    renderer.vao_ = gl::Vao::Allocate("PointRenderer/VAO");

    GLenum indexType;
    if constexpr (sizeof(mesh.indices[0]) == 1) {
        indexType = GL_UNSIGNED_BYTE;
    } else if constexpr (sizeof(mesh.indices[0]) == 2) {
        indexType = GL_UNSIGNED_SHORT;
    } else if constexpr (sizeof(mesh.indices[0]) == 4) {
        indexType = GL_UNSIGNED_INT;
    }

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
             .stride          = sizeof(decltype(mesh)::Vertex),
             .offset          = offsetof(decltype(mesh)::Vertex, uv)})
        .MakeVertexAttribute(
            renderer.meshAttributesBuffer_,
            {.location        = ATTRIB_NORMAL_LOCATION,
             .valuesPerVertex = 3,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(decltype(mesh)::Vertex),
             .offset          = offsetof(decltype(mesh)::Vertex, normal)})
        .MakeVertexAttribute(
            renderer.instancesBuffer_,
            {.location        = ATTRIB_INSTANCE_COLOR_LOCATION,
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
        .MakeIndexed(renderer.indexBuffer_, indexType);

    gl::ShaderDefine const defines[] = {
        {.name = "ATTRIB_POSITION", .value = ATTRIB_POSITION_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "ATTRIB_UV", .value = ATTRIB_UV_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "ATTRIB_NORMAL", .value = ATTRIB_NORMAL_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "ATTRIB_COLOR", .value = ATTRIB_INSTANCE_COLOR_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "ATTRIB_INSTANCE_MATRIX", .value = ATTRIB_INSTANCE_MATRIX_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_MVP", .value = UNIFORM_MVP_LOCATION, .type = gl::ShaderDefine::INT32},
    };

    auto maybeProgram = gl::LinkProgramFromFiles(
        "data/engine/shaders/instanced_simple.vert", "data/engine/shaders/color_palette.frag",
        CpuView{defines, std::size(defines)}, "PointRenderer");
    assert(maybeProgram);
    renderer.program_ = std::move(*maybeProgram);

    renderer.lastInstance_ = maxPoints;

    return renderer;
}

void PointRenderer::Render(glm::mat4 const& camera, int32_t firstInstance, int32_t numInstances) const {
    if (lastInstance_ <= 0 || firstInstance >= lastInstance_) {
        // XLOGW("Limit of points is <= 0 in PointRenderer", 0);
        return;
    }
    auto programGuard = UniformCtx{program_};
    programGuard.SetUniformMatrix4x4(UNIFORM_MVP_LOCATION, glm::value_ptr(camera));
    RenderVaoInstanced(vao_, std::min(firstInstance, lastInstance_), std::min(lastInstance_ - firstInstance, numInstances));
}

void PointRenderer::LimitInstances(int32_t numInstances) {
    lastInstance_ =
        std::min(numInstances, static_cast<int32_t>(instancesBuffer_.SizeBytes() / sizeof(PointRendererInput::Point)));
}

void PointRenderer::Fill(
    std::vector<PointRendererInput::Point> const& points, int32_t numPoints, int32_t numPointsOffset) {
    using T               = typename std::decay<decltype(*points.begin())>::type;
    if (std::size(points) == 0 | numPoints == 0) {\
        return;
    }
    auto const byteOffset = numPointsOffset * sizeof(T);
    auto const numBytes   = std::min(
        instancesBuffer_.SizeBytes() - byteOffset, // buffer limit
        numPoints * sizeof(T)                      // argument limit
    );
    instancesBuffer_.Fill(CpuMemory<GLvoid const>{points.data(), numBytes}, byteOffset);
}

} // namespace engine::gl
