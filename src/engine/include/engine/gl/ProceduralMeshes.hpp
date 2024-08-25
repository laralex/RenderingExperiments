#pragma once

#include "engine/Prelude.hpp"
#include "engine/gl/GpuMesh.hpp"

namespace engine {

struct BoxMesh;
struct IcosphereMesh;

} // namespace engine

namespace engine::gl {

auto AllocateBoxMesh [[nodiscard]] (BoxMesh const& cpuMesh, GpuMesh::AttributesLayout layout) -> GpuMesh;

auto AllocateIcosphereMesh [[nodiscard]] (IcosphereMesh const& cpuMesh, GpuMesh::AttributesLayout layout) -> GpuMesh;

} // namespace engine::gl