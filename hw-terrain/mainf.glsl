#version 300 es
precision highp float;
uniform vec4 color;
out vec4 fragColor;
in vec3 outnormal;
in vec3 fnormal;
in float height;
uniform vec3 eyedir;
uniform vec3 lightdir;
// const vec3 lightdir = vec3(0.8, -0.6, .5);
uniform int lightType;
/*
0 for Lambert
1 for Phong
2 for Blinn-Phong
*/
uniform int colorType;
/*
0 for Default
1 for Height-based color ramp
2 for Rocky cliffs
*/

void main() {
    vec4 usedColor;
    vec4 R = vec4(1, 0, 0, 0);
    vec4 G = vec4(0, 1, 0, 0);
    vec4 B = vec4(0, 0, 1, 0);
    vec3 n = normalize(fnormal);
    vec3 normal = normalize(outnormal);
    float pow1 = 20.0;
    float pow2 = 150.0;
    float k1 = 1.;
    float k2 = 20.5;

    switch(colorType) {
        case 0:
            usedColor = color;
            break;
        case 1:
            usedColor = height > 0. ? R * (1. - height) + G * height : B * (-height) + (1. + height) * R;
            break;
        case 2:
            if(dot(normal, vec3(0, 0, 1)) > 0.5) {
                usedColor = vec4(0, 1, 0, 0);//Green
            } else {
                usedColor = vec4(1, 0, 1, 0);//Purple
                k1 = .7;
                k2 = 10.;
                pow1 *= 100.;
                pow2 *= 100.;
            }
            break;
    }

    // lighting:
    vec3 x = normal * dot(normal, lightdir);
    vec3 r = 2.0 * x - lightdir;
    float phongbit = max(0.0, dot(r, eyedir));
    float phong = pow(phongbit, pow1);
    float lambert = k1 * max(0.0, dot(lightdir, normal));
    vec3 halfway = normalize(lightdir + vec3(0, 0, 1));
    float blinn = pow(max(dot(halfway, n), 0.0), pow2);
    vec3 lightcolor = vec3(1, 1, 1);
    switch(lightType) {
        case 0:
            fragColor = vec4((usedColor.rgb * lambert), color.a);
            break;
        case 1:
            fragColor = vec4((usedColor.rgb * lambert) + vec3(phong, phong, phong) / 3.0, color.a);
            break;
        case 2:
            fragColor = vec4(usedColor.rgb * (lightcolor * lambert) + (lightcolor * blinn) * k2, color.a);
            break;
    }

}