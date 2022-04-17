#include <iostream>
#include "GL/glew.h"
#include "GL/3dgl.h"
#include "GL/glut.h"
#include "GL/freeglut_ext.h"

// Include GLM core features
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;
using namespace glm;

bool fogOn;
float waterLevel = 10.6f;

// Particle System Params
const float M_PI = 3.14159;
const float PERIOD = 0.01f;
const float LIFETIME = 6;
const int NPARTICLES = (int)(LIFETIME / PERIOD);


C3dglProgram Program;
C3dglProgram ProgramWater;
C3dglProgram ProgramTerrain;
C3dglProgram ProgramParticle;
C3dglProgram ProgramAnim;

// 3D Models
C3dglTerrain terrain, terrain2, water;
C3dglSkyBox skybox;
C3dglModel test;
C3dglModel player;

//textures
C3dglBitmap grass, bm_pebbles, bm_land;
C3dglBitmap bm_terrainTexColor;
C3dglBitmap bm_terrainTexNormal;

C3dglBitmap bm_charTexColor;
C3dglBitmap bm_charTexNormal;

GLuint idTexGrass, idTexNone, idColourTerrain, idNormalTerrain, idTexChar, idNormalChar, idTexPeb, idTexLand;
GLuint idBufferVelocity, idBufferStartTime;


// camera position (for first person type camera navigation)
mat4 matrixView;			// The View Matrix
float angleTilt = 15.f;		// Tilt Angle
vec3 cam(0);				// Camera movement values

