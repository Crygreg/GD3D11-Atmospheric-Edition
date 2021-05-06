#pragma once
#include "pch.h"

class D3D11ConstantBuffer;
class D3D11VertexBuffer;

class D3D11GShader {
public:
    D3D11GShader();
    ~D3D11GShader();


    /** Loads shader */
    XRESULT LoadShader( const wchar_t* geometryShader, const std::vector<D3D_SHADER_MACRO>& makros = std::vector<D3D_SHADER_MACRO>(), bool createStreamOutFromVS = false, int soLayout = 0 );

    /** Applys the shader */
    XRESULT Apply();

    /** Returns a reference to the constantBuffer vector*/
    std::vector<D3D11ConstantBuffer*>& GetConstantBuffer();

    /** Returns the shader */
    Microsoft::WRL::ComPtr<ID3D11GeometryShader> GetShader() { return GeometryShader.Get(); }

    /** Returns this textures ID */
    UINT16 GetID() { return ID; };

private:
    Microsoft::WRL::ComPtr<ID3D11GeometryShader> GeometryShader;
    std::vector<D3D11ConstantBuffer*> ConstantBuffers;

    std::wstring File;

    /** ID of this shader */
    UINT16 ID;
};

