//--------------------------------------------------------------------------------------
// File: PhongShaders.fx
//
// Copyright (c) Kyung Hee University.
//--------------------------------------------------------------------------------------

#define NUM_LIGHTS (2)

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register( t0 );
Texture2D txNormal : register( t1 );
TextureCube skyMap : register( t2 );
SamplerState samLinear : register( s0 );
SamplerState samNormal : register( s1 );
SamplerState samSkyMap : register( s2 );

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

  Summary:  Constant buffer used for world transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangesEveryFrame : register( b2 )
{
	matrix World;
	float4 OutputColor;
	bool HasNormalMap;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbLights

  Summary:  Constant buffer used for shading
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbLights : register( b3 )
{
	float4 LightPositions[NUM_LIGHTS];
	float4 LightColors[NUM_LIGHTS];
	float4 LightAttenuationDistance[NUM_LIGHTS];
};

//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   VS_PHONG_INPUT

  Summary:  Used as the input to the vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct VS_PHONG_INPUT
{
	float4 Position : POSITION;
	float2 TexCoord : TEXCOORD0;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Bitangent : BITANGENT;
	row_major matrix mTransform : INSTANCE_TRANSFORM;
};

struct VS_ENV_INPUT
{
	float4 Position : POSITION;
	float2 TexCoord : TEXCOORD0;
	float3 Normal : NORMAL;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   PS_PHONG_INPUT

  Summary:  Used as the input to the pixel shader, output of the 
            vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct PS_PHONG_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
	float3 Norm : NORMAL;
	float4 WorldPos : POSITION;
	float3 Tangent : TANGENT;
	float3 Bitangent : BITANGENT;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   PS_LIGHT_CUBE_INPUT

  Summary:  Used as the input to the pixel shader, output of the 
            vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct PS_LIGHT_CUBE_INPUT
{
	float4 Position : SV_POSITION;
};

struct PS_ENV_INPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float3 ReflectionVec : REFLECTION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_PHONG_INPUT VSPhong(VS_PHONG_INPUT input)
{
	PS_PHONG_INPUT output = (PS_PHONG_INPUT)0;
	output.Pos = input.Position;
	output.Pos = mul(output.Pos, World);
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);
	output.Tex = input.TexCoord;
	output.Norm = normalize(mul(float4(input.Normal, 1.0f), World).xyz);
	output.WorldPos = mul(input.Position, World);

	if(HasNormalMap)
	{
		output.Tangent = normalize(mul(float4(input.Tangent, 0.0f), World).xyz);
		output.Bitangent = normalize(mul(float4(input.Bitangent, 0.0f), World).xyz);
	}

	return output;
}

PS_LIGHT_CUBE_INPUT VSLightCube(VS_PHONG_INPUT input)
{
	PS_LIGHT_CUBE_INPUT output = (PS_LIGHT_CUBE_INPUT)0;
	output.Position = input.Position;
	output.Position = mul(output.Position, World);
	output.Position = mul(output.Position, View);
	output.Position = mul(output.Position, Projection);

	return output;
}

PS_ENV_INPUT VSEnvironmentMap(VS_ENV_INPUT input)
{
	PS_ENV_INPUT output = (PS_ENV_INPUT)0;
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
float4 PSPhong(PS_PHONG_INPUT input) : SV_Target
{
	float3 toViewDir = normalize((CameraPosition - input.WorldPos).xyz);
	float3 normal = normalize(input.Norm);
	
	float3 ambient = float3(0.1f, 0.1f, 0.1f);
	float3 diffuse = float3(0, 0, 0);
	float3 specular = float3(0, 0, 0);

	if(HasNormalMap)
	{
		float4 bumpMap = txNormal.Sample(samNormal, input.Tex);

		bumpMap = (bumpMap * 2.0f) - 1.0f;

		float3 bumpNormal = (bumpMap.x * input.Tangent) + (bumpMap.y * input.Bitangent) + (bumpMap.z * normal);

		normal = normalize(bumpNormal);
	}

	for (uint i = 0; i < NUM_LIGHTS; ++i)
	{
		float distance_square = dot(input.WorldPos.xyz - LightPositions[i].xyz, input.WorldPos.xyz - LightPositions[i].xyz);

		float attenuation =  LightAttenuationDistance[i].z / (distance_square + 0.000001);

		ambient += float4(float3(0.1f,0.1f,0.1f) * LightColors[i].xyz, 1.0f) * attenuation;

		float3 fromLightDir = normalize((input.WorldPos - LightPositions[i]).xyz);
	
		diffuse += max(dot(normal, -fromLightDir), 0) * LightColors[i].xyz * attenuation;
		
		float3 refDir = reflect(fromLightDir, normal);

		specular += pow(max(dot(refDir, toViewDir), 0), 20) * LightColors[i].xyz * attenuation;
	}

	return float4(saturate(ambient + diffuse + specular), 1) * txDiffuse.Sample(samLinear, input.Tex);
}

float4 PSLightCube(PS_LIGHT_CUBE_INPUT input) : SV_Target
{
	return OutputColor;
}

float4 PSEnvironmentMap(PS_ENV_INPUT input) : SV_TARGET
{
	float4 environment = skyMap.Sample(samSkyMap, input.ReflectionVec);
	return environment;
}