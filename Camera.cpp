#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        //TODO
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraUpDirection = cameraUp;  //glm::vec3(0.0f, 1.0f, 0.0f);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));
    }


    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        //TODO
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
        //return glm::mat4();
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        //TODO

        glm::vec3 newCamPos = glm::vec3(cameraPosition);
        switch (direction) {

        case MOVE_FORWARD:
            newCamPos += cameraFrontDirection * speed;
            break;

        case MOVE_BACKWARD:
            newCamPos -= cameraFrontDirection * speed;
            break;

        case MOVE_RIGHT:
            newCamPos += cameraRightDirection * speed;
            break;

        case MOVE_LEFT:
            newCamPos -= cameraRightDirection * speed;
            break;
        }

        if ((newCamPos.y > 1.0) && (newCamPos.z > -75.0f) && (newCamPos.z < 75.0f) && (newCamPos.x > -75.0f) && (newCamPos.x < 75.0f))
            cameraPosition = newCamPos;

    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
        glm::vec3 rotate;
        rotate.x = glm::cos(glm::radians(pitch)) * glm::cos(glm::radians(yaw));
        rotate.y = glm::sin(glm::radians(pitch));//glm::cos(glm::radians(yaw));// * glm::sin(glm::radians(yaw));
        rotate.z = glm::cos(glm::radians(pitch)) * glm::sin(glm::radians(yaw));
        cameraFrontDirection = glm::normalize(rotate);
        cameraRightDirection = glm::normalize(glm::cross(rotate, glm::vec3(0.0f, 1.0f, 0.0f)));
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));

    }

    glm::vec3 Camera::getPos()
    {
        return this->cameraPosition;
    }
}