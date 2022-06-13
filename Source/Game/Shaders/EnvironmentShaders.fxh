//--------------------------------------------------------------------------------------
// File: EnvironmentShaders.fxh
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License (MIT).
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
TextureCube environmentMap : register( t0 );
SamplerState textureSampler : register( s0 );

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangeOnCameraMovement

  Summary:  Constant buffer used for view transformation and shading
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangeOnCameraMovement : register( b0 )
{
	matrix View;
	float4 CameraPosition;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangeOnResize

  Summary:  Constant buffer used for projection transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangeOnResize : register( b1 )
{
	matrix Projection;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangesEveryFrame

  Summary:  Constant buffer used for world transformation, and the 
            color of the voxel
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangesEveryFrame : register( b2 )
{
	matrix World;
	float4 OutputColor;
	bool HasNormalMap;
};

//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   VS_INPUT

  Summary:  Used as the input to the vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct VS_INPUT
{
	float4 Position : POSITION;
	float2 TexCoord : TEXCOORD0;
	float3 Normal : NORMAL;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   PS_INPUT

  Summary:  Used as the input to the pixel shader, output of the 
            vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float3 TexCoord : TEXCOORD0;
	float3 ReflectionVec : REFLECTION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------

PS_INPUT VSEnvironmentMap(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.Position = input.Position;
	output.Position = mul(output.Position, World);
	output.Position = mul(output.Position, View);
	output.Position = mul(output.Position, Projection);
	output.TexCoord = input.TexCoord;
	
	float3 worldPos = mul(input.Position, World).xyz;
	float3 incident = normalize(worldPos - CameraPosition);
	float3 normal = normalize(mul(float4(input.Normal, 0), World).xyz);

	output.ReflectionVec = reflect(incident, normal);

	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float4 PSEnvironmentMap(PS_INPUT input) : SV_TARGET
{
	float4 output = (float4)0;

	float3 environment = environmentMap.Sample(textureSampler, input.ReflectionVec).rgb;

	output.rgb = saturate(lerp(ambient, environment, ReflectionAmount));
	output.a = color.a;

	return output;
}