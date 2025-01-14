// Author: Jake Rieger
// Created: 1/14/2025.
//

#include "DxBuffer.hpp"
#include "DxGraphicsDevice.hpp"
#include "Panic.inl"

namespace x::dx {
    DxBuffer::DxBuffer(const ComPtr<ID3D11Buffer>& buffer, DxGraphicsDevice& device)
        : _buffer(buffer), _device(device) {
        _buffer->GetDesc(&_description);
        _dynamic = _description.Usage == D3D11_USAGE_DYNAMIC;
    }

    void DxBuffer::Update(const void* data, size_t sizeInBytes) const {
        if (sizeInBytes > _description.ByteWidth) { Panic("Update size exceeds buffer size."); }
        if (_dynamic) {
            UpdateDynamic(data, sizeInBytes);
        } else {
            UpdateDefault(data);
        }
    }

    void DxBuffer::BindAsVertexBuffer(u32 slot, u32 stride, u32 offset) const {
        if ((_description.BindFlags & D3D11_BIND_VERTEX_BUFFER) == 0) {
            Panic("Buffer description has incorrect BindFlags for vertex buffer.");
        }
        ID3D11Buffer* buffer = _buffer.Get();
        _device.GetImmediateContext()->IASetVertexBuffers(slot, 1, &buffer, &stride, &offset);
    }

    void DxBuffer::BindAsIndexBuffer(DXGI_FORMAT format, u32 offset) const {
        if ((_description.BindFlags & D3D11_BIND_INDEX_BUFFER) == 0) {
            Panic("Buffer description has incorrect BindFlags for index buffer.");
        }
        _device.GetImmediateContext()->IASetIndexBuffer(_buffer.Get(), format, offset);
    }

    void DxBuffer::BindAsConstantBuffer(u32 slot, ShaderStages stages) const {
        if ((_description.BindFlags & D3D11_BIND_CONSTANT_BUFFER) == 0) {
            Panic("Buffer description has incorrect BindFlags for constant buffer.");
        }
        ID3D11Buffer* buffer = _buffer.Get();
        if (HasStage(stages, ShaderStages::Vertex)) {
            _device.GetImmediateContext()->VSSetConstantBuffers(slot, 1, &buffer);
        }
        if (HasStage(stages, ShaderStages::Pixel)) {
            _device.GetImmediateContext()->PSSetConstantBuffers(slot, 1, &buffer);
        }
        if (HasStage(stages, ShaderStages::Compute)) {
            _device.GetImmediateContext()->CSSetConstantBuffers(slot, 1, &buffer);
        }
    }

    void DxBuffer::UpdateDynamic(const void* data, size_t sizeInBytes) const {
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        const HRESULT hr = _device.GetImmediateContext()->Map(_buffer.Get(),
                                                              0,
                                                              D3D11_MAP_WRITE_DISCARD,
                                                              0,
                                                              &mappedResource);
        if (FAILED(hr)) { Panic("Failed to map buffer resource."); }
        memcpy(mappedResource.pData, data, sizeInBytes);
        _device.GetImmediateContext()->Unmap(_buffer.Get(), 0);
    }

    void DxBuffer::UpdateDefault(const void* data) const {
        _device.GetImmediateContext()->UpdateSubresource(_buffer.Get(), 0, None, data, 0, 0);
    }
}  // namespace x::dx