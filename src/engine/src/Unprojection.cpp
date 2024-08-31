#include "engine/Unprojection.hpp"

namespace engine {

// adopted from paper
// https://web.archive.org/web/20120531231005/http://crazyjoke.free.fr/doc/3D/plane%20extraction.pdf
FrustumPlanes CameraToPlanes(glm::mat4 const& mvp, bool normalize) {
    FrustumPlanes planes;

    planes.left = glm::vec4{
        mvp[3][0] + mvp[0][0],
        mvp[3][1] + mvp[0][1],
        mvp[3][2] + mvp[0][2],
        mvp[3][3] + mvp[0][3],
    };

    planes.right = glm::vec4{
        mvp[3][0] - mvp[0][0],
        mvp[3][1] - mvp[0][1],
        mvp[3][2] - mvp[0][2],
        mvp[3][3] - mvp[0][3],
    };

    planes.bottom = glm::vec4{
        mvp[3][0] + mvp[1][0],
        mvp[3][1] + mvp[1][1],
        mvp[3][2] + mvp[1][2],
        mvp[3][3] + mvp[1][3],
    };

    planes.top = glm::vec4{
        mvp[3][0] - mvp[1][0],
        mvp[3][1] - mvp[1][1],
        mvp[3][2] - mvp[1][2],
        mvp[3][3] - mvp[1][3],
    };

    planes.near = glm::vec4{
        mvp[3][0] + mvp[2][0],
        mvp[3][1] + mvp[2][1],
        mvp[3][2] + mvp[2][2],
        mvp[3][3] + mvp[2][3],
    };

    planes.far = glm::vec4{
        mvp[3][0] - mvp[2][0],
        mvp[3][1] - mvp[2][1],
        mvp[3][2] - mvp[2][2],
        mvp[3][3] - mvp[2][3],
    };

    if (normalize) {
        planes.left   = glm::normalize(planes.left);
        planes.right  = glm::normalize(planes.right);
        planes.bottom = glm::normalize(planes.bottom);
        planes.top    = glm::normalize(planes.top);
        planes.near   = glm::normalize(planes.near);
        planes.far    = glm::normalize(planes.far);
    }

    return planes;
}

Frustum ProjectionToFrustum(glm::mat4 const& proj) {
    float near          = proj[2][3] / (proj[2][2] - 1);
    float far           = proj[2][3] / (proj[2][2] + 1);
    glm::vec2 unproject = near / glm::vec2{proj[0][0], proj[1][1]};
    return Frustum{
        /*left*/ (proj[0][2] - 1) * unproject.x,
        /*right*/ (proj[0][2] + 1) * unproject.x,
        /*bottom*/ (proj[1][2] - 1) * unproject.y,
        /*top*/ (proj[1][2] + 1) * unproject.y,
        /*near*/ near,
        /*far*/ far,
    };
}

} // namespace engine
