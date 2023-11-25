// old_tv.glsl

uniform sampler2D tex;
uniform float time;
uniform vec2 resolution;

void main() {
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    vec3 color = texture2D(tex, uv).rgb;

    // Add some noise
    float noise = (fract(sin(dot(gl_FragCoord.xy + time, vec2(12.9898, 78.233))) * 43758.5453) - 0.5;
    color += noise * 0.1;

    // Simulate scanlines
    if (mod(gl_FragCoord.y, 2.0) < 1.0) {
        color *= 0.9;
    }

    // Simulate CRT curvature
    uv = 2.0 * uv - 1.0;
    float r = length(uv);
    color *= 1.0 - 0.1 * r * r;

    gl_FragColor = vec4(color, 1.0);
}
