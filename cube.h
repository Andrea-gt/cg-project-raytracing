#pragma once

#include <glm/glm.hpp>
#include "object.h"
#include "material.h"
#include "intersect.h"
#include <string>

class Cube : public Object {
public:

    Cube(const glm::vec3& center, float sideLength, const Material& mat)
            : center(center), sideLength(sideLength), Object(mat) {}

    Intersect rayIntersect(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) const {
        glm::vec3 halfExtents = glm::vec3(sideLength * 0.5f);
        glm::vec3 minPoint = center - halfExtents;
        glm::vec3 maxPoint = center + halfExtents;

        glm::vec3 tMin = (minPoint - rayOrigin) / rayDirection;
        glm::vec3 tMax = (maxPoint - rayOrigin) / rayDirection; //https://gist.github.com/DomNomNom/46bb1ce47f68d255fd5d


        glm::vec3 t1 = glm::min(tMin, tMax);
        glm::vec3 t2 = glm::max(tMin, tMax);


        float tNear = glm::max(glm::max(t1.x, t1.y), t1.z);
        float tFar = glm::min(glm::min(t2.x, t2.y), t2.z);

        if (tNear > tFar || tFar < 0) {
            return Intersect{false};
        }

        glm::vec3 hitPoint;
        glm::vec3 normal;

        if (tNear > 0) {
            hitPoint = rayOrigin + tNear * rayDirection;
            normal = calculateNormal(hitPoint, rayDirection, maxPoint, minPoint);
        } else {
            hitPoint = rayOrigin + tFar * rayDirection;
            normal = calculateNormal(hitPoint, rayDirection, maxPoint, minPoint);
        }


        int maxAxis = 0;
        for (int i = 1; i < 3; ++i) {
            if (fabs(normal[i]) > fabs(normal[maxAxis])) {
                maxAxis = i; // Find the axis of the largest normal component
            }
        }

        glm::vec2 texCoords;
        switch (maxAxis) {
            case 0: // x-axis
                texCoords = glm::vec2(hitPoint.y - minPoint.y, hitPoint.z - minPoint.z) / sideLength;
                break;
            case 1: // y-axis
                texCoords = glm::vec2(hitPoint.x - minPoint.x, hitPoint.z - minPoint.z) / sideLength;
                break;
            case 2: // z-axis
                texCoords = glm::vec2(hitPoint.x - minPoint.x, hitPoint.y - minPoint.y) / sideLength;
                break;
        }

        return Intersect{true, tNear, hitPoint, normal, texCoords};
    }

private:
    glm::vec3 center;
    float sideLength;

    glm::vec3 calculateNormal(const glm::vec3 &hitPoint, glm::vec3 rayDirection, glm::vec3 tMax, glm::vec3 tMin) const {
        //https://www.rose-hulman.edu/class/csse/csse451/AABB/
        float epsilon = 0.001f;

        // Initialize normal vector
        glm::vec3 normal = glm::vec3(0.0f);

        // Check each axis separately to determine which face the hitPoint is on
        for (int i = 0; i < 3; ++i) {
            if (fabs(hitPoint[i]- tMin[i]) < epsilon) {
                normal[i] = -1.0f; // Set the normal component to -1 for this axis
            }

            if (fabs(hitPoint[i]- tMax[i]) < epsilon) {
                normal[i] = 1.0f; // Set the normal component to 1 for this axis
            }
        }

        if (glm::dot(rayDirection, normal) > 0) {
            normal = -normal; // Reverse the normal direction if not aligned with the ray
        };

        return glm::normalize(normal); // If the hit point isn't exactly on any face, return zero vector or handle it as needed
    }
};
