// Author: Jake Rieger
// Created: 1/13/2025.
//

#include "Camera.hpp"

namespace x {
    using namespace DirectX;

    Camera::Camera() {
        _position    = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        _forward     = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
        _up          = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        _right       = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
        _fovY        = XM_PIDIV4;  // 45 degrees in radians
        _aspectRatio = 16.0f / 9.0f;
        _zNear       = 0.1f;
        _zFar        = 1000.0f;

        UpdateViewMatrix();
        UpdateProjectionMatrix();
    }

    void Camera::SetPosition(const DirectX::XMVECTOR& position) {
        _position = position;
        UpdateViewMatrix();
    }

    void Camera::MoveForward(f32 distance) {
        _position = XMVectorAdd(_position, XMVectorScale(_forward, distance));
        UpdateViewMatrix();
    }

    void Camera::MoveRight(f32 distance) {
        _position = XMVectorAdd(_position, XMVectorScale(_right, distance));
        UpdateViewMatrix();
    }

    void Camera::MoveUp(f32 distance) {
        _position = XMVectorAdd(_position, XMVectorScale(_up, distance));
        UpdateViewMatrix();
    }

    void Camera::Rotate(f32 pitch, f32 yaw, f32 roll) {
        const XMMATRIX rotation = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

        _forward = XMVector3TransformNormal(_forward, rotation);
        _up      = XMVector3TransformNormal(_up, rotation);
        _right   = XMVector3TransformNormal(_right, rotation);

        _forward = XMVector3Normalize(_forward);
        _right   = XMVector3Cross(_up, _forward);
        _up      = XMVector3Cross(_forward, _right);

        UpdateViewMatrix();
    }

    void Camera::LookAt(const DirectX::XMVECTOR& target) {
        _forward = XMVector3Normalize(XMVectorSubtract(target, _position));
        _right = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), _forward));
        _up    = XMVector3Cross(_forward, _right);

        UpdateViewMatrix();
    }

    void Camera::SetFOV(f32 fovY) {
        _fovY = fovY;
    }

    void Camera::SetAspectRatio(f32 ratio) {
        _aspectRatio = ratio;
    }

    void Camera::SetClipPlanes(f32 near, f32 far) {
        _zNear = near;
        _zFar  = far;
    }

    DirectX::XMMATRIX Camera::GetViewMatrix() const {
        return _viewMatrix;
    }

    DirectX::XMMATRIX Camera::GetProjectionMatrix() const {
        return _projectionMatrix;
    }

    DirectX::XMMATRIX Camera::GetViewProjectionMatrix() const {
        return XMMatrixMultiply(_viewMatrix, _projectionMatrix);
    }

    void Camera::UpdateViewMatrix() {
        _viewMatrix = XMMatrixLookToLH(_position, _forward, _up);
    }

    void Camera::UpdateProjectionMatrix() {
        _projectionMatrix = XMMatrixPerspectiveFovLH(_fovY, _aspectRatio, _zNear, _zFar);
    }
}  // namespace x