#version 330
//texture variables
in vec2 aTexCoord;    
out vec2 texCoord0;
  
//Uniforms: Transformation Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;

// Particle-specific Uniforms
uniform vec3 initialPos = vec3(0, 0, 0);		// Initial Position (source of the fountain)
uniform vec3 gravity = vec3(0.0, 0.01, 0.0);	// Gravity Acceleration in world coords
uniform float particleLifetime;			// Max Particle Lifetime
uniform float time;					// Animation Time

// Special Vertex Attributes
in vec3 aVelocity;					// Particle initial velocity
in float aStartTime;					// Particle "birth" time

// Output Variable (sent to Fragment Shader)
out float age;					// age of the particle (0..1)

void main()
{
	float t = mod(time - aStartTime, particleLifetime);
	vec3 pos = initialPos + aVelocity * t + gravity * t * t;
	age = t / particleLifetime;

	// calculate position (normal calculation not applicable here)
	vec4 position = matrixModelView * vec4(pos, 1.0);
	gl_Position = matrixProjection * position;
	gl_PointSize = mix(250, 500, age);

	//calculation in main() Make sure it’s at the end of the main()
	texCoord0 = aTexCoord; 
}
