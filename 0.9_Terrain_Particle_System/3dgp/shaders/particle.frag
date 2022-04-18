#version 330

in float age;
out vec4 outColor;

//global variables
uniform sampler2D texture0;  
in vec2 texCoord0; 




void main()
{

	outColor = texture(texture0, gl_PointCoord);
	outColor.a = 1 - outColor.r * outColor.g * outColor.b;
	outColor.a *= 1 - age;


//	outColor = vec4(0.5, 0.5, 0.5, 1 - age);
//	//calculation 
//	outColor *= texture(texture0, texCoord0); 

}
