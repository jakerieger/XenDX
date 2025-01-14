// Author: Jake Rieger
// Created: 1/14/2025.
//

#pragma once

#include "Types.hpp"
#include <d3d11.h>
#include <wrl/client.h>

namespace x::dx {
    using Microsoft::WRL::ComPtr;

    // Forward declaration
    class DxGraphicsDevice;

    enum class ShaderStages : u32 {
        None    = 0,
        Vertex  = 1 << 0,
        Pixel   = 1 << 1,
        Compute = 1 << 2,
    };

    inline ShaderStages operator|(ShaderStages a, ShaderStages b) {
        return CAST<ShaderStages>(CAST<u32>(a) | CAST<u32>(b));
    }

    inline ShaderStages operator&(ShaderStages a, ShaderStages b) {
        return CAST<ShaderStages>(CAST<u32>(a) & CAST<u32>(b));
    }

    inline bool HasStage(ShaderStages stages, ShaderStages stage) {
        return (CAST<u32>(stages) & CAST<u32>(stage)) != 0;
    }

    class DxBuffer {
    private:
        ComPtr<ID3D11Buffer> _buffer;
        DxGraphicsDevice& _device;
        D3D11_BUFFER_DESC _description;
        bool _dynamic;

    public:
        DxBuffer(const ComPtr<ID3D11Buffer>& buffer, DxGraphicsDevice& device);

        DxBuffer(const DxBuffer&)            = delete;
        DxBuffer& operator=(const DxBuffer&) = delete;

        void Update(const void* data, size_t sizeInBytes) const;
        void BindAsVertexBuffer(u32 slot, u32 stride, u32 offset = 0) const;
        void BindAsIndexBuffer(DXGI_FORMAT format, u32 offset = 0) const;
        void BindAsConstantBuffer(u32 slot, ShaderStages stages) const;

        u32 GetSize() const {
            return _description.ByteWidth;
        }
        bool IsDynamic() const {
            return _dynamic;
        }
        ID3D11Buffer* GetRawBuffer() const {
            return _buffer.Get();
        }
        bool SupportsBinding(D3D11_BIND_FLAG bind) const {
            return (_description.BindFlags & bind) != 0;
        }

    private:
        void UpdateDynamic(const void* data, size_t sizeInBytes) const;
        void UpdateDefault(const void* data) const;
    };
}  // namespace x::dx
