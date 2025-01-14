// Author: Jake Rieger
// Created: 1/14/2025.
//

#pragma once

#include "Types.hpp"
#include "DxGraphicsDevice.hpp"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

namespace x::dx {
    using Microsoft::WRL::ComPtr;

    class DxShader {
    protected:
        DxGraphicsDevice& _device;
        ComPtr<ID3DBlob> _shaderBlob;
        ComPtr<ID3D11ShaderReflection> _reflection;
        ComPtr<ID3D11DeviceChild> _shader;

    public:
        explicit DxShader(DxGraphicsDevice& device) : _device(device) {}
        virtual ~DxShader() = default;

    protected:
        void InitializeFromFile(const wstr& filename, const str& entryPoint, const str& target);
        D3D11_SHADER_DESC GetShaderDesc() const;
    };

    class DxVertexShader : public DxShader {
    public:
        explicit DxVertexShader(DxGraphicsDevice& device) : DxShader(device) {}
        void LoadFromFile(const wstr& filename, const str& entryPoint = "main");
    };

    class DxPixelShader : public DxShader {};

    class DxComputeShader : public DxShader {};
}  // namespace x::dx
