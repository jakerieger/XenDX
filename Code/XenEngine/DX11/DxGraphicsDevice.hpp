// Author: Jake Rieger
// Created: 1/14/2025.
//

#pragma once

#include "Types.hpp"
#include "DxBuffer.hpp"
#include <d3d11.h>
#include <wrl/client.h>

namespace x::dx {
    using Microsoft::WRL::ComPtr;

#ifdef NDEBUG
    constexpr bool kEnableDebugLayer = false;
#else
    constexpr bool kEnableDebugLayer = true;
#endif

    class DxBuffer;
    class DxTexture;
    class DxShader;

    struct BufferDescription {
        u32 sizeInBytes         = 0;
        D3D11_USAGE usage       = D3D11_USAGE_DEFAULT;
        u32 bindFlags           = 0;
        u32 cpuAccessFlags      = 0;
        u32 miscFlags           = 0;
        u32 structureByteStride = 0;
        const void* initialData = None;
    };

    class DxGraphicsDevice {
    private:
        ComPtr<ID3D11Device> _device;
        ComPtr<ID3D11DeviceContext> _immediateContext;
        ComPtr<ID3D11Debug> _debugDevice;

    public:
        DxGraphicsDevice();
        ~DxGraphicsDevice();

        // Remove copy
        DxGraphicsDevice(const DxGraphicsDevice&)            = delete;
        DxGraphicsDevice& operator=(const DxGraphicsDevice&) = delete;

        shared_ptr<DxBuffer> CreateBuffer(const BufferDescription&);
        shared_ptr<DxBuffer>
        CreateVertexBuffer(const void* data, u32 sizeInBytes, bool dynamic = false);
        shared_ptr<DxBuffer>
        CreateIndexBuffer(const void* data, u32 sizeInBytes, bool dynamic = false);

    private:
        void Initialize();
        void CreateDeviceAndContext();
        void SetupDebugLayer();

    public:
        ID3D11Device* GetDevice() const {
            return _device.Get();
        }
        ID3D11DeviceContext* GetImmediateContext() const {
            return _immediateContext.Get();
        }
    };
}  // namespace x::dx
