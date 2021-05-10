#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 FragPos;
uniform vec3 viewPos;
uniform vec3 fogColor;
uniform float fogIntensity;

vec3 applyFog(vec3 shading, float dist, vec3 fogColor, float fogIntensity);

void main()
{
    vec3 lightColor = vec3(1.0);
    FragColor = vec4(applyFog(lightColor, length(FragPos - viewPos), fogColor, fogIntensity), 1.0);
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 0)
        BrightColor = vec4(FragColor.rgb, 1.0);
	else
		BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}

vec3 applyFog(vec3 shading, float dist, vec3 fogColor, float fogIntensity)
{
    float fogAmount = 1.0 - exp(-dist * fogIntensity);
    return mix(shading, fogColor, fogAmount);
}