attribute  vec2 position;
attribute  vec3 color;
varying    vec3 fragmentColor;
smooth out vec2 vTexCoords;/*纹理坐标*/
void main(){
  fragmentColor=color;
  gl_Position=vec4(position.x, position.y, 0.0, 1.0);
  vTexCoords=position.xy;
}