#pragma once
#include "pch.h"

class D3D11ConstantBuffer;
class D3D11VertexBuffer;
class D3D11HDShader {
public:
    static std::map<UINT8, D3D11HDShader*> ShadersByID;

    D3D11HDShader();
    ~D3D11HDShader();

    /** Loads shader */
    XRESULT LoadShader( const wchar_t* hullShader, const wchar_t* domainShader );

    /** Applys the shader */
    XRESULT Apply();

    /** Unbinds the currently bound hull/domain shaders */
    static void Unbind();

    /** Returns a reference to the constantBuffer vector*/
    std::vector<D3D11ConstantBuffer*>& GetConstantBuffer();

    /** Returns the shader */
    Microsoft::WRL::ComPtr<ID3D11HullShader> GetHShader() { return HullShader.Get(); }

    /** Returns the shader */
    Microsoft::WRL::ComPtr<ID3D11DomainShader> GetDShader() { return DomainShader.Get(); }

    /** Returns this textures ID */
    UINT16 GetID() { return ID; };
private:
    Microsoft::WRL::ComPtr<ID3D11HullShader> HullShader;
    Microsoft::WRL::ComPtr<ID3D11DomainShader> DomainShader;
    std::vector<D3D11ConstantBuffer*> ConstantBuffers;

    std::wstring File;

    /** ID of this shader */
    UINT16 ID;
};

