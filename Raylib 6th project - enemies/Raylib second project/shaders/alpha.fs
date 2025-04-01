//GLSL OpenGL 3.3
 #version 330


 in vec2 fragTexCoord;			//Input texture coordinates passed from the vertex shader
 in vec4 fragColor;			//Input colors passed from the vertex shader
 out vec4 outputColor;			//Output colors returned to the framebuffer

 uniform sampler2D texture0;	//2D texture sampler used to fetch the texure color
 uniform vec4 colDiffuse;		//Diffuse color uniform to tint the final color
 
 void main()
 {
	//Samples the texture at the given texture element coordinate
    vec4 sampleColor = texture(texture0, fragTexCoord);
	 
	
	//If the sampled textele has zero (or less) alpha (then its transparent),
	//Discard the fragment, dont drawn or write it to the framebuffer
	if( sampleColor.a <= 0.0 )
	{
		discard;
	}
	
	//Multiplies the texure color with the diffuse and vertex color to get the final output color
    outputColor = sampleColor*colDiffuse*fragColor;
 }