project(XenDX)

add_library(Xen STATIC
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
)

target_link_libraries(Xen PRIVATE
        # Windows/DirectX libraries
        d3d11.lib
        dxgi.lib
        d3dcompiler.lib
        dxguid.lib
)