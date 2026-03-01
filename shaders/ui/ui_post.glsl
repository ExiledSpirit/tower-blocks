#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;
uniform float effectIntensity; 
uniform float time;

// CRT Configuration
const float curvature = 3.0;    // Higher = more screen bulge
const float scanlineWeight = 0.05; // Darkness of scanlines
const float scanlineFreq = 800.0;  // Number of scanlines

// Helper for screen curvature (Barrel Distortion)
vec2 curve(vec2 uv) {
    uv = uv * 2.0 - 1.0;
    vec2 offset = abs(uv.yx) / curvature;
    uv = uv + uv * offset * offset;
    uv = uv * 0.5 + 0.5;
    return uv;
}

void main() {
    // 1. Apply Curvature
    vec2 uv = curve(fragTexCoord);
    
    // Hard cutoff for the "bezel" (black edges outside the curve)
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        finalColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    // 2. Chromatic Aberration (Your existing logic)
    float amount = (0.005 + sin(time * 2.0) * 0.001) * effectIntensity;
    float rCol = texture(texture0, vec2(uv.x + amount, uv.y)).r;
    float gCol = texture(texture0, uv).g;
    float bCol = texture(texture0, vec2(uv.x - amount, uv.y)).b;
    float alpha = texture(texture0, uv).a;
    
    vec4 baseColor = vec4(rCol, gCol, bCol, alpha);

    // 3. Scanlines
    // We use a sine wave based on Y coordinate to create dark horizontal strips
    float scanline = sin(uv.y * scanlineFreq) * scanlineWeight;
    baseColor.rgb -= scanline;

    // 4. Vignette (Darkens the corners)
    float vignette = uv.x * uv.y * (1.0 - uv.x) * (1.0 - uv.y);
    vignette = clamp(pow(16.0 * vignette, 0.1), 0.0, 1.0);
    baseColor.rgb *= vignette;

    // 5. Flicker (Subtle noise)
    float flicker = 1.0 - (sin(time * 30.0) * 0.01);
    baseColor.rgb *= flicker;

    // 6. Final Color Assembly
    // (Combining your existing Bloom logic here if desired)
    finalColor = baseColor;
}