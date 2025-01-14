project(XenDX)

add_library(Xen STATIC
        # Core Engine Components
        ${ENGINE}/Camera.cpp
        ${ENGINE}/Camera.hpp
        ${ENGINE}/ComponentManager.hpp
        ${ENGINE}/EntityId.hpp
        ${ENGINE}/GameState.hpp
        ${ENGINE}/Scene.cpp
        ${ENGINE}/Scene.hpp
        ${ENGINE}/Resource.hpp
        ${ENGINE}/TransformComponent.hpp
        ${ENGINE}/TransformComponent.cpp
        # DirectX 11 Abstractions
        ${ENGINE}/DX11/DxGraphicsDevice.cpp
        ${ENGINE}/DX11/DxGraphicsDevice.hpp
        ${ENGINE}/DX11/DxBuffer.cpp
        ${ENGINE}/DX11/DxBuffer.hpp
        ${ENGINE}/DX11/DxShader.cpp
        ${ENGINE}/DX11/DxShader.hpp
)

target_link_libraries(Xen PRIVATE
        # Windows/DirectX libraries
        d3d11.lib
        dxgi.lib
        d3dcompiler.lib
        dxguid.lib
)