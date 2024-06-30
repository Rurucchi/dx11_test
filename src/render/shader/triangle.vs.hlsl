// these names must match D3D11_INPUT_ELEMENT_DESC array
struct VS_INPUT {
	float2 pos   : POSITION;
	float2 uv    : TEXCOORD;
	float3 color : COLOR;
};

// b0 = constant buffer bound to slot 0
cbuffer cbuffer0 : register(b0) {
	float4x4 uTransform;
}