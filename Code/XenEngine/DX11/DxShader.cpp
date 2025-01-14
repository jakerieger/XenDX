// Author: Jake Rieger
// Created: 1/14/2025.
//

#include "DxShader.hpp"
#include "Panic.inl"

namespace x::dx {
    void
    DxShader::InitializeFromFile(const wstr& filename, const str& entryPoint, const str& target) {
        ComPtr<ID3DBlob> errorBlob;
        HRESULT hr = D3DCompileFromFile(filename.c_str(),
                                        None,
                                        D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                        entryPoint.c_str(),
                                        target.c_str(),
                                        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
                                        0,
                                        &_shaderBlob,
                                        &errorBlob);
        if (FAILED(hr)) {
            str errorMessage = "Failed to compile shader: ";
            if (errorBlob) errorMessage += CAST<cstr>(errorBlob->GetBufferPointer());
            Panic(errorMessage.c_str());
        }
        hr = D3DReflect(_shaderBlob->GetBufferPointer(),
                        _shaderBlob->GetBufferSize(),
                        IID_ID3D11ShaderReflection,
                        &_reflection);
        if (FAILED(hr)) { Panic("Failed to retrieve shader reflection"); }
    }

    D3D11_SHADER_DESC DxShader::GetShaderDesc() const {
        D3D11_SHADER_DESC desc {};
        std::ignore = _reflection->GetDesc(&desc);
        return desc;
    }
}  // namespace x::dx