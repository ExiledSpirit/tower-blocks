#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;
uniform float effectIntensity; // Pulse for perfect hits
uniform float time;

void main() {
    vec2 uv = fragTexCoord;
    
    // 1. Chromatic Aberration (The Pulse)
    float amount = 0.005 * effectIntensity;
    vec4 rCol = texture(texture0, vec2(uv.x + amount, uv.y));
    vec4 gCol = texture(texture0, uv);
    vec4 bCol = texture(texture0, vec2(uv.x - amount, uv.y));
    
    vec4 baseColor = vec4(rCol.r, gCol.g, bCol.b, gCol.a);

    // 2. Simple Bloom (Bright Pass + Blur)
    // We sample nearby pixels to create a "glow" around bright areas
    vec4 sum = vec4(0.0);
    float samples = 8.0;
    float spread = 0.003;

    for (float i = 0.0; i < samples; i++) {
        float angle = i * (6.2831 / samples);
        vec2 offset = vec2(cos(angle), sin(angle)) * spread;
        vec4 col = texture(texture0, uv + offset);
        
        // Only add to bloom if the pixel is bright (Yellow/White)
        float brightness = (col.r + col.g + col.b) / 3.0;
        if (brightness > 0.6) {
            sum += col * 0.25; 
        }
    }

    // 3. Combine base UI with the glow
    // We multiply sum by 1.5 to make it pop, and baseColor for the sharp text
    vec4 glow = sum * (1.0 + effectIntensity); 
    finalColor = baseColor + glow;
    
    // Maintain alpha from the original texture
    finalColor.a = baseColor.a;
}
