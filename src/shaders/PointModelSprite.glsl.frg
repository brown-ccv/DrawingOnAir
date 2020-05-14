
uniform sampler2D spriteTex;
uniform sampler2D colorTex;

void main()
{
  vec2 spriteCoords = vec2(gl_TexCoord[2].s/gl_TexCoord[2].t + gl_TexCoord[0].s/gl_TexCoord[2].t, 
                           gl_TexCoord[0].t);
  vec4 spriteCol = texture2D(spriteTex, spriteCoords);
  vec4 colorCol  = texture2D(colorTex, gl_TexCoord[1].st);
  gl_FragColor = spriteCol * colorCol;
}

