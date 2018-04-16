//////////////////////////////////////////////////////////////////////
// HLSL File:
// This example is compiled using the fxc shader compiler.
// It is possible directly compile HLSL in VS2013
//////////////////////////////////////////////////////////////////////

// This first constant buffer is special.
// The framework looks for particular variables and sets them automatically.
// See the CommonApp comments for the names it looks for.
cbuffer CommonApp
{
	float4x4 g_WVP;
	float4 g_lightDirections[MAX_NUM_LIGHTS];
	float3 g_lightColours[MAX_NUM_LIGHTS];
	int g_numLights;
	float4x4 g_InvXposeW;
	float4x4 g_W;
};


// When you define your own cbuffer you can use a matching structure in your app but you must be careful to match data alignment.
// Alternatively, you may use shader reflection to find offsets into buffers based on variable names.
// The compiler may optimise away the entire cbuffer if it is not used but it shouldn't remove indivdual variables within it.
// Any 'global' variables that are outside an explicit cbuffer go
// into a special cbuffer called "$Globals". This is more difficult to work with
// because you must use reflection to find them.
// Also, the compiler may optimise individual globals away if they are not used.
cbuffer MyApp
{
	float	g_frameCount;
	float3	g_waveOrigin;
}


// VSInput structure defines the vertex format expected by the input assembler when this shader is bound.
// You can find a matching structure in the C++ code.
struct VSInput
{
	float4 pos:POSITION;
	float4 colour:COLOUR0;
	float3 normal:NORMAL;
	float2 tex:TEXCOORD;
};

// PSInput structure is defining the output of the vertex shader and the input of the pixel shader.
// The variables are interpolated smoothly across triangles by the rasteriser.
struct PSInput
{
	float4 pos:SV_Position;
	float4 colour:COLOUR0;
	float3 normal:NORMAL;
	float2 tex:TEXCOORD;
	float4 mat:COLOUR1;
};

// PSOutput structure is defining the output of the pixel shader, just a colour value.
struct PSOutput
{
	float4 colour:SV_Target;
};

// Define several Texture 'slots'
Texture2D g_materialMap;
Texture2D g_texture0;
Texture2D g_texture1;
Texture2D g_texture2;


// Define a state setting 'slot' for the sampler e.g. wrap/clamp modes, filtering etc.
SamplerState g_sampler;

// The vertex shader entry point. This function takes a single vertex and transforms it for the rasteriser.
void VSMain(const VSInput input, out PSInput output)
{
	output.pos = mul(input.pos, g_WVP);

	output.normal = input.normal;
	output.colour = input.colour;


	//the first value changes the range of the x or y values for the shaking
	//the second value increases or descreases the speed of the shaking with the lower the value the faster the shaking which is limited to the maximum frame count
	//the frame count limits the speed so everything isn't overloaded
	output.pos.x += 0 * radians(sin(output.pos.y - g_frameCount / 2));
	output.pos.y += 0 * radians(sin(output.pos.z - g_frameCount / 2));


	// map ranges from -512 to 512 which gives a total range of 1024
	//the texture is outputed
	output.tex.x = (input.pos.x + 512) / 1024;
	output.tex.y = 1 - (input.pos.z + 512) / 1024;

	output.mat = g_materialMap.SampleLevel(g_sampler, output.tex, 0);

}

// The pixel shader entry point. This function writes out the fragment/pixel colour.
void PSMain(const PSInput input, out PSOutput output)
{
	float4 t = (0,0,0,0); //initialize

	for (int i = 0; i < g_numLights; ++i)
	{
		clamp(t, 0, 1); //This keeps the value of t between 0 and 1 and it can't change hence the clamp name
		t += max(dot(normalize(input.normal), g_lightDirections[i]), 0); //normalize
		t.w = 1;
	}


	float4 m = input.mat;
	float4 colour2 = (0, 0, 0, 0);
	output.colour = colour2;


	output.colour += g_texture0.Sample(g_sampler, input.tex * 20) * m.x;
	output.colour += g_texture1.Sample(g_sampler, input.tex * 20) * m.y;
	output.colour += g_texture2.Sample(g_sampler, input.tex * 20) * m.z;


	output.colour.w = 1;
	output.colour *= t;

}