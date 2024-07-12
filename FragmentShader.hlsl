// an ultra simple hlsl fragment shader
// TODO: Part 2c // TODO: Part 4d
cbuffer SHADER_SCENE_DATA
{
    float4 lightDirection, lightColor;
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
StructuredBuffer<INSTANCE_DATA> DrawInfo : register(b1, space0);

// TODO: Part 3c
// TODO: Part 4a (optional)
// TODO: Part 4b
// TODO: Part 3g
// TODO: Part 3h
float4 main() : SV_TARGET
{
	// TODO: Part 3e
	// TODO: Part 3h
    return float4(1.0f, 1.0f, 1.0f, 0); // TODO: Part 1a (optional)
	// TODO: Part 4c
	// TODO: Part 4d (half-vector or reflect method, your choice)
}