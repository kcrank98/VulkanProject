// an ultra simple hlsl fragment shader
// TODO: Part 2c // TODO: Part 4d
cbuffer SHADER_SCENE_DATA : register(b0, space0)
{
    float4 lightDirection, lightColor, sunAmbient, camPos;
    matrix viewMatrix, projectionMatrix;
};

// TODO: Part 3b
struct VEC3
{
    float x, y, z;
};

struct OBJ_ATTRIBUTES
{
    VEC3 Kd; // diffuse reflectivity
    float d; // dissolve (transparency) 
    VEC3 Ks; // specular reflectivity
    float Ns; // specular exponent
    VEC3 Ka; // ambient reflectivity
    float sharpness; // local reflection map sharpness
    VEC3 Tf; // transmission filter
    float Ni; // optical density (index of refraction)
    VEC3 Ke; // emissive reflectivity
    uint illum; // illumination model
};

struct INSTANCE_DATA
{
    matrix worldMatrix;
    OBJ_ATTRIBUTES material;
};
StructuredBuffer<INSTANCE_DATA> DrawInfo : register(b0, space1);

// TODO: Part 3c
// TODO: Part 4a (optional)
// TODO: Part 4b
// TODO: Part 3g
// TODO: Part 3h
struct Out_Vertex
{
    float4 posH : SV_POSITION;
    float4 posW : WORLD;
    float4 clr : COLOR;
    float4 nrm : NORMAL;
    nointerpolation uint index : INDEX;
};

float4 main(Out_Vertex input) : SV_TARGET
{
	// Normalize the light direction
    float3 lightDir = normalize(lightDirection.xyz);
    //float3 lightDir = normalize(input.posW.xyz);

    // Normalize the normal
    float3 normal = normalize(input.nrm.xyz);

    // Lambertian diffuse term
    float NdotL = max(dot(normal, -lightDir), 0.0);

    // Get the diffuse reflectivity from the material
    float3 diffuseColor = float3(DrawInfo[input.index].material.Kd.x,
    DrawInfo[input.index].material.Kd.y, DrawInfo[input.index].material.Kd.z);

    // Calculate the final color
    float3 color = diffuseColor * lightColor.rgb * NdotL;
    
    float3 totalReflected = { 0.0f, 0.0f, 0.0f };
    
    //float3 sunAmbient = { 0.0f, 0.0f, 0.0f };
    //sunAmbient.x = lightColor.x * 0.25f;
    //sunAmbient.y = lightColor.y * 0.25f;
    //sunAmbient.z = lightColor.z * 0.35f;
    
    float3 ambient = { 0.0f, 0.0f, 0.0f };
    ambient.x = DrawInfo[input.index].material.Ka.x;
    ambient.y = DrawInfo[input.index].material.Ka.y;
    ambient.z = DrawInfo[input.index].material.Ka.z;
    float3 totalIndirect = ambient * sunAmbient.xyz;
    //float3 direct = saturate(diffuseColor + totalIndirect);

    float3 diffuse = { 0.0f, 0.0f, 0.0f };
    diffuse.x = DrawInfo[input.index].material.Kd.x;
    diffuse.y = DrawInfo[input.index].material.Kd.y;
    diffuse.z = DrawInfo[input.index].material.Kd.z;
    
    float3 emissive = { 0.0f, 0.0f, 0.0f };
    emissive.x = DrawInfo[input.index].material.Ke.x;
    emissive.y = DrawInfo[input.index].material.Ke.y;
    emissive.z = DrawInfo[input.index].material.Ke.z;
    
    //spec light
    
    
    float3 saturated = saturate(color + totalIndirect);
    
    return float4((saturated * diffuse)
    + totalReflected + emissive, input.clr.w);
    
    //return saturate(totalDirect + totalIndirect) * diffuse + totalReflected + emissive
    
}