bool init()
{
	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!

	//Initialise Shaders
	//C3dglShader AnimVertexShader;
	C3dglShader VertexShader;
	C3dglShader FragmentShader;
	C3dglShader WaterVertexShader;
	C3dglShader WaterFragmentShader;
	C3dglShader TerrainVertexShader;
	C3dglShader TerrainFragmentShader;

	C3dglShader ParticleVertexShader;
	C3dglShader ParticleFragmentShader;

	if (!VertexShader.Create(GL_VERTEX_SHADER))return false;
	if (!VertexShader.LoadFromFile("basic.vert"))return false;
	if (!VertexShader.Compile())return false;

	//if (!AnimVertexShader.Create(GL_VERTEX_SHADER))return false;
	//if (!AnimVertexShader.LoadFromFile("anim.vert"))return false;
	//if (!AnimVertexShader.Compile())return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER))return false;
	if (!FragmentShader.LoadFromFile("basic.frag"))return false;
	if (!FragmentShader.Compile())return false;

	if (!Program.Create())return false;
	if (!Program.Attach(VertexShader))return false;
	if (!Program.Attach(FragmentShader))return false;
	if (!Program.Link())return false;
	if (!Program.Use(true))return false;

	/*if (!ProgramAnim.Create())return false;
	if (!ProgramAnim.Attach(AnimVertexShader))return false;
	if (!ProgramAnim.Link())return false;
	if (!ProgramAnim.Use(true))return false;*/

	if (!WaterVertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!WaterVertexShader.LoadFromFile("shaders/water.vert")) return false;
	if (!WaterVertexShader.Compile()) return false;

	if (!WaterFragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!WaterFragmentShader.LoadFromFile("shaders/water.frag")) return false;
	if (!WaterFragmentShader.Compile()) return false;

	if (!ProgramWater.Create()) return false;
	if (!ProgramWater.Attach(WaterVertexShader)) return false;
	if (!ProgramWater.Attach(WaterFragmentShader)) return false;
	if (!ProgramWater.Link()) return false;
	if (!ProgramWater.Use(true)) return false;

	if (!TerrainVertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!TerrainVertexShader.LoadFromFile("shaders/terrain.vert")) return false;
	if (!TerrainVertexShader.Compile()) return false;

	if (!TerrainFragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!TerrainFragmentShader.LoadFromFile("shaders/terrain.frag")) return false;
	if (!TerrainFragmentShader.Compile()) return false;

	if (!ProgramTerrain.Create()) return false;
	if (!ProgramTerrain.Attach(TerrainVertexShader)) return false;
	if (!ProgramTerrain.Attach(TerrainFragmentShader)) return false;
	if (!ProgramTerrain.Link()) return false;
	if (!ProgramTerrain.Use(true)) return false;


	if (!ParticleVertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!ParticleVertexShader.LoadFromFile("shaders/particle.vert")) return false;
	if (!ParticleVertexShader.Compile()) return false;

	if (!ParticleFragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!ParticleFragmentShader.LoadFromFile("shaders/particle.frag")) return false;
	if (!ParticleFragmentShader.Compile()) return false;

	if (!ProgramParticle.Create()) return false;
	if (!ProgramParticle.Attach(ParticleVertexShader)) return false;
	if (!ProgramParticle.Attach(ParticleFragmentShader)) return false;
	if (!ProgramParticle.Link()) return false;
	if (!ProgramParticle.Use(true)) return false;

	::glutSetVertexAttribCoord3(Program.GetAttribLocation("aVertex"));
	::glutSetVertexAttribNormal(Program.GetAttribLocation("aNormal"));
	//::glutSetVertexAttribCoord3(ProgramParticle.GetAttribLocation("aVertex"));
	//::glutSetVertexAttribNormal(ProgramParticle.GetAttribLocation("aNormal"));

	// Setup the particle system
	ProgramParticle.SendUniform("initialPos", -160.0, 15.0, -220.0);
	ProgramParticle.SendUniform("gravity", 0.0, 0.1, 0.0);
	ProgramParticle.SendUniform("particleLifetime", LIFETIME);


	// setup lights (for basic and terrain programs only, water does not use these lights):
	ProgramWater.SendUniform("lightAmbient.color", 0.1, 0.1, 0.1);
	ProgramWater.SendUniform("lightDir.direction", 1.0, 0.5, 1.0);
	ProgramWater.SendUniform("lightDir.diffuse", 1.0, 1.0, 1.0);

	//setup materials (for basic and terrain programs only, water does not use these materials):
	ProgramWater.SendUniform("materialAmbient", 1.0, 1.0, 1.0);		// full power (note: ambient light is extremely dim)
	ProgramWater.SendUniform("materialDiffuse", 1.0, 1.0, 1.0);

	// setup lights (for basic and terrain programs only, water does not use these lights):
	ProgramTerrain.SendUniform("lightAmbient.color", 0.1, 0.1, 0.1);
	ProgramTerrain.SendUniform("lightDir.direction", 1.0, 0.5, 1.0);
	ProgramTerrain.SendUniform("lightDir.diffuse", 1.0, 1.0, 1.0);

	// setup materials (for basic and terrain programs only, water does not use these materials):
	ProgramTerrain.SendUniform("materialAmbient", 1.0, 1.0, 1.0);		// full power (note: ambient light is extremely dim)
	ProgramTerrain.SendUniform("materialDiffuse", 1.0, 1.0, 1.0);

	//setup the water colours and level
	ProgramWater.SendUniform("waterColor", 0.2f, 0.22f, 0.02f);
	ProgramWater.SendUniform("skyColor", 0.2f, 0.6f, 1.f);
	ProgramTerrain.SendUniform("waterColor", 0.2f, 0.22f, 0.02f);
	ProgramTerrain.SendUniform("waterLevel", waterLevel);


	Program.SendUniform("lightAmbient.color", 0.2, 0.2, 0.2);
	Program.SendUniform("lightDir.direction", 1.0, 0.5, 1.0);
	Program.SendUniform("lightDir.diffuse", 0.2, 0.2, 0.2);	// dimmed white light



	//pointlight
	Program.SendUniform("lightPoint.position", 600.0f, 200.0f, 0.0f);
	Program.SendUniform("lightPoint.diffuse", 1.0, 1.0, 1.0);
	Program.SendUniform("lightPoint.specular", 1.0, 1.0, 1.0);
	Program.SendUniform("shininess", 13.0f);

	Program.SendUniform("lightEmissive.color", 1.0, 1.0, 1.0);

	fogOn = false;

	//fog init
	Program.SendUniform("fogColour", 0.5f, 0.5f, 0.5f); 
	Program.SendUniform("fogDensity", 0.0f);



	// load Sky Box     
	if (!skybox.load("models\\TropicalSunnyDay\\TropicalSunnyDayFront1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayLeft1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayBack1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayRight1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayUp1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayDown1024.jpg")) return false;

	// load your 3D models here!
	if (!terrain.loadHeightmap("models\\try_this.png", 100.0f)) return false; //originally set to 10
	if (!terrain2.loadHeightmap("models\\try_this2.png", 90.0f)) return false; //originally set to 10
	//if (!lava.loadHeightmap("models\\try_this3.png", 90.0f)) return false; //lava
	if (!water.loadHeightmap("models\\watermap.png", 10)) return false;


	if (!player.load("models\\Standard Walk.dae")) return false;
	player.loadAnimations();

	//loading textures
	grass.Load("models\\TextureGrass\\grass.png", GL_RGBA);
	if (!grass.GetBits()) return false;

	bm_charTexColor.Load("models\\textures_anim\\Guard_02__diffuse.png", GL_RGBA);
	if (!bm_charTexColor.GetBits()) return false;

	bm_charTexNormal.Load("models\\textures_anim\\Guard_02__normal.png", GL_RGBA);
	if (!bm_charTexNormal.GetBits()) return false;
	

	bm_terrainTexNormal.Load("models\\TextureRock\\TexturesCom_Rock_CliffVolcanic_1K_normal.png", GL_RGBA);
	if (!bm_terrainTexNormal.GetBits()) return false;

	bm_pebbles.Load("models\\TextureGrass\\pebbles.png", GL_RGBA);
	if (!bm_pebbles.GetBits()) return false;

	bm_land.Load("models\\TextureGrass\\sand.png", GL_RGBA);
	if (!bm_land.GetBits()) return false;

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexGrass);
	glBindTexture(GL_TEXTURE_2D, idTexGrass);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, grass.GetWidth(), grass.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, grass.GetBits());
	

	// none (simple-white) texture
	glGenTextures(1, &idTexNone);
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	BYTE bytes[] = { 255, 255, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, &bytes);


	// Send the texture info to the shaders
	Program.SendUniform("texture0", 0);
	Program.SendUniform("textureNormal", 1);

	///// NORMAL MAPPING TEXTURE/////
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idNormalTerrain);
	glBindTexture(GL_TEXTURE_2D, idNormalTerrain);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm_terrainTexNormal.GetWidth(), bm_terrainTexNormal.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, bm_terrainTexNormal.GetBits());
	glGenerateMipmap(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idNormalChar);
	glBindTexture(GL_TEXTURE_2D, idNormalChar);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm_charTexNormal.GetWidth(), bm_charTexNormal.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, bm_charTexNormal.GetBits());
	glGenerateMipmap(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &idTexChar);
	glBindTexture(GL_TEXTURE_2D, idTexChar);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm_charTexColor.GetWidth(), bm_charTexColor.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, bm_charTexColor.GetBits());

	ProgramTerrain.Use();
	// setup water multitexturing
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &idTexPeb);
	glBindTexture(GL_TEXTURE_2D, idTexPeb);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm_pebbles.GetWidth(), bm_pebbles.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, bm_pebbles.GetBits());
	ProgramTerrain.SendUniform("textureBed", 1);

	
	glActiveTexture(GL_TEXTURE2);
	glGenTextures(1, &idTexLand);
	glBindTexture(GL_TEXTURE_2D, idTexLand);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm_land.GetWidth(), bm_land.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, bm_land.GetBits());
	ProgramTerrain.SendUniform("textureShore", 2);
	
	
	// Initialise the View Matrix (initial position of the camera)
	matrixView = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));
	matrixView *= lookAt(
		vec3(-160.0, 10.0, -220.0), //position
		vec3(4.0, 10.0, 30.0),  //look at
		vec3(0.0, 1.0, 0.0)); //direction, where's the top of the object


	// Prepare the particle buffers
	std::vector<float>bufferVelocity;
	std::vector<float>bufferStartTime;
	float time = 0;
	for (int i = 0; i < NPARTICLES; i++)
	{
		float theta = (float)M_PI / 1.5f * (float)rand() / (float)RAND_MAX;
		float phi = (float)M_PI * 2.f * (float)rand() / (float)RAND_MAX;
		float x = sin(theta) * cos(phi);
		float y = cos(theta);
		float z = sin(theta) * sin(phi);
		float v = 2 + 0.5f * (float)rand() / (float)RAND_MAX;

		bufferVelocity.push_back(x * v);
		bufferVelocity.push_back(y * v);
		bufferVelocity.push_back(z * v);

		bufferStartTime.push_back(time);
		time += PERIOD;
	}
	glGenBuffers(1, &idBufferVelocity);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferVelocity.size(), &bufferVelocity[0],
		GL_STATIC_DRAW);
	glGenBuffers(1, &idBufferStartTime);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferStartTime.size(), &bufferStartTime[0],
		GL_STATIC_DRAW);



	// setup the screen background colour
	//glClearColor(0.2f, 0.6f, 1.f, 1.0f);   // blue sky background
	glClearColor(1.0f, 0.0f, 0.f, 1.0f);   // blue sky background
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << endl;

	return true;
}

