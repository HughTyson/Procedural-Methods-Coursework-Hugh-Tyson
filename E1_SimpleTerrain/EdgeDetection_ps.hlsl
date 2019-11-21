Texture2D shaderTexture : register(t0);
SamplerState Sampler0 : register(s0);

#define clamp(value,minimum,maximum) max(min(value,maximum),minimum)

cbuffer ScreenSizeBuffer : register(b0)
{
	float screenWidth;
	float screenHeight;
	float2 padding;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};


float4 main(InputType input) : SV_TARGET
{

	float4 kernal_pos[9];


	float width = 1.f / screenWidth;
	float height = 1.f / screenHeight;

	//get the colour of the surrounding points around the original point (4)

	/*
		6 7 8
		3 4 5
		0 1 2

	*/
	kernal_pos[0] = shaderTexture.Sample(Sampler0, input.tex + float2(-width, -height));
	kernal_pos[1] = shaderTexture.Sample(Sampler0, input.tex + float2(0.f, -height));
	kernal_pos[2] = shaderTexture.Sample(Sampler0, input.tex + float2(width, -height));
	kernal_pos[3] = shaderTexture.Sample(Sampler0, input.tex + float2(-width, 0.f));
	kernal_pos[4] = shaderTexture.Sample(Sampler0, input.tex);
	kernal_pos[5] = shaderTexture.Sample(Sampler0, input.tex + float2(width, 0.f));
	kernal_pos[6] = shaderTexture.Sample(Sampler0, input.tex + float2(-width, height));
	kernal_pos[7] = shaderTexture.Sample(Sampler0, input.tex + float2(0.f, height));
	kernal_pos[8] = shaderTexture.Sample(Sampler0, input.tex + float2(width, height));

	/*

	create the sobel kernals one for horizontal, one for verticle

	horizontal

		-1 0 1
		-1 0 2
		-1 0 1

	verticle

		-1 -2 -1
		 0  0  0
		 1  2  1

	*/

	float4 sobel_verticle = (kernal_pos[0] + (2.f*kernal_pos[1]) + kernal_pos[2]) + (-kernal_pos[6] - (2.f*kernal_pos[7]) - kernal_pos[8]);
	float4 sobel_horizontal = (kernal_pos[2] + (2.f*kernal_pos[5]) + kernal_pos[8]) + (-kernal_pos[0] - (2.f*kernal_pos[3]) - kernal_pos[6]);

	//float4 sobel_verticle = (kernal_pos[6] + (2.f*kernal_pos[7]) + kernal_pos[8]) + (-kernal_pos[0] - (2.f*kernal_pos[1]) - kernal_pos[2]);
	//float4 sobel_horizontal = (kernal_pos[0] + (2.f*kernal_pos[3]) + kernal_pos[6]) + (-kernal_pos[2] - (2.f*kernal_pos[5]) - kernal_pos[8]);

	float4 sobel = sqrt((pow(sobel_horizontal, 2) + pow(sobel_verticle, 2)));

	float4 final_colour;

	final_colour = sobel;

	return final_colour;

}