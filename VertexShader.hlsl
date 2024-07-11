// an ultra simple hlsl vertex shader
// TODO: Part 1f
struct In_Vertex
{
    float3 posH : POSITION;
    float3 clr : COLOR;
    float3 nrm : NORMAL;
};

// TODO: Part 2c // TODO: Part 4d
// TODO: Part 3b
// TODO: Part 3c
// TODO: Part 4a
// TODO: Part 4b
// TODO: Part 3g
// TODO: Part 3h
float4 main(In_Vertex inputVertex : POSITION) : SV_POSITION 
{
	// TODO: Part 1h
    //inputVertex.posH.y += -0.75f;
    //inputVertex.posH.z += 0.75f;
	
	// TODO: Part 3g
	// TODO: Part 2f
	// TODO: Part 3h
	// TODO: Part 4b
	return float4(inputVertex.posH, 1);
}