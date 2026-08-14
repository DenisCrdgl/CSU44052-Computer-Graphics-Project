#include "renders/frustum/frustum.h"

Frustum::Frustum(const glm::mat4& mtrx){
    planes[0] = 
        glm::vec4(
            mtrx[0][3] + mtrx[0][0], 
            mtrx[1][3] + mtrx[1][0], 
            mtrx[2][3] + mtrx[2][0], 
            mtrx[3][3] + mtrx[3][0]
        );

    planes[1] = 
        glm::vec4(
            mtrx[0][3] - mtrx[0][0], 
            mtrx[1][3] - mtrx[1][0], 
            mtrx[2][3] - mtrx[2][0], 
            mtrx[3][3] - mtrx[3][0]
        );

    planes[2] = 
        glm::vec4(
            mtrx[0][3] + mtrx[0][1], 
            mtrx[1][3] + mtrx[1][1], 
            mtrx[2][3] + mtrx[2][1], 
            mtrx[3][3] + mtrx[3][1]
        );

    planes[3] = 
        glm::vec4(
            mtrx[0][3] - mtrx[0][1], 
            mtrx[1][3] - mtrx[1][1], 
            mtrx[2][3] - mtrx[2][1], 
            mtrx[3][3] - mtrx[3][1]
        );

    planes[4] = 
        glm::vec4(
            mtrx[0][3] + mtrx[0][2], 
            mtrx[1][3] + mtrx[1][2], 
            mtrx[2][3] + mtrx[2][2], 
            mtrx[3][3] + mtrx[3][2]
        );

    planes[5] = 
        glm::vec4(
            mtrx[0][3] - mtrx[0][2], 
            mtrx[1][3] - mtrx[1][2], 
            mtrx[2][3] - mtrx[2][2], 
            mtrx[3][3] - mtrx[3][2]
        );

    for(auto& plane : planes){
        float length = glm::length(glm::vec3(plane));
        if(length > 0.0f){
            plane /= length;
        }
    }
}

bool Frustum::isIntersection(const glm::vec3& min, const glm::vec3& max) const{
    for(auto& plane : planes){
        float posX;
        if(plane.x >= 0.0f){
            posX = max.x;
        }
        else{
            posX = min.x;
        }

        float posY;
        if(plane.y >= 0.0f){
            posY = max.y;
        }
        else{
            posY = min.y;
        }

        float posZ;
        if(plane.z >= 0.0f){
            posZ = max.z;
        }
        else{
            posZ = min.z;
        }

        glm::vec3 posVert(posX, posY, posZ);

        if(glm::dot(glm::vec3(plane), posVert) + plane.w < 0.0f){
            return false;
        }
    }

    return true;
}