// Helper function used to display player coordinates (X-Z) on-screen
void displayCoords(float x, float z)
{
	char buf[100];
	snprintf(buf, sizeof(buf), "(%1.1f, %1.1f)", x, z);
	Program.SendUniform("Text", 1);
	glWindowPos2i(10, 10);  // move in 10 pixels from the left and bottom edges
	for (char* p = buf; *p; p++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
	Program.SendUniform("Text", 0);
}

void done()
{
}

void renderScene(mat4 &matrixView, float time)
{
	mat4 m;
	Program.SendUniform("matrixView", matrixView); //ESSENTIAL FOR DIRECTIONAL LIGHT
	Program.SendUniform("materialEmissive", 0.0, 0.0, 0.0); //Avoid Emissive Light from all the objects but the bulbs
	Program.SendUniform("useNormalMap", false); //start without normal map

	// RENDER THE PARTICLE SYSTEM
	ProgramParticle.Use();
	ProgramParticle.SendUniform("time", time);

	m = matrixView;
	//m = rotate(m, radians(90.f), vec3(0.0f, 0.0f, 1.0f));
	//m = translate(m, vec3(-160.0, 15.0, -220.0));
	//m = scale(m, vec3(10.0f, 10.0f, 10.0f));
	ProgramParticle.SendUniform("matrixModelView", m);
	
	// render the buffer
	glEnableVertexAttribArray(0);	// velocity
	glEnableVertexAttribArray(1);	// start time
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_POINTS, 0, NPARTICLES);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glPointSize(50);
	

	if (fogOn) 
	{
		Program.SendUniform("fogDensity", 0.05f);
	}
	else 
	{
		Program.SendUniform("fogDensity", 0.0f);
	}
	if (!fogOn) 
	{
		// render the skybox
		Program.SendUniform("lightAmbient.color", 1.0, 1.0, 1.0);
		Program.SendUniform("materialAmbient", 1.0, 1.0, 1.0);
		Program.SendUniform("materialDiffuse", 0.0, 0.0, 0.0);
		Program.SendUniform("materialSpecular", 0.0, 0.0, 0.0);
		m = matrixView;
		skybox.render(m);
		Program.SendUniform("lightAmbient.color", 0.2, 0.2, 0.2);

	}

	// render the water
	ProgramWater.Use();
	m = matrixView;
	m = translate(m, vec3(0, waterLevel, -200));
	m = scale(m, vec3(0.5f, 1.0f, 0.5f));
	ProgramWater.SendUniform("matrixModelView", m);
	water.render(m);

	ProgramTerrain.Use();
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, idNormalTerrain);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	Program.SendUniform("useNormalMap", true);

	/*glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, idTexPeb);
	glBindTexture(GL_TEXTURE_2D, idTexLand);
	ProgramTerrain.SendUniform("textureBed", 1);
	ProgramTerrain.SendUniform("textureShore", 2);*/

	// render the terrain
	Program.SendUniform("materialAmbient", 1.0, 1.0, 1.0);
	Program.SendUniform("materialDiffuse", 0.5, 0.2, 0.0);
	Program.SendUniform("materialSpecular", 0.7, 0.2, 0.2);
	m = translate(matrixView, vec3(0, 0, 0));
	terrain.render(m);
	
	Program.SendUniform("useNormalMap", false);

	
	// render the terrain2 (green grass textured)
	glBindTexture(GL_TEXTURE_2D, idTexGrass);
	Program.SendUniform("materialAmbient", 0.0, 1.0, 0.0);
	Program.SendUniform("materialDiffuse", 0.1, 0.8, 0.1);
	Program.SendUniform("materialSpecular", 0.0, 1.0, 0.1);
	m = translate(matrixView, vec3(0, -5, 0));
	terrain2.render(m);


	
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	Program.SendUniform("materialAmbient", 1.0, 1.0, 1.0);
	Program.SendUniform("materialDiffuse", 1.0, 1.0, 1.0);
	Program.SendUniform("materialSpecular", 1.0, 1.0, 1.0);
	Program.SendUniform("materialEmissive", 15.0, 15.0, 15.7);
	m = matrixView;
	m = translate(m, vec3(600.0f, 200.0f, 0.0f));
	Program.SendUniform("matrixModelView", m);
	glutSolidSphere(60, 32, 32);
	Program.SendUniform("lightPoint.matrix", m);


	//render player 
	// calculate and send bone transforms
	std::vector<float> transforms;
	player.getAnimData(0, time, transforms);
	Program.SendUniformMatrixv("bones", (float*)&transforms[0], transforms.size() / 16);
	Program.SendUniform("materialEmissive", 0.0, 0.0, 0.0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, idNormalChar);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexChar);
	Program.SendUniform("useNormalMap", true);
	m = matrixView;
	m = translate(m, vec3(-202, 20, -250));
	m = scale(m, vec3(5.0f, 5.0f, 5.0f));
	player.render(m);
	Program.SendUniform("useNormalMap", false);

	

	
	// Display debug information: your current coordinates (x, z)
