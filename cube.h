#pragma once

#include <glm/glm.hpp>
#include "object.h"
#include "material.h"
#include "intersect.h"

class Cube : public Object {
public:

    std::string key;
    glm::vec2 tSize;

    Cube(const glm::vec3& center, float sideLength, const Material& mat)
            : center(center), sideLength(sideLength), Object(mat) {}

    Intersect rayIntersect(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) const {
        glm::vec3 halfExtents = glm::vec3(sideLength) * 0.5f;
        glm::vec3 minBounds = center - halfExtents;
        glm::vec3 maxBounds = center + halfExtents;

        glm::vec3 tMin = (minBounds - rayOrigin) / rayDirection;
        glm::vec3 tMax = (maxBounds - rayOrigin) / rayDirection;

        glm::vec3 t1 = glm::min(tMin, tMax);
        glm::vec3 t2 = glm::max(tMin, tMax);

        float tNear = glm::max(glm::max(t1.x, t1.y), t1.z);
        float tFar = glm::min(glm::min(t2.x, t2.y), t2.z);

        if (tNear > tFar || tFar < 0.0f) {
            return Intersect{false};
        }

        float dist = (tNear < 0.0f) ? tFar : tNear;

        glm::vec3 point = rayOrigin + dist * rayDirection;

        glm::vec3 normal(0.0f);
        float epsilon = 0.001f;

        if (fabs(point.x - minBounds.x) < epsilon) normal.x = -1.0f;
        if (fabs(point.x - maxBounds.x) < epsilon) normal.x = 1.0f;
        if (fabs(point.y - minBounds.y) < epsilon) normal.y = -1.0f;
        if (fabs(point.y - maxBounds.y) < epsilon) normal.y = 1.0f;
        if (fabs(point.z - minBounds.z) < epsilon) normal.z = -1.0f;
        if (fabs(point.z - maxBounds.z) < epsilon) normal.z = 1.0f;

        if (glm::dot(rayDirection, normal) > 0) {
            normal = -normal;
        }

        glm::vec3 hitVector = point - minBounds;
        glm::vec2 texCoord(0.0f);

        if (normal.x != 0) {
            texCoord.x = hitVector.z / sideLength;
            texCoord.y = hitVector.y / sideLength;
        }
        else if (normal.y != 0) {
            texCoord.x = hitVector.x / sideLength;
            texCoord.y = hitVector.z / sideLength;
        }
        else if (normal.z != 0)
        {
            texCoord.x = hitVector.x / sideLength;
            texCoord.y = hitVector.y / sideLength;
        }

        if (normal.x < 0 || normal.y < 0 || normal.z < 0)
        {
            texCoord = glm::vec2(1.0f) - texCoord;
        }

        return Intersect{true, dist, point, glm::normalize(normal), texCoord};
    }

private:
    glm::vec3 center;
    float sideLength;
};