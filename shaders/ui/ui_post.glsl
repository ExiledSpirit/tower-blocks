#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;
uniform float effectIntensity; // 0.0 to 1.0
uniform float time;

void main() {
    // Chromatic Aberration: Split Red and Blue channels
    float offset = 0.005 * effectIntensity;
    float r = texture(texture0, vec2(fragTexCoord.x + offset, fragTexCoord.y)).r;
    float g = texture(texture0, fragTexCoord).g;
    float b = texture(texture0, vec2(fragTexCoord.x - offset, fragTexCoord.y)).b;
    
    vec4 texel = vec4(r, g, b, 1.0);
    
    // Vignette effect (darken corners)
    vec2 uv = fragTexCoord - 0.5;
    float vgn = 1.0 - dot(uv, uv) * 1.2;
    
    // Pulse brightness if perfect hit
    float pulse = 1.0 + (sin(time * 10.0) * 0.2 * effectIntensity);
    
    finalColor = texel * vgn * pulse;
    
    // Maintain transparency for the UI (so we don't see a black box over the game)
    finalColor.a = texture(texture0, fragTexCoord).a;
}