// These coordinates are available as inv[3].x, inv[3].z
	mat4 inv = inverse(matrixView);
	displayCoords(inv[3].x, inv[3].z);

}

void onRender()
{
	// this global variable controls the animation
	float time = glutGet(GLUT_ELAPSED_TIME) * 0.001f;

	ProgramWater.SendUniform("t", glutGet(GLUT_ELAPSED_TIME) / 1000.f);

	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// setup the View Matrix (camera)
	mat4 m = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));// switch tilt off
	m = translate(m, cam);												// animate camera motion (controlled by WASD keys)
	m = rotate(m, radians(-angleTilt), vec3(1.f, 0.f, 0.f));			// switch tilt on
	matrixView = m * matrixView;

	// move the camera up following the profile of terrain (Y coordinate of the terrain)
	float terrainY = -terrain.getInterpolatedHeight(inverse(matrixView)[3][0], inverse(matrixView)[3][2]);
	matrixView = translate(matrixView, vec3(0, terrainY, 0));

	// render the scene objects
	renderScene(matrixView, time);

	// the camera must be moved down by terrainY to avoid unwanted effects
	matrixView = translate(matrixView, vec3(0, -terrainY, 0));

	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();

	Program.SendUniform("matrixModelView", m);
	ProgramWater.SendUniform("matrixView", matrixView);
	ProgramTerrain.SendUniform("matrixView", matrixView);

}

