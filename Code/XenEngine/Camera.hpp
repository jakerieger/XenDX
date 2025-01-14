// Author: Jake Rieger
// Created: 1/13/2025.
//

#pragma once

#include "Types.hpp"
#include <DirectXMath.h>

namespace x {
    class Camera {
    public:
        Camera();

        void SetPosition(const DirectX::XMVECTOR& position);
        void MoveForward(f32 distance);
        void MoveRight(f32 distance);
        void MoveUp(f32 distance);
        void Rotate(f32 pitch, f32 yaw, f32 roll);
        void LookAt(const DirectX::XMVECTOR& target);
        void SetFOV(f32 fovY);
        void SetAspectRatio(f32 ratio);
        void SetClipPlanes(f32 near, f32 far);

        DirectX::XMMATRIX GetViewMatrix() const;
        DirectX::XMMATRIX GetProjectionMatrix() const;
        DirectX::XMMATRIX GetViewProjectionMatrix() const;

    private:
        DirectX::XMVECTOR _position;
        DirectX::XMVECTOR _forward;
        DirectX::XMVECTOR _up;
        DirectX::XMVECTOR _right;

        f32 _fovY;
        f32 _aspectRatio;
        f32 _zNear;
        f32 _zFar;

        DirectX::XMMATRIX _viewMatrix;
        DirectX::XMMATRIX _projectionMatrix;

        void UpdateViewMatrix();
        void UpdateProjectionMatrix();
    };
}  // namespace x
