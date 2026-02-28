#version 330

out vec2 fragCoord;

void main() {
  FragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
  FragNormal = normalize(mat3(matModel) * vertexNormal);

  gl_Position = mvp * vec4(vertexPosition, 1.0);
}