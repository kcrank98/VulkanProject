// an ultra simple hlsl vertex shader
// TODO: Part 1f
struct In_Vertex
{
    float3 posH : POSITION;
    float3 clr : COLOR;
    float3 nrm : NORMAL;
};

struct Out_Vertex
{
    float4 posH : SV_POSITION;
    float4 posW : WORLD;
    float4 clr : COLOR;
    float4 nrm : NORMAL;
    nointerpolation uint index : INDEX;
};

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
// TODO: Part 4a
// TODO: Part 4b
// TODO: Part 3g
// TODO: Part 3h
Out_Vertex main(In_Vertex inputVertex,
    uint matrix_index : SV_InstanceID)
{
	// TODO: Part 1h
    //inputVertex.posH.y += -0.75f;
    //inputVertex.posH.z += 0.75f;
    
    Out_Vertex outputVertex = (Out_Vertex) 0;
    outputVertex.posH = float4(inputVertex.posH, 1);
	
	// TODO: Part 3g
	// TODO: Part 2f
    outputVertex.posH = mul(DrawInfo[matrix_index].worldMatrix, outputVertex.posH);
    outputVertex.posW = outputVertex.posH;
    outputVertex.posH = mul(viewMatrix, outputVertex.posH);
    outputVertex.posH = mul(projectionMatrix, outputVertex.posH);
	// TODO: Part 3h
    outputVertex.index = matrix_index;
    //outputVertex.nrm = float4(inputVertex.nrm, 0.0f);
    outputVertex.nrm = mul(DrawInfo[matrix_index].worldMatrix, float4(inputVertex.nrm, 0.0f));
    outputVertex.clr = float4(inputVertex.clr, 1.0f);
    
    // i think i need to do this?
    //camPos = mul(DrawInfo[matrix_index].worldMatrix, camPos);
    
	// TODO: Part 4b
    return outputVertex;
}