precision mediump   float;
varying   vec4      v_color;   /*顶点颜色*/
uniform   sampler2D colorMap;  /*纹理*/
smooth in vec2      vTexCoords;/*纹理坐标*/
void main(void){
 vec4 clr=v_color;/*顶点颜色*/
 clr*=texture(colorMap,vTexCoords);
 /*连同所有数据一起降低亮度*/
 /*网页中的图片同样要受到影响*/
 gl_FragColor = vec4(
   clr.x*0.3,
   clr.y*0.3,
   clr.z*0.3,
   1.0
 );/*亮度降低至30%*/
}