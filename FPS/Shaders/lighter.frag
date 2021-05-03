#version 330 core
struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    //float shininess;
};

/*struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dirLight;*/

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  
#define NR_POINT_LIGHTS 1
uniform PointLight pointLights[NR_POINT_LIGHTS];

/*struct SpotLight {
    vec3 position;
    vec3 direction;

    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform SpotLight spotLight;*/

out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;
  
uniform Material material;
uniform vec3 viewPos;

//uniform vec3 lightPos;
uniform float far_plane;
uniform samplerCube depthMap;

//vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float shadow);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir); 
//vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow);
float ShadowCalculation(vec3 fragPos);

//lowp float closestDepth;
//lowp vec3 result;

void main()
{
    //float lightDistance = length(fs_in.FragPos.xyz - lightPos);
    //lightDistance = lightDistance / far_plane;
    //gl_FragDepth = lightDistance;
    // shadow
    float shadow = ShadowCalculation(fs_in.FragPos);

    // properties
    vec3 norm = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    /*vec3 lightDir = normalize(pointLights[0].position - fs_in.FragPos);
    // diffuse shading
    float diff = max(dot(norm, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 2);
    // attenuation
    float dist = length(pointLights[0].position - fs_in.FragPos);
    float attenuation = 1.0 / (pointLights[0].constant + pointLights[0].linear * dist + pointLights[0].quadratic * (dist * dist));    
    // combine results
    vec3 coloura = pointLights[0].ambient  * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
    vec3 colourd = pointLights[0].diffuse  * diff * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
    vec3 colours = pointLights[0].specular * spec * vec3(texture(material.texture_specular1, fs_in.TexCoords));
    coloura  *= attenuation;
    colourd  *= attenuation;
    colours *= attenuation;
    vec3 result = (coloura + colourd + colours);*/
    //return ambient + (1.0 - shadow) * (diffuse + specular);
    //return ambient + max(0.0, 1.0 - shadow) * (diffuse + specular);

    // might have to use a few shadow values for each respective light in the scene
    //vec3 result = CalcSpotLight(spotLight, norm, fs_in.FragPos, viewDir, shadow);    
    //for(int i = 0; i < NR_POINT_LIGHTS; i++)
    //    result += CalcPointLight(pointLights[i], norm, fs_in.FragPos, viewDir, shadow);
    //vec3 fragToLight = normalize(fs_in.FragPos - pointLights[0].position); 
    //float closestDepth = texture(depthMap, fragToLight).r;
    vec3 result = CalcPointLight(pointLights[0], norm, fs_in.FragPos, viewDir);
    
    // just view depth map for debugging
    //closestDepth *= far_plane;
    //float shadow = closestDepth / far_plane;
    //float shadow = closestDepth;
    //shadow = 0.4;
//    if (shadow < 0.0) {
 //       shadow = 0.0;
 //   }
 //   if (shadow > 1.0) {
 //       shadow = 1.0;
 //   }
    //shadow = min(1.0, shadow);
    //shadow = max(0.0, shadow);
    //if (isinf(shadow)) {
    //    shadow = 0.0;
    //} else if (isnan(shadow)) {
    //    shadow = 0.0;
    //}
//    result = vec3(shadow * 0.5, shadow * 0.5, shadow * 0.5);
    //result = vec3(shadow, result.y, result.z);
    //vec3 final = vec3(shadow, result.y, result.z);

    /*float x = 1.0;

    if (closestDepth > 0.5) {
        x = 1.0;
    } else {
        x = 0.0;
    }

    float red = result.x * x;
    float green = result.y * x;
    float blue = result.z * x;

    if (red < 0.0) {
        red = 0.0;
    }
    if (red > 1.0) {
        red = 1.0;
    }
    if (green < 0.0) {
        green = 0.0;
    }
    if (green > 1.0) {
        green = 1.0;
    }
    if (blue < 0.0) {
        blue = 0.0;
    }
    if (blue > 1.0) {
        blue = 1.0;
    }
    vec3 final = vec3(red, green, blue);*/
//    result = vec3(shadow * result.x, shadow * result.y, shadow * result.z);
//    result = vec3(shadow, shadow, shadow);
    //result = result * shadow;
    //result *= closestDepth;
    //gl_FragData[0] = vec4(final, 1.0);
    //FragColor.r = shadow;
    //FragColor.g = shadow;
    //FragColor.b = shadow;
    //FragColor.a = 1.0;
    //FragColor = vec4(texture(depthMap, fragToLight).r * (ambient + diffuse + specular), 1.0);
    //shadow = 0.0;
    //FragColor = vec4(vec3(shadow), 1.0);
    result *= (1.0 - shadow);
    FragColor = vec4(result, 1.0);
    //FragColor = vec4(vec3(1.0) * result, 1.0);
    //shadow = 1.0;
}

float ShadowCalculation(vec3 fragPos)
{
    vec3 fragToLight = fragPos - pointLights[0].position; 
    float closestDepth = texture(depthMap, fragToLight).r;
    closestDepth *= far_plane;
    float currentDepth = length(fragToLight);
    float bias = 0.05;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
    /*
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - pointLights[0].position;
    // use the fragment to light vector to sample from the depth map    
    // float closestDepth = texture(depthMap, fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    // closestDepth *= far_plane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // test for shadows
    // float bias = 0.05; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    // float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;
    // PCF
    // float shadow = 0.0;
    // float bias = 0.05; 
    // float samples = 4.0;
    // float offset = 0.1;
    // for(float x = -offset; x < offset; x += offset / (samples * 0.5))
    // {
        // for(float y = -offset; y < offset; y += offset / (samples * 0.5))
        // {
            // for(float z = -offset; z < offset; z += offset / (samples * 0.5))
            // {
                // float closestDepth = texture(depthMap, fragToLight + vec3(x, y, z)).r; // use lightdir to lookup cubemap
                // closestDepth *= far_plane;   // Undo mapping [0;1]
                // if(currentDepth - bias > closestDepth)
                    // shadow += 1.0;
            // }
        // }
    // }
    // shadow /= (samples * samples * samples);
    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= far_plane;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);
        
    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    
        
    return shadow;*/
}

/*vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float shadow) {
    vec3 lightDir = normalize(-light.direction);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); // took out material.shininess

    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, fs_in.TexCoords));

    return ambient + (1.0 - shadow) * (diffuse + specular);
}*/

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 2);
    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient  = light.ambient  * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, fs_in.TexCoords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    //return ambient + (1.0 - shadow) * (diffuse + specular);
    //return ambient + max(0.0, 1.0 - shadow) * (diffuse + specular);
    return ambient + diffuse + specular;
    //return ambient;
} 

/*vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // attenuation
    float dist = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, fs_in.TexCoords));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return ambient + (1.0 - shadow) * (diffuse + specular);
}*/