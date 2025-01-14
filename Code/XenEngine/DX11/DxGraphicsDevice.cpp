// Author: Jake Rieger
// Created: 1/14/2025.
//

#include "DxGraphicsDevice.hpp"

#include "Panic.inl"

namespace x::dx {
    DxGraphicsDevice::DxGraphicsDevice() {
        Initialize();
    }

    DxGraphicsDevice::~DxGraphicsDevice() {
        // Report live objects when in debug mode
        if (kEnableDebugLayer && _debugDevice) {
            std::ignore = _debugDevice->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
        }
    }

    shared_ptr<DxBuffer> DxGraphicsDevice::CreateBuffer(const BufferDescription& desc) {
        D3D11_BUFFER_DESC bd;
        bd.ByteWidth           = desc.sizeInBytes;
        bd.Usage               = desc.usage;
        bd.BindFlags           = desc.bindFlags;
        bd.CPUAccessFlags      = desc.cpuAccessFlags;
        bd.MiscFlags           = desc.miscFlags;
        bd.StructureByteStride = desc.structureByteStride;

        D3D11_SUBRESOURCE_DATA initData      = {};
        D3D11_SUBRESOURCE_DATA* pInitialData = None;

        if (desc.initialData) {
            initData.pSysMem = desc.initialData;
            pInitialData     = &initData;
        }

        ComPtr<ID3D11Buffer> buffer;
        const HRESULT hr = _device->CreateBuffer(&bd, pInitialData, &buffer);
        if (FAILED(hr)) { Panic("Failed to create buffer"); }

        return make_shared<DxBuffer>(buffer, *this);
    }

    shared_ptr<DxBuffer>
    DxGraphicsDevice::CreateVertexBuffer(const void* data, u32 sizeInBytes, bool dynamic) {
        BufferDescription desc;
        desc.sizeInBytes    = sizeInBytes;
        desc.usage          = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
        desc.bindFlags      = D3D11_BIND_VERTEX_BUFFER;
        desc.cpuAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
        desc.initialData    = data;

        return CreateBuffer(desc);
    }

    shared_ptr<DxBuffer>
    DxGraphicsDevice::CreateIndexBuffer(const void* data, u32 sizeInBytes, bool dynamic) {
        BufferDescription desc;
        desc.sizeInBytes    = sizeInBytes;
        desc.usage          = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
        desc.bindFlags      = D3D11_BIND_INDEX_BUFFER;
        desc.cpuAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
        desc.initialData    = data;

        return CreateBuffer(desc);
    }

    void DxGraphicsDevice::Initialize() {
        CreateDeviceAndContext();
        if (kEnableDebugLayer) { SetupDebugLayer(); }
    }

    void DxGraphicsDevice::CreateDeviceAndContext() {
        D3D_FEATURE_LEVEL featureLevels[] = {
          D3D_FEATURE_LEVEL_11_1,
          D3D_FEATURE_LEVEL_11_0,
        };

        u32 createDeviceFlags = 0;
        if (kEnableDebugLayer) { createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG; }

        ComPtr<ID3D11Device> tempDevice;
        ComPtr<ID3D11DeviceContext> tempContext;
        D3D_FEATURE_LEVEL actualFeatureLevel;

        const HRESULT hr = D3D11CreateDevice(None,
                                             D3D_DRIVER_TYPE_HARDWARE,
                                             None,
                                             createDeviceFlags,
                                             featureLevels,
                                             _countof(featureLevels),
                                             D3D11_SDK_VERSION,
                                             &tempDevice,
                                             &actualFeatureLevel,
                                             &tempContext);
        if (FAILED(hr)) { Panic("Failed to create D3D11 device"); }

        _device           = tempDevice;
        _immediateContext = tempContext;
    }

    void DxGraphicsDevice::SetupDebugLayer() {
        if (FAILED(_device.As(&_debugDevice))) { Panic("Failed to create debug layer"); }
    }
}  // namespace x::dx