// called before window opened or resized - to setup the Projection Matrix
void onReshape(int w, int h)
{
	float ratio = w * 1.0f / h;      // we hope that h is not zero
	glViewport(0, 0, w, h);
	mat4 matrixProjection = perspective(radians(60.f), ratio, 0.02f, 1000.f);

	// Setup the Projection Matrix
	Program.SendUniform("matrixProjection", matrixProjection);
	ProgramWater.SendUniform("matrixProjection", matrixProjection);
	ProgramTerrain.SendUniform("matrixProjection", matrixProjection);
	ProgramParticle.SendUniform("matrixProjection", matrixProjection);
}

// Handle WASDQE keys
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': cam.z = std::max(cam.z * 1.05f, 0.01f); break;
	case 's': cam.z = std::min(cam.z * 1.05f, -0.01f); break;
	case 'a': cam.x = std::max(cam.x * 1.05f, 0.01f); break;
	case 'd': cam.x = std::min(cam.x * 1.05f, -0.01f); break;
	case 'e': cam.y = std::max(cam.y * 1.05f, 0.01f); break;
	case 'q': cam.y = std::min(cam.y * 1.05f, -0.01f); break;

	case '1':
		if (fogOn)
		{
			//fog off
			fogOn = false;
			glClearColor(0.2f, 0.6f, 1.f, 1.0f);   // blue sky background
			break;
		}
		else
		{
			//fog on
			fogOn = true;
			glClearColor(0.5f, 0.5f, 0.5f, 1.0f);   // blue sky background
			break;
		}
	}
	// speed limit
	cam.x = std::max(-0.15f, std::min(0.15f, cam.x));
	cam.y = std::max(-0.15f, std::min(0.15f, cam.y));
	cam.z = std::max(-0.15f, std::min(0.15f, cam.z));
}

