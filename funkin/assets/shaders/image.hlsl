// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

cbuffer OrthoBuffer : register(b0) {
    float4x4 projection;
};

Texture2D    tex  : register(t0);
SamplerState samp : register(s0);

struct VSInput {
    float2 position : POSITION;
    float2 uv       : TEXCOORD;
    float4 color    : COLOR;
};

struct PSInput {
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD;
    float4 color    : COLOR;
};

PSInput VSMain(VSInput input) {
    PSInput output;
    output.position = mul(projection, float4(input.position, 0.0f, 1.0f));
    output.uv       = input.uv;
    output.color    = input.color;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET {
    float4 texColor = tex.Sample(samp, input.uv);
    return texColor * input.color;
}
