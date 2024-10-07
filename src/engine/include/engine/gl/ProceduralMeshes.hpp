#pragma once

#include "engine/Precompiled.hpp"
#include "engine/gl/GpuMesh.hpp"

namespace engine {

struct BoxMesh;
struct IcosphereMesh;
struct UvSphereMesh;
struct PlaneMesh;

} // namespace engine

namespace engine::gl {

auto AllocateBoxMesh [[nodiscard]] (GlContext& gl, BoxMesh const& cpuMesh, GpuMesh::AttributesLayout layout)
-> GpuMesh;

auto AllocateIcosphereMesh
    [[nodiscard]] (GlContext& gl, IcosphereMesh const& cpuMesh, GpuMesh::AttributesLayout layout) -> GpuMesh;

auto AllocateUvSphereMesh
    [[nodiscard]] (GlContext& gl, UvSphereMesh const& cpuMesh, GpuMesh::AttributesLayout layout) -> GpuMesh;

auto AllocatePlaneMesh [[nodiscard]] (GlContext& gl, PlaneMesh const& cpuMesh, GpuMesh::AttributesLayout layout)
-> GpuMesh;

} // namespace engine::gl