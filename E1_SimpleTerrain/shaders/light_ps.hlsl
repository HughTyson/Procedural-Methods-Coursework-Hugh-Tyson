// Light pixel shader
// Calculate diffuse lighting for a single directional light (also texturing)

Texture2D texture0 : register(t0);
Texture2D texture1 : register(t1);
Texture2D texture2 : register(t2);
Texture2D texture3 : register(t3);
SamplerState sampler0 : register(s0);

cbuffer LightBuffer : register(b0)
{
	float4 ambientColour;
	float4 diffuseColour;
	float3 lightDirection;
	float padding;
};

cbuffer WaterBuffer : register(b1)
{
	float water_height;
	int use_water;
	float2 water_padding;
}

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 worldPosition : TEXCOORD1;
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
	float4 dirtColour;
	float4 sandColour;
	float blendAmount;

	grassColour = texture0.Sample(sampler0, input.tex);
	rockColour = texture1.Sample(sampler0, input.tex);
	dirtColour = texture2.Sample(sampler0, input.tex);
	sandColour = texture3.Sample(sampler0, input.tex);

	//calculate the slop value
	float slope = 1.0f - input.normal.y;

	lightColour = ambientColour + calculateLighting( -lightDirection, input.normal, diffuseColour);

	//calculate the texture based on the slope
	if (slope < 0.2) //check if the slope value is less than 0.2 (flat)
	{
		blendAmount = slope / 0.2f; //calculate the blend amount
		textureColour = lerp(grassColour, dirtColour, blendAmount);

		if (use_water == 1)
		{
			if (water_height >= input.worldPosition.y)
			{
				//now
				textureColour = lerp( sandColour,dirtColour, blendAmount);
			}

			if ((input.worldPosition.y - 0.3) < water_height)
			{
				textureColour = lerp(textureColour, sandColour, blendAmount);
			}
		}
	}
	else if ((slope < 0.7f) && (slope >= 0.2f)) //check if the slo[e value is between 0.2 and 0.7 (average)
	{
		blendAmount = (slope - 0.2f) * (1.0f / (0.7f - 0.2f));;

		textureColour = lerp(dirtColour, rockColour, blendAmount);

		if (use_water == 1)
		{
			if (water_height >= input.worldPosition.y)
			{
				textureColour = lerp( sandColour,rockColour, blendAmount);
			}

			if ((input.worldPosition.y - 0.3) < water_height)
			{
				textureColour = lerp(textureColour, sandColour, blendAmount);
			}
		}
	}

	if (slope >= 0.7) //check if slope is greater than 0.7 (steep)
	{
		textureColour = rockColour;
		
		if (use_water == 1)
		{
			blendAmount = slope / 0.7f;

			if (water_height >= input.worldPosition.y)
			{
				textureColour = rockColour;
			}
			
			if ((input.worldPosition.y - 0.3) < water_height)
			{
				slope = 1.0f - input.normal.y - 0.3;
				blendAmount = slope / 0.7f;
				textureColour = lerp(textureColour, sandColour, blendAmount);
			}
		}
	}

	//chekc if the point is under the height of the water

	//if (use_water)
	//{
	//	if (water_height > input.worldPosition.y)
	//	{
	//		float lerp_amount = (input.worldPosition.y) /5;
	//		textureColour = lerp(textureColour, sandColour, lerp_amount);
	//	}
	//}

	

	return lightColour * textureColour;
}