// Handle WASDQE keys (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	case 's': cam.z = 0; break;
	case 'a':
	case 'd': cam.x = 0; break;
	case 'q':
	case 'e': cam.y = 0; break;
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

// Handle mouse click
bool bJustClicked = false;
void onMouse(int button, int state, int x, int y)
{
	bJustClicked = (state == GLUT_DOWN);
	glutSetCursor(bJustClicked ? GLUT_CURSOR_CROSSHAIR : GLUT_CURSOR_INHERIT);
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
}

// handle mouse move
void onMotion(int x, int y)
{
	if (bJustClicked)
		bJustClicked = false;
	else
	{
		glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);

		// find delta (change to) pan & tilt
		float deltaPan = 0.25f * (x - glutGet(GLUT_WINDOW_WIDTH) / 2);
		float deltaTilt = 0.25f * (y - glutGet(GLUT_WINDOW_HEIGHT) / 2);

		// View = Tilt * DeltaPan * Tilt^-1 * DeltaTilt * View;
		angleTilt += deltaTilt;
		mat4 m = mat4(1.f);
		m = rotate(m, radians(angleTilt), vec3(1.f, 0.f, 0.f));
		m = rotate(m, radians(deltaPan), vec3(0.f, 1.f, 0.f));
		m = rotate(m, radians(-angleTilt), vec3(1.f, 0.f, 0.f));
		m = rotate(m, radians(deltaTilt), vec3(1.f, 0.f, 0.f));
		matrixView = m * matrixView;
	}
}

int main(int argc, char **argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Level_2 - Outdoor Scene");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cerr << "GLEW Error: " << glewGetErrorString(err) << endl;
		return 0;
	}
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;

	// register callbacks
	glutDisplayFunc(onRender);
	glutReshapeFunc(onReshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);

	cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Version: " << glGetString(GL_VERSION) << endl;

	// init light and everything – not a GLUT or callback function!
	if (!init())
	{
		cerr << "Application failed to initialise" << endl;
		return 0;
	}

	// enter GLUT event processing cycle
	glutMainLoop();

	done();

	return 1;
}

