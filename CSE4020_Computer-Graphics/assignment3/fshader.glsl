
#version 330


in  vec3 fN;
in  vec3 fL;
in  vec3 fE;

in  vec4 color;
in  vec2 texCoord;

out vec4 fColor;


uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform vec4 LightPosition;
uniform float Shininess;


uniform sampler2D mainTex;
uniform sampler2D cloudTex;
uniform sampler2D nightTex;

void main() 
{ 
    vec3 N = normalize(fN);
    vec3 E = normalize(fE);
    vec3 L = normalize(fL);

    vec3 H = normalize( L + E );
    
    vec4 ambient = AmbientProduct;

    float Kd = max(dot(L, N), 0.0);
    vec4 diffuse = Kd*DiffuseProduct;
    
    float Ks = pow(max(dot(N, H), 0.0), Shininess);
    vec4 specular = Ks*SpecularProduct;

    // discard the specular highlight if the light's behind the vertex
    if( dot(L, N) < 0.0 ) {
	specular = vec4(0.0, 0.0, 0.0, 1.0);
    }

    //fColor = ambient + diffuse + specular;
    //fColor.a = 1.0;




    vec4 earthColor = texture2D(mainTex, texCoord);
    vec4 cloudRed = texture2D(cloudTex, texCoord);
    vec4 cloudColor = vec4(cloudRed.x, cloudRed.x, cloudRed.x, cloudRed.a);
    //fColor = earthColor + cloudColor;

    vec4 nightRed = texture2D(nightTex, texCoord);
    vec4 nightColor = vec4(nightRed.x, nightRed.x, nightRed.x, nightRed.a);


    fColor = (ambient + diffuse + specular) * (earthColor + cloudColor) + nightColor;
    fColor.a = 1.0;
} 
