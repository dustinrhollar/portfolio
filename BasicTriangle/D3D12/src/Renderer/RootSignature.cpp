
struct RootSignature
{
    ID3D12RootSignature *_handle = 0;
    
    RenderError Init(UINT numParameters,
                     const D3D12_ROOT_PARAMETER1* _pParameters,
                     UINT numStaticSamplers = 0,
                     const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = nullptr,
                     D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE);
    
    RenderError Free();
    
    
};

RenderError 
RootSignature::Init(UINT                             numParameters,
                    const D3D12_ROOT_PARAMETER1     *pParameters,
                    UINT                             numStaticSamplers,
                    const D3D12_STATIC_SAMPLER_DESC *pStaticSamplers,
                    D3D12_ROOT_SIGNATURE_FLAGS       flags)
{
    RenderError result = RenderError::Success;
    
    // Create the root signature
    D3D12_FEATURE_DATA_ROOT_SIGNATURE FeatureData = {};
    FeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    
    if (FAILED(d3d_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &FeatureData, sizeof(FeatureData))))
    {
        FeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }
    
    D3D12_VERSIONED_ROOT_SIGNATURE_DESC RootSignatureDesc;
    RootSignatureDesc.Version                    = D3D_ROOT_SIGNATURE_VERSION_1_1;
    RootSignatureDesc.Desc_1_1.Flags             = flags;
    RootSignatureDesc.Desc_1_1.NumParameters     = numStaticSamplers;
    RootSignatureDesc.Desc_1_1.pParameters       = pParameters;
    RootSignatureDesc.Desc_1_1.NumStaticSamplers = numStaticSamplers;
    RootSignatureDesc.Desc_1_1.pStaticSamplers   = pStaticSamplers;
    
    ID3DBlob *Signature;
    ID3DBlob *Error;
    JMP_FAILED(D3D12SerializeVersionedRootSignature(&RootSignatureDesc, &Signature, &Error), 
               RenderError::RootSignatureError);
    JMP_FAILED(d3d_device->CreateRootSignature(0, Signature->GetBufferPointer(), Signature->GetBufferSize(),
                                               IIDE(&_handle)), 
               RenderError::RootSignatureError);
    
    if (Signature)
    {
        Signature->Release();
        Signature = NULL;
    }
    
    LBL_FAIL:;
    return result;
}

RenderError 
RootSignature::Free()
{
    RenderError result = RenderError::Success;
    D3D_RELEASE(_handle);
    return result;
}