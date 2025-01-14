#include "DX11/DxGraphicsDevice.hpp"
#include "DX11/DxBuffer.hpp"

int main() {
    using namespace x;

    dx::DxGraphicsDevice device;
    auto buff =
      device.CreateBuffer({64, D3D11_USAGE_DEFAULT, D3D11_BIND_CONSTANT_BUFFER, 0, 0, 0, None});
    buff->BindAsConstantBuffer(0, dx::ShaderStages::Vertex | dx::ShaderStages::Pixel);

    return 0;
}