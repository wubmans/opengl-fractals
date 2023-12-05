//------------------------------------------------------------------------------
// Ryan Schmitt & Ian Stewart
// 9/30/08
// 
//
// Description:
// Camera capable of moving around a scene.
//------------------------------------------------------------------------------

#ifndef __FLYING_CAMERA_H__
#define __FLYING_CAMERA_H__

#include "camera.h"
#include <math.h>
#include <algorithm>

#define PI 3.14159265

class Camera
{
   private:
     

   public:
      float pitch = 0.0f;
      float yaw = 0.0f;
      int xFactor = 1;
      int yFactor = 1;
      int zFactor = 1;

      glm::vec3 loc;
      glm::vec3 dir;
      glm::vec3 up;
      float fov = 90.0f;
      float zNear = 0.1f; 
      float zFar = 1000.0f;

      Camera(glm::vec3 loc, glm::vec3 dir, glm::vec3 up)
      {
         this->loc = loc;
         this->dir = dir;
         this->up = up;
         yaw = atan2f(dir.z, dir.x) * 180 / PI;
         pitch = atan2f(dir.y, sqrtf((dir.x * dir.x) + (dir.z * dir.z))) * 180 / PI;
      }

      //------------------------------------------------------------------------
      // Allow movement in the x, y, and z axes?

      void allowX(bool val)
      {
         xFactor = val?1:0;
      }

      void allowY(bool val)
      {
         yFactor = val?1:0;
      }

      void allowZ(bool val)
      {
         zFactor = val?1:0;
      }

      //------------------------------------------------------------------------
      // Movement Functions

      void moveForward(float delta)
      {
         loc.x += delta*dir.x;
         loc.y += delta*dir.y;
         loc.z += delta*dir.z;
      }

      void moveBackward(float delta)
      {
         loc.x -= delta*dir.x;
         loc.y -= delta*dir.y;
         loc.z -= delta*dir.z;
      }


      void moveLeft(float delta)
      {
         glm::vec3 v = glm::cross(dir, up);
         loc.x -= delta*v.x;
         loc.y -= delta*v.y;
         loc.z -= delta*v.z;
      }

      void moveRight(float delta)
      {
         glm::vec3 v = glm::cross(dir, up);
         loc.x += delta*v.x;
         loc.y += delta*v.y;
         loc.z += delta*v.z;
      }

      //------------------------------------------------------------------------
      // Adjust the pitch and yaw

      void setPitch(float delta)
      {
         //Clamp to 90 and -90
         pitch = std::max(-89.f, std::min(89.f, pitch+delta));
         updateDir();
      }

      void setYaw(float delta) {
         yaw += delta;
         updateDir();
      }

   protected:

      //------------------------------------------------------------------------
      // Update the forward direction based on pitch and yaw.

      void updateDir()
      {
         dir.x = cos(yaw*PI/180)*cos(pitch*PI/180);
         dir.y = sin(pitch*PI/180);
         dir.z = sin((yaw)*PI/180)*cos(pitch*PI/180);
      }

};

#endif
