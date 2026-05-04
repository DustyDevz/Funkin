// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "pch.hpp"
#include "dx12_gal.hpp"

namespace Funkin::Renderer::GAL {
    namespace {
        static D3D12_COMPARISON_FUNC toCompareFunc(CompareOp op) {
            switch (op) {
                case CompareOp::Never:          return D3D12_COMPARISON_FUNC_NEVER;
                case CompareOp::Less:           return D3D12_COMPARISON_FUNC_LESS;
                case CompareOp::Equal:          return D3D12_COMPARISON_FUNC_EQUAL;
                case CompareOp::LessOrEqual:    return D3D12_COMPARISON_FUNC_LESS_EQUAL;
                case CompareOp::Greater:        return D3D12_COMPARISON_FUNC_GREATER;
                case CompareOp::NotEqual:       return D3D12_COMPARISON_FUNC_NOT_EQUAL;
                case CompareOp::GreaterOrEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
                case CompareOp::Always:         return D3D12_COMPARISON_FUNC_ALWAYS;
                default:                        return D3D12_COMPARISON_FUNC_LESS;
            }
        }

        static D3D12_PRIMITIVE_TOPOLOGY toPrimTopology(PrimitiveTopology t) {
            switch (t) {
                case PrimitiveTopology::TriangleList:  return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                case PrimitiveTopology::TriangleStrip: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
                case PrimitiveTopology::LineList:      return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
                case PrimitiveTopology::LineStrip:     return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
                case PrimitiveTopology::PointList:     return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
                default:                               return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            }
        }

        static D3D12_PRIMITIVE_TOPOLOGY_TYPE toPrimTopologyType(PrimitiveTopology t) {
            switch (t) {
                case PrimitiveTopology::TriangleList:
                case PrimitiveTopology::TriangleStrip: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
                case PrimitiveTopology::LineList:
                case PrimitiveTopology::LineStrip:     return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
                case PrimitiveTopology::PointList:     return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
                default:                               return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            }
        }

        static D3D12_TEXTURE_ADDRESS_MODE toAddressMode(WrapMode w) {
            switch (w) {
                case WrapMode::Clamp:  return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                case WrapMode::Repeat: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                case WrapMode::Mirror: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
                case WrapMode::Border: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
                default:               return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            }
        }
    }

    ShaderHandle DX12Gal::createShader(const ShaderDesc& desc) {
        DX12GalShader shader = {};
        shader.bytecode.assign(desc.bytecode, desc.bytecode + desc.size);
        shader.stage = desc.stage;

        ShaderHandle h;
        h.id = m_shaders.insert(std::move(shader));
        return h;
    }

    void DX12Gal::destroyShader(ShaderHandle shader) {
        m_shaders.remove(shader.id);
    }

    ComPtr<ID3D12RootSignature> DX12Gal::buildRootSignature(const PipelineDesc& /*desc*/) {
        D3D12_DESCRIPTOR_RANGE ranges[3] = {};

        ranges[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        ranges[0].NumDescriptors                    = 8;
        ranges[0].BaseShaderRegister                = 0;
        ranges[0].RegisterSpace                     = 0;
        ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        ranges[1].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
        ranges[1].NumDescriptors                    = 8;
        ranges[1].BaseShaderRegister                = 0;
        ranges[1].RegisterSpace                     = 0;
        ranges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        ranges[2].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        ranges[2].NumDescriptors                    = 4;
        ranges[2].BaseShaderRegister                = 0;
        ranges[2].RegisterSpace                     = 0;
        ranges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        D3D12_ROOT_PARAMETER params[4] = {};

        params[0].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        params[0].DescriptorTable.NumDescriptorRanges = 1;
        params[0].DescriptorTable.pDescriptorRanges   = &ranges[0];
        params[0].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_ALL;

        params[1].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        params[1].DescriptorTable.NumDescriptorRanges = 1;
        params[1].DescriptorTable.pDescriptorRanges   = &ranges[1];
        params[1].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;

        params[2].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        params[2].DescriptorTable.NumDescriptorRanges = 1;
        params[2].DescriptorTable.pDescriptorRanges   = &ranges[2];
        params[2].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_ALL;

        params[3].ParameterType            = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        params[3].Constants.ShaderRegister = 4;
        params[3].Constants.RegisterSpace  = 0;
        params[3].Constants.Num32BitValues = 32;
        params[3].ShaderVisibility         = D3D12_SHADER_VISIBILITY_ALL;

        D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
        rsDesc.NumParameters     = 4;
        rsDesc.pParameters       = params;
        rsDesc.NumStaticSamplers = 0;
        rsDesc.pStaticSamplers   = nullptr;
        rsDesc.Flags             = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ComPtr<ID3DBlob> blob, errorBlob;
        HRESULT hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1,
                                                &blob, &errorBlob);
        if (FAILED(hr)) {
            if (errorBlob)
                throw std::runtime_error(static_cast<const char*>(errorBlob->GetBufferPointer()));
            throw std::runtime_error("Failed to serialize root signature");
        }

        ComPtr<ID3D12RootSignature> rootSig;
        if (FAILED(m_device->CreateRootSignature(0,
                                                blob->GetBufferPointer(),
                                                blob->GetBufferSize(),
                                                IID_PPV_ARGS(&rootSig))))
            throw std::runtime_error("Failed to create root signature");

        return rootSig;
    }

