// Light pixel shader
// Calculate diffuse lighting for a single directional light (also texturing)

Texture2D texture0 : register(t0);
Texture2D texture1 : register(t1);
Texture2D texture2 : register(t2);
SamplerState sampler0 : register(s0);


cbuffer LightBuffer : register(b0)
{
	float4 ambientColour;
	float4 diffuseColour;
	float3 lightDirection;
	float padding;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;

};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
	float intensity = saturate(dot(normal, lightDirection));
	float4 colour = saturate(diffuse * intensity);
	return colour;
}

float4 main(InputType input) : SV_TARGET
{
	float4 textureColour;
	float4 lightColour;
	float4 grassColour;
	float4 rockColour;
	float dirtColour;

	float blendAmount;

	grassColour = texture0.Sample(sampler0, input.tex);
	rockColour = texture1.Sample(sampler0, input.tex);
	dirtColour = texture2.Sample(sampler0, input.tex);
	
	float slope = 1.0f - input.normal.y;
	
	if (slope < 0.2)
	{
		blendAmount = slope / 0.2f;
		textureColour = lerp(grassColour, dirtColour, blendAmount);
	}
	else if ((slope < 0.7f) && (slope >= 0.2f))
	{
		blendAmount = (slope - 0.2f) * 2;

		textureColour = lerp(dirtColour, rockColour, blendAmount);
	}

	if (slope >= 0.7)
	{
		textureColour = rockColour;
	}

	lightColour = ambientColour + calculateLighting( -lightDirection, input.normal, diffuseColour);
	
	return lightColour * textureColour;
}


