#version 410 core

uniform vec2 iResolution;
uniform vec2 iMouse;
uniform float iScroll;
uniform float iTime;
out vec4 fragColor;

float zoom = 2.1;

void mainImage(out vec4 fragColor, in vec2 fragCoord);
void main()
{
    mainImage(fragColor, gl_FragCoord.xy);
}

// -----------------

float circle(vec2 uv, vec2 p, float r, float blur) {
    float d = length(uv-p);
    // float c = d < r ? 1.0 : 0.0;
    float c = smoothstep(r+blur, r, d);
    return c;
}

vec2 rotateZ(vec2 p, float a) {
	return cos(a)*p.xy+sin(a)*vec2(p.y,-p.x);
}

float N21(vec2 p) {
    p = fract(p*vec2(233.34, 851.73));
    p += dot(p, p+23.45);
    return fract(p.x*p.y);
}

vec3 N23(vec2 p) {
    float a = N21(p.xy);
    float b = N21(p.yx);
    return vec3(a, b, N21(p + vec2(a,b)));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord.xy / iResolution.xy;                // [0.0 -> 1.0, 0.0 -> 1.0]
    uv.x -= 0.5; // from bottom-left, to center-middle
    uv.x *= iResolution.x / iResolution.y; // keep aspect ratio

    zoom = iScroll;
    //zoom += 2*sin(iTime/2); // zoom in and out
    uv *= 30*zoom;
    // uv = rotateZ(uv, iTime); // rotate

    vec2 gp = floor(uv); // grid coordinate

    float s = 1.0;
    vec3 col = N23(gp) * s; // random color for ball

    col = vec3(0.0); // black by default

    if (gp.y == 0 || gp.y == 1) {
        // uv.x -= 0.5;
        if (gp.x == 0) {
            col = vec3(1.0);
        }
    }
    if (gp.y > 1) {
        float e = exp2(gp.y - 2);
        if (gp.x >= -e && gp.x < e) {
            col = vec3(1.0);
        }
    }

    vec2 gv = fract(uv)-0.5; // in cell coordinate (-0.5 -> center in the middle)
    float c = circle(gv, vec2(0., 0.), 0.20, 0.005);

    fragColor = vec4(c * col, 1.);
}
