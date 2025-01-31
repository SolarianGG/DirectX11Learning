
struct DirectionalLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float3 Direction;
    float pad;
};

struct PointLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;

    float3 Position;
    float Range;

    float3 Att;
    float pad;
};

struct SpotLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;

    float3 Position;
    float Range;

    float3 Direction;
    float Spot;

    float3 Att;
    float pad;
};

struct Material
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular; // w = SpecPower
    float4 Reflect;
};

float toonDiffusal(float kd)
{
    if (kd <= 0.0)
        return 0.4f;
    else if (kd <= 0.5)
        return 0.6f;
    else
        return 1.0f;
}

float toonSpecular(float ks)
{
    if (ks <= 0.1)
        return 0.0f;
    else if (ks <= 0.8)
        return 0.5f;
    else
        return 0.8f;
}

void ComputeDirectionalLight(Material mat, DirectionalLight L, float3 normal, float3 toEye, 
    out float4 ambient, out float4 diffuse, out float4 spec
)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The light vector aims opposite the direction the light rays travel.
    float3 lightVec = -L.Direction;

	// Add ambient term.
    ambient = mat.Ambient * L.Ambient;

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	
    float diffuseFactor = dot(lightVec, normal);
    
   //  diffuseFactor = toonDiffusal(diffuseFactor);

	// Flatten to avoid dynamic branching.
	[flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
        // specFactor = toonSpecular(specFactor);
					
        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        spec = specFactor * mat.Specular * L.Specular;
    }

}

void ComputePointLight(Material mat, PointLight L, float3 pos, float3 normal, float3 toEye,
				   out float4 ambient, out float4 diffuse, out float4 spec)
{
	// Initialize outputs.
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The vector from the surface to the light.
    float3 lightVec = L.Position - pos;
		
	// The distance from surface to light.
    float d = length(lightVec);
	
	// Range test.
    if (d > L.Range)
        return;
		
	// Normalize the light vector.
    lightVec /= d;
	
	// Ambient term.
    ambient = mat.Ambient * L.Ambient;

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.

    float diffuseFactor = dot(lightVec, normal);
    // diffuseFactor = toonDiffusal(diffuseFactor);

	// Flatten to avoid dynamic branching.
	[flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
        //  specFactor = toonSpecular(specFactor);
					
        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        spec = specFactor * mat.Specular * L.Specular;
    }

	// Attenuate
    float att = 1.0f / dot(L.Att, float3(1.0f, d, d * d));

    diffuse *= att;
    spec *= att;
}

void ComputeSpotLight(Material mat, SpotLight L, float3 pos, float3 normal, float3 toEye,
				  out float4 ambient, out float4 diffuse, out float4 spec)
{
	// Initialize outputs.
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The vector from the surface to the light.
    float3 lightVec = L.Position - pos;
		
	// The distance from surface to light.
    float d = length(lightVec);
	
	// Range test.
    if (d > L.Range)
        return;
		
	// Normalize the light vector.
    lightVec /= d;
	
	// Ambient term.
    ambient = mat.Ambient * L.Ambient;

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.

    float diffuseFactor = dot(lightVec, normal);
   // diffuseFactor = toonDiffusal(diffuseFactor); // for toon

	// Flatten to avoid dynamic branching.
	[flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
        //specFactor = toonSpecular(specFactor); // for toon 
					
        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        spec = specFactor * mat.Specular * L.Specular;
    }
	
	// Scale by spotlight factor and attenuate.
    float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);

	// Scale by spotlight factor and attenuate.
    float att = spot / dot(L.Att, float3(1.0f, d, d * d));

    ambient *= spot;
    diffuse *= att;
    spec *= att;
}