    PipelineHandle DX12Gal::createPipeline(const PipelineDesc& desc) {
        DX12GalPipeline pipeline = {};
        pipeline.rootSig     = buildRootSignature(desc);
        pipeline.vertexStride = desc.vertexLayout.stride;
        pipeline.topology    = toPrimTopology(desc.topology);

        auto* vs = m_shaders.get(desc.vs.id);
        auto* ps = m_shaders.get(desc.ps.id);

        std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
        inputElements.reserve(desc.vertexLayout.count);
        for (uint32_t i = 0; i < desc.vertexLayout.count; ++i) {
            const auto& attr = desc.vertexLayout.attributes[i];
            D3D12_INPUT_ELEMENT_DESC elem = {};
            elem.SemanticName         = attr.semantic;
            elem.SemanticIndex        = attr.index;
            elem.Format               = toDXGI(attr.format);
            elem.InputSlot            = 0;
            elem.AlignedByteOffset    = attr.offset;
            elem.InputSlotClass       = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            elem.InstanceDataStepRate = 0;
            inputElements.push_back(elem);
        }

        D3D12_BLEND_DESC blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        if (desc.blend != BlendMode::None) {
            auto& rt        = blendDesc.RenderTarget[0];
            rt.BlendEnable  = TRUE;
            rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
            switch (desc.blend) {
                case BlendMode::Alpha:
                    rt.SrcBlend       = D3D12_BLEND_SRC_ALPHA;
                    rt.DestBlend      = D3D12_BLEND_INV_SRC_ALPHA;
                    rt.BlendOp        = D3D12_BLEND_OP_ADD;
                    rt.SrcBlendAlpha  = D3D12_BLEND_ONE;
                    rt.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
                    break;
                case BlendMode::Additive:
                    rt.SrcBlend       = D3D12_BLEND_ONE;
                    rt.DestBlend      = D3D12_BLEND_ONE;
                    rt.BlendOp        = D3D12_BLEND_OP_ADD;
                    rt.SrcBlendAlpha  = D3D12_BLEND_ONE;
                    rt.DestBlendAlpha = D3D12_BLEND_ONE;
                    break;
                case BlendMode::Multiply:
                    rt.SrcBlend       = D3D12_BLEND_DEST_COLOR;
                    rt.DestBlend      = D3D12_BLEND_ZERO;
                    rt.BlendOp        = D3D12_BLEND_OP_ADD;
                    rt.SrcBlendAlpha  = D3D12_BLEND_ONE;
                    rt.DestBlendAlpha = D3D12_BLEND_ZERO;
                    break;
                case BlendMode::PremultipliedAlpha:
                    rt.SrcBlend       = D3D12_BLEND_ONE;
                    rt.DestBlend      = D3D12_BLEND_INV_SRC_ALPHA;
                    rt.BlendOp        = D3D12_BLEND_OP_ADD;
                    rt.SrcBlendAlpha  = D3D12_BLEND_ONE;
                    rt.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
                    break;
                default: break;
            }
        }

        D3D12_RASTERIZER_DESC rasterDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        switch (desc.raster.cullMode) {
            case CullMode::None:  rasterDesc.CullMode = D3D12_CULL_MODE_NONE;  break;
            case CullMode::Front: rasterDesc.CullMode = D3D12_CULL_MODE_FRONT; break;
            case CullMode::Back:  rasterDesc.CullMode = D3D12_CULL_MODE_BACK;  break;
        }
        rasterDesc.FillMode = (desc.raster.fillMode == FillMode::Wireframe)
                                ? D3D12_FILL_MODE_WIREFRAME
                                : D3D12_FILL_MODE_SOLID;

        D3D12_DEPTH_STENCIL_DESC dsDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        dsDesc.DepthEnable    = desc.depth.testEnabled  ? TRUE  : FALSE;
        dsDesc.DepthWriteMask = desc.depth.writeEnabled ? D3D12_DEPTH_WRITE_MASK_ALL
                                                        : D3D12_DEPTH_WRITE_MASK_ZERO;
        dsDesc.DepthFunc      = toCompareFunc(desc.depth.compare);
        dsDesc.StencilEnable  = FALSE;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature        = pipeline.rootSig.Get();
        if (vs) psoDesc.VS = { vs->bytecode.data(), vs->bytecode.size() };
        if (ps) psoDesc.PS = { ps->bytecode.data(), ps->bytecode.size() };
        psoDesc.InputLayout           = { inputElements.data(), static_cast<UINT>(inputElements.size()) };
        psoDesc.BlendState            = blendDesc;
        psoDesc.RasterizerState       = rasterDesc;
        psoDesc.DepthStencilState     = dsDesc;
        psoDesc.PrimitiveTopologyType = toPrimTopologyType(desc.topology);
        psoDesc.NumRenderTargets      = desc.renderTargetCount;
        for (uint32_t i = 0; i < desc.renderTargetCount; ++i)
            psoDesc.RTVFormats[i] = toDXGI(desc.renderTargetFmt);
        psoDesc.DSVFormat  = (desc.depthStencilFmt != PixelFormat::Unknown)
                                ? toDXGI(desc.depthStencilFmt)
                                : DXGI_FORMAT_UNKNOWN;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.SampleDesc = { 1, 0 };
        psoDesc.NodeMask   = 0;

        if (FAILED(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipeline.pso))))
            throw std::runtime_error("Failed to create graphics pipeline state");

        PipelineHandle h;
        h.id = m_pipelines.insert(std::move(pipeline));
        return h;
    }

    void DX12Gal::destroyPipeline(PipelineHandle pipeline) {
        m_pipelines.remove(pipeline.id);
    }

    SamplerHandle DX12Gal::createSampler(const SamplerDesc& desc) {
        D3D12_SAMPLER_DESC samplerDesc = {};

        switch (desc.filter) {
            case FilterMode::Nearest:     samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;   break;
            case FilterMode::Linear:      samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;  break;
            case FilterMode::Anisotropic: samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;         break;
            default:                      samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;  break;
        }

        samplerDesc.AddressU       = toAddressMode(desc.wrapU);
        samplerDesc.AddressV       = toAddressMode(desc.wrapV);
        samplerDesc.AddressW       = toAddressMode(desc.wrapW);
        samplerDesc.MaxAnisotropy  = static_cast<UINT>(desc.anisotropy);
        samplerDesc.MinLOD         = desc.minLod;
        samplerDesc.MaxLOD         = desc.maxLod;
        samplerDesc.MipLODBias     = 0.0f;
        samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        samplerDesc.BorderColor[0] = desc.borderColor.r;
        samplerDesc.BorderColor[1] = desc.borderColor.g;
        samplerDesc.BorderColor[2] = desc.borderColor.b;
        samplerDesc.BorderColor[3] = desc.borderColor.a;

        DX12GalSampler sampler = {};
        //sampler.handle = m_samplerHeap.allocateCPU();
        sampler.handle = m_samplerStaging.allocateCPU();
        m_device->CreateSampler(&samplerDesc, sampler.handle);

        SamplerHandle h;
        h.id = m_samplers.insert(std::move(sampler));
        return h;
    }

    void DX12Gal::destroySampler(SamplerHandle sampler) {
        m_samplers.remove(sampler.id);
    }
}