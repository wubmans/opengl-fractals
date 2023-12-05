//------------------------------------------------------------------------------
// Ryan Schmitt & Ian Stewart
// 9/30/08
// 
//
// Description:
// Basic Stationary Camera Class
//------------------------------------------------------------------------------

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <glm/glm.hpp>

class Camera
{
   public:
      glm::vec3 loc;
      glm::vec3 dir;
      glm::vec3 up;
      float fov;
      float zNear, zFar;
      // AABoundingBox bounds;

      Camera(float setFOV, float setX, float setY, float setZ,
             float dx,     float dy,   float dz,
             float upx,    float upy,  float upz,
             float setzNear, float setzFar)
      {
         loc = glm::vec3(setX, setY, setZ);
         dir = glm::vec3(dx, dy, dz);
         up = glm::vec3(upx, upy, upz);
         fov = setFOV;
         zNear = setzNear;
         zFar = setzFar;
         // bounds = AABoundingBox(&loc, 2.f, 2.f, 2.f ); //Set the size of the camera for collisions
      }
};

#endif
