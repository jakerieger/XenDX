#include "DX11/DxGraphicsDevice.hpp"
#include "DX11/DxBuffer.hpp"

int main() {
    using namespace x;

    dx::DxGraphicsDevice dxgDevice;
    auto buff = dxgDevice.CreateVertexBuffer(None, 64, false);
    buff->BindAsVertexBuffer(0, 0, 0);

    return 0;
}