#define _USE_MATH_DEFINES

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "..\src\GLSLShader.h"
#include <vector>
#include "Obj.h"

#include <SOIL.h>

#include <cstdlib>

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "SOIL.lib")

using namespace std;

//output screen resolution
const int WIDTH  = 1280;
const int HEIGHT = 960;

//offscreen texture size
const int RTT_WIDTH = WIDTH/4;
const int RTT_HEIGHT = HEIGHT/4;

//shaders for use in the recipe
GLSLShader	shader,
			flatShader,
			finalShader,
			ssaoFirstShader,
			ssaoSecondShader,
			gaussianH_shader,
			gaussianV_shader;

//IDs for vertex array and buffer object
GLuint vaoID;
GLuint vboVerticesID;
GLuint vboIndicesID;

//projection and modelview matrices
glm::mat4  P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);

//Objloader instance
ObjLoader obj;
vector<Mesh*> meshes;			//all meshes 
vector<Material*> materials;	//all materials 
vector<unsigned short> indices;	//all mesh indices 
vector<Vertex> vertices;		//all mesh vertices  
vector<GLuint> textures;		//all textures

//light crosshair gizmo vetex array and buffer object IDs
GLuint lightVAOID;
GLuint lightVerticesVBO;
glm::vec3 lightPosOS=glm::vec3(0,2,0); //objectspace light position

//spherical cooridate variables for light rotation
float theta = 1.6f;
float phi = -0.6f;
float radius = 70;

//camera transformation variables
int state = 0, oldX=0, oldY=0;
float rX=42, rY=180, dist = -80;

//OBJ mesh filename to load
const std::string mesh_filename = "../media/blocks.obj";

//FBO ids for normal and filtering FBO
GLuint fboID, filterFBOID;
//colour and depth attachment texture IDs
GLuint normalTextureID, depthTextureID;
//filtered texture colour attachment IDs
GLuint blurTexID[2];

//quad vertex array and vertex buffer object IDs
GLuint quadVAOID;
GLuint quadVBOID;
GLuint quadIndicesID;

//noise texture ID
GLuint noiseTexID;

//sampling radius for SSAO
float sampling_radius = 0.25f;
//flag to enable/disable SSAO
bool bUseSSAO = true;

//initialization of FBOs
void InitFBO() {
	//setup offscreen rendering fbo
	glGenFramebuffers(1, &fboID);
	glBindFramebuffer(GL_FRAMEBUFFER, fboID);

	//generate one colour attachment and one depth attachment texture
	glGenTextures(1, &normalTextureID);
	glGenTextures(1, &depthTextureID);

	//bind colour attachment to texture unit 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalTextureID);

	//set texture parameters
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, RTT_WIDTH, RTT_HEIGHT, 0, GL_BGRA, GL_FLOAT, NULL);

	//bind depth attachment to texture unit 3
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthTextureID);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, RTT_WIDTH, RTT_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	//bind the colour and depth attachments to FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, normalTextureID, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D, depthTextureID, 0);

	//check FBO completeness status
	GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if(status==GL_FRAMEBUFFER_COMPLETE) {
		printf("FBO setup succeeded.");
	} else {
		printf("Error in FBO setup.");
	}

	//setup filtering fbo
	glGenFramebuffers(1,&filterFBOID);
	glBindFramebuffer(GL_FRAMEBUFFER,filterFBOID);

	//set two colour attachment textures for filtering
	glGenTextures(2, blurTexID);
	for(int i=0;i<2;i++) {
		glActiveTexture(GL_TEXTURE4+i);
		glBindTexture(GL_TEXTURE_2D, blurTexID[i]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA32F,RTT_WIDTH,RTT_HEIGHT,0,GL_RGBA,GL_FLOAT,NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0+i,GL_TEXTURE_2D,blurTexID[i],0);
	}

	//check the framebuffer completeness status
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status == GL_FRAMEBUFFER_COMPLETE) {
		cout<<"\nFiltering FBO setup successful."<<endl;
	} else {
		cout<<"Problem in Filtering FBO setup."<<endl;
	}

	//bind texture unit 0 as active texture since it will be used for loading 
	//of model textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//unbind the FBO 
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//release all FBO releated resources
void ShutdownFBO() {
	glDeleteTextures(2, blurTexID);
	glDeleteTextures(1, &normalTextureID);
	glDeleteTextures(1, &depthTextureID);
	glDeleteFramebuffers(1, &fboID);
	glDeleteFramebuffers(1, &filterFBOID);
}

//mouse clock handler
void OnMouseDown(int button, int s, int x, int y)
{
	if (s == GLUT_DOWN)
	{
		oldX = x;
		oldY = y;
	}

	if(button == GLUT_MIDDLE_BUTTON)
		state = 0;
	else if(button == GLUT_RIGHT_BUTTON)
		state = 2;
	else
		state = 1;
}

//mouse move handler
void OnMouseMove(int x, int y)
{
	if (state == 0)
		dist *= (1 + (y - oldY)/60.0f);
	else if(state ==2) {
		theta += (oldX - x)/60.0f;
		phi += (y - oldY)/60.0f;

		//update the light position
		lightPosOS.x = radius * cos(theta)*sin(phi);
		lightPosOS.y = radius * cos(phi);
		lightPosOS.z = radius * sin(theta)*sin(phi);

	} else {
		rY += (x - oldX)/5.0f;
		rX += (y - oldY)/5.0f;
	}
	oldX = x;
	oldY = y;

	glutPostRedisplay();
}

//OpenGL initialization function
void OnInit() {

	//generate the pseudorandom noise data
	glm::vec4 pData[64][64];
	for(int j=0;j<64;j++) {
		for(int i=0;i<64;i++) {
			pData[i][j].x = (float)rand() / RAND_MAX;
			pData[i][j].y = (float)rand() / RAND_MAX;
			pData[i][j].z = (float)rand() / RAND_MAX;
			pData[i][j].w = (float)rand() / RAND_MAX;
		}
	}

	//use the pseudorandom noise data to generate a 64x64 nosie texture
	glGenTextures(1, &noiseTexID);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, noiseTexID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 64, 64, 0, GL_BGRA, GL_FLOAT, pData);

	//get the mesh path for loading of textures
	std::string mesh_path = mesh_filename.substr(0, mesh_filename.find_last_of("/")+1);

	//load the obj model
	if(!obj.Load(mesh_filename.c_str(), meshes, vertices, indices, materials)) {
		cout<<"Cannot load the Obj mesh"<<endl;
		exit(EXIT_FAILURE);
	}
	GL_CHECK_ERRORS

	//bind texture 0 as active texture unit
	glActiveTexture(GL_TEXTURE0);
	//load material textures
	for(size_t k=0;k<materials.size();k++) {
		//if the diffuse texture name is not empty
		if(materials[k]->map_Kd != "") {
			GLuint id = 0;

			//generate a new OpenGL texture
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_2D, id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			int texture_width = 0, texture_height = 0, channels=0;

			const string& filename =  materials[k]->map_Kd;

			std::string full_filename = mesh_path;
			full_filename.append(filename);

			//use SOIL to load the texture
			GLubyte* pData = SOIL_load_image(full_filename.c_str(), &texture_width, &texture_height, &channels, SOIL_LOAD_AUTO);
			if(pData == NULL) {
				cerr<<"Cannot load image: "<<full_filename.c_str()<<endl;
				exit(EXIT_FAILURE);
			}

			//Flip the image on Y axis
			int i,j;
			for( j = 0; j*2 < texture_height; ++j )
			{
				int index1 = j * texture_width * channels;
				int index2 = (texture_height - 1 - j) * texture_width * channels;
				for( i = texture_width * channels; i > 0; --i )
				{
					GLubyte temp = pData[index1];
					pData[index1] = pData[index2];
					pData[index2] = temp;
					++index1;
					++index2;
				}
			}
			//get the image format
			GLenum format = GL_RGBA;
			switch(channels) {
				case 2:	format = GL_RG32UI; break;
				case 3: format = GL_RGB;	break;
				case 4: format = GL_RGBA;	break;
			}
			//allocate the texture
			glTexImage2D(GL_TEXTURE_2D, 0, format, texture_width, texture_height, 0, format, GL_UNSIGNED_BYTE, pData);

			//release the SOIL image data
			SOIL_free_image_data(pData);

			//add the texture id to a vector
			textures.push_back(id);
		}
	}
	GL_CHECK_ERRORS

	//setup flat shader
	flatShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/flat.vert");
	flatShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/flat.frag");
	//compile and link shader
	flatShader.CreateAndLinkProgram();
	flatShader.Use();
		//add attribute and uniform
		flatShader.AddAttribute("vVertex");
		flatShader.AddUniform("MVP");
	flatShader.UnUse();

	//load final shader
	finalShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/Passthrough.vert");
	finalShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/final.frag");
	//compile and link shader
	finalShader.CreateAndLinkProgram();
	finalShader.Use();
		//add attribute and uniform
		finalShader.AddAttribute("vVertex");
		finalShader.AddUniform("MVP");
		finalShader.AddUniform("textureMap");
		//set values of constant uniforms as initialization
		glUniform1i(finalShader("textureMap"), 4);
	finalShader.UnUse();

	//load the point light rendering shader
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/shader.frag");
	shader.CreateAndLinkProgram();
	shader.Use();
		//add attribute and uniform
		shader.AddAttribute("vVertex");
		shader.AddAttribute("vNormal");
		shader.AddAttribute("vUV");

		shader.AddUniform("MV");
		shader.AddUniform("N");
		shader.AddUniform("P");
		shader.AddUniform("textureMap");
		shader.AddUniform("useDefault");

		shader.AddUniform("light_position");
		shader.AddUniform("diffuse_color");
		//set values of constant uniforms as initialization
		glUniform1i(shader("textureMap"), 0);
	shader.UnUse();

	//load the horizontal Gaussian blurring shader
	gaussianH_shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/Passthrough.vert");
	gaussianH_shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/GaussH.frag");

	//compile and link shader
	gaussianH_shader.CreateAndLinkProgram();
	gaussianH_shader.Use();
		//add attribute and uniform
		gaussianH_shader.AddAttribute("vVertex");
		gaussianH_shader.AddUniform("textureMap");
		//set values of constant uniforms as initialization
		glUniform1i(gaussianH_shader("textureMap"),5);
	gaussianH_shader.UnUse();

	//load the vertical Gaussian blurring shader
	gaussianV_shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/Passthrough.vert");
	gaussianV_shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/GaussV.frag");

	//compile and link shader
	gaussianV_shader.CreateAndLinkProgram();
	gaussianV_shader.Use();
		//add attribute and uniform
		gaussianV_shader.AddAttribute("vVertex");
		gaussianV_shader.AddUniform("textureMap");
		//set values of constant uniforms as initialization
		glUniform1i(gaussianV_shader("textureMap"),4);
	gaussianV_shader.UnUse();

	//load the first step SSAO shader
	ssaoFirstShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/SSAO_FirstStep.vert");
	ssaoFirstShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/SSAO_FirstStep.frag");
	//compile and link shader
	ssaoFirstShader.CreateAndLinkProgram();
	ssaoFirstShader.Use();
		//add attribute and uniform
		ssaoFirstShader.AddAttribute("vVertex");
		ssaoFirstShader.AddAttribute("vNormal");
		ssaoFirstShader.AddUniform("MVP");
		ssaoFirstShader.AddUniform("N");
	ssaoFirstShader.UnUse();

	//load the second step SSAO shader
	ssaoSecondShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/Passthrough.vert");
	ssaoSecondShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/SSAO_SecondStep.frag");
	//compile and link shader
	ssaoSecondShader.CreateAndLinkProgram();
	ssaoSecondShader.Use();
		//add attribute and uniform
		ssaoSecondShader.AddAttribute("vVertex");
		ssaoSecondShader.AddUniform("samples");
		ssaoSecondShader.AddUniform("invP");
		ssaoSecondShader.AddUniform("normalTex");
		ssaoSecondShader.AddUniform("depthTex");
		ssaoSecondShader.AddUniform("noiseTex");
		ssaoSecondShader.AddUniform("radius");
		ssaoSecondShader.AddUniform("viewportSize");
		ssaoSecondShader.AddUniform("invViewportSize");

		//set values of constant uniforms as initialization
		glUniform2f(ssaoSecondShader("viewportSize"), float(RTT_WIDTH), float(RTT_HEIGHT));
		glUniform2f(ssaoSecondShader("invViewportSize"), 1.0f/float(RTT_WIDTH), 1.0f/float(RTT_HEIGHT));
		glUniform1i(ssaoSecondShader("normalTex"),1);
		glUniform1i(ssaoSecondShader("noiseTex"),2);
		glUniform1i(ssaoSecondShader("depthTex"),3);

		glm::mat4 biasMat;
		biasMat = glm::translate(glm::mat4(1),glm::vec3(0.5,0.5,0.5));
		biasMat = glm::scale(biasMat, glm::vec3(0.5,0.5,0.5));
		glm::mat4 invP = biasMat*glm::inverse(P);
		glUniformMatrix4fv(ssaoSecondShader("invP"), 1, GL_FALSE, glm::value_ptr(invP));

		glm::vec2 samples[16];
		float angle = (float)M_PI_4;
		for(int i=0;i<16;i++) {
			samples[i].x = cos(angle) * (float)(i+1)/16.0f;
			samples[i].y = sin(angle) * (float)(i+1)/16.0f;
			angle += (float)M_PI_2;
			if(((i + 1) % 4) == 0)
				angle += (float)M_PI_4;
		}
		glUniform2fv(ssaoSecondShader("samples"), 16, &(samples[0].x));
	ssaoSecondShader.UnUse();

	GL_CHECK_ERRORS


	//setup the vertex array object and vertex buffer object for the mesh
	//geometry handling 
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	glGenBuffers(1, &vboIndicesID);

	glBindVertexArray(vaoID);
		glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
		//pass mesh vertices
		glBufferData (GL_ARRAY_BUFFER, sizeof(Vertex)*vertices.size(), &(vertices[0].pos.x), GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//enable vertex attribute array for vertex position
		glEnableVertexAttribArray(shader["vVertex"]);
		glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE,sizeof(Vertex),0);
		GL_CHECK_ERRORS
		//enable vertex attribute array for vertex normal
		glEnableVertexAttribArray(shader["vNormal"]);
		glVertexAttribPointer(shader["vNormal"], 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, normal)) );

		GL_CHECK_ERRORS
		//enable vertex attribute array for vertex texture coordinates
		glEnableVertexAttribArray(shader["vUV"]);
		glVertexAttribPointer(shader["vUV"], 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, uv)) );

		//if we have a single material, it means the 3ds model contains one mesh
		//we therefore load it into an element array buffer
		if(materials.size()==1) {
			//pass indices to the element array buffer if there is a single material
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*indices.size(), &(indices[0]), GL_STATIC_DRAW);
		}
		GL_CHECK_ERRORS

	//setup fullscreen quad vertices
	glm::vec2 quadVerts[4];
	quadVerts[0] = glm::vec2(0,0);
	quadVerts[1] = glm::vec2(1,0);
	quadVerts[2] = glm::vec2(1,1);
	quadVerts[3] = glm::vec2(0,1);

	//setup quad indices
	GLushort quadIndices[]={ 0,1,2,0,2,3};

	//setup quad vertex array and vertex buffer objects
	glGenVertexArrays(1, &quadVAOID);
	glGenBuffers(1, &quadVBOID);
	glGenBuffers(1, &quadIndicesID);

	glBindVertexArray(quadVAOID);
		glBindBuffer (GL_ARRAY_BUFFER, quadVBOID);
		//pass quad vertices to vertex buffer object
		glBufferData (GL_ARRAY_BUFFER, sizeof(quadVerts), &quadVerts[0], GL_STATIC_DRAW);

		GL_CHECK_ERRORS
		//enable vertex attribute array for vertex position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,0,0);
		//pass quad indices to element array buffer
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, quadIndicesID);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), &quadIndices[0], GL_STATIC_DRAW);


	//glBindVertexArray(0);

	//setup vao and vbo stuff for the light position crosshair
	glm::vec3 crossHairVertices[6];
	crossHairVertices[0] = glm::vec3(-0.5f,0,0);
	crossHairVertices[1] = glm::vec3(0.5f,0,0);
	crossHairVertices[2] = glm::vec3(0, -0.5f,0);
	crossHairVertices[3] = glm::vec3(0, 0.5f,0);
	crossHairVertices[4] = glm::vec3(0,0, -0.5f);
	crossHairVertices[5] = glm::vec3(0,0, 0.5f);

	//setup light gizmo vertex array and vertex buffer object IDs
	glGenVertexArrays(1, &lightVAOID);
	glGenBuffers(1, &lightVerticesVBO);
	glBindVertexArray(lightVAOID);

	glBindBuffer (GL_ARRAY_BUFFER, lightVerticesVBO);
	//pass crosshair vertices to the buffer object
	glBufferData (GL_ARRAY_BUFFER, sizeof(crossHairVertices), &(crossHairVertices[0].x), GL_STATIC_DRAW);
	GL_CHECK_ERRORS
	//enable vertex attribute array for vertex position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0,0);

	GL_CHECK_ERRORS

	//use spherical coordinates to get the light position
	lightPosOS.x = radius * cos(theta)*sin(phi);
	lightPosOS.y = radius * cos(phi);
	lightPosOS.z = radius * sin(theta)*sin(phi);

	//enable depth test and culling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	//set clear colour to corn blue
	glClearColor(0.5,0.5,1,1);

	//initializa FBO
	InitFBO();
	cout<<"Initialization successfull"<<endl;
}

//release al allocated resources
void OnShutdown() {
	ShutdownFBO();

	//delete all textures
	size_t total_textures = textures.size();
	for(size_t i=0;i<total_textures;i++) {
		glDeleteTextures(1, &textures[i]);
	}
	textures.clear();

	//delete all meshes
	size_t total_meshes = meshes.size();
	for(size_t i=0;i<total_meshes;i++) {
		delete meshes[i];
		meshes[i]=0;
	}
	meshes.clear();

	size_t total_materials = materials.size();
	for( size_t i=0;i<total_materials;i++) {
		delete materials[i];
		materials[i] = 0;
	}
	materials.clear();

	//Delete textures
	glDeleteTextures(1, &noiseTexID);

	//Destroy shader
	shader.DeleteShaderProgram();
	ssaoFirstShader.DeleteShaderProgram();
	ssaoSecondShader.DeleteShaderProgram();
	gaussianH_shader.DeleteShaderProgram();
	gaussianV_shader.DeleteShaderProgram();
	finalShader.DeleteShaderProgram();
	flatShader.DeleteShaderProgram();

	//Destroy vao and vbo
	glDeleteBuffers(1, &vboVerticesID);
	glDeleteBuffers(1, &vboIndicesID);
	glDeleteVertexArrays(1, &vaoID);

	glDeleteVertexArrays(1, &quadVAOID);
	glDeleteBuffers(1, &quadVBOID);
	glDeleteBuffers(1, &quadIndicesID);

	glDeleteVertexArrays(1, &lightVAOID);
	glDeleteBuffers(1, &lightVerticesVBO);
	cout<<"Shutdown successfull"<<endl;
}

//resize event handler
void OnResize(int w, int h) {
	//set the viewport
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//setup the projection matrix
	P = glm::perspective(60.0f,(float)w/h, 0.1f,1000.0f);
}

//display callback function
void OnRender() {
	//clear the colour and depth buffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//set the viewing transformation
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));
	
	//bind the mesh vertex array object
	glBindVertexArray(vaoID); {
		//bind the mesh shader
		shader.Use();
			//pass the shader uniforms
			glUniformMatrix4fv(shader("MV"), 1, GL_FALSE, glm::value_ptr(MV));
			glUniformMatrix3fv(shader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));
			glUniformMatrix4fv(shader("P"), 1, GL_FALSE, glm::value_ptr(P));
			glUniform3fv(shader("light_position"),1, &(lightPosOS.x));
			//loop through all materials
			for(size_t i=0;i<materials.size();i++) {
				Material* pMat = materials[i];
				//if material texture filename is not empty
				if(pMat->map_Kd !="") {
					glUniform1f(shader("useDefault"), 0.0);

					//get the currently bound texture and check if the current texture ID
					//is not equal, if so bind the new texture
					GLint whichID[1];
					glGetIntegerv(GL_TEXTURE_BINDING_2D, whichID);
					if(whichID[0] != textures[i]) {
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, textures[i]);
					}
				} else
					//otherwise we have no texture, we use a default colour
					glUniform1f(shader("useDefault"), 1.0);

				//if we have a single material, we render the whole mesh in a single call
				if(materials.size()==1)
					glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0);
				else
					//otherwise we render the submesh
					glDrawElements(GL_TRIANGLES, pMat->count, GL_UNSIGNED_SHORT, (const GLvoid*)(&indices[pMat->offset]));
			}
		//unbind the shader
		shader.UnUse();
	}

	//if SSAO is enabled
	if(bUseSSAO) {
		//bind the FBO
		glBindFramebuffer(GL_FRAMEBUFFER, fboID);
		//set the viewport to the size of the offscreen render target
		glViewport(0,0,RTT_WIDTH, RTT_HEIGHT);
		//set the colour attachment 0 ad the draw buffer
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
			//clear the colour and depth buffers
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

			//bind the mesh vertex array object
			glBindVertexArray(vaoID); {
			//bind the shader 
			ssaoFirstShader.Use();
				//set the shader uniforms
				glUniformMatrix4fv(ssaoFirstShader("MVP"), 1, GL_FALSE, glm::value_ptr(P*MV));
				glUniformMatrix3fv(ssaoFirstShader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));
				//loop through all materials
				for(size_t i=0;i<materials.size();i++) {
					Material* pMat = materials[i];
					//if we have a single material, we render the whole mesh in a single call
					if(materials.size()==1)
						glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0);
					else
						//otherwise we render the submesh
						glDrawElements(GL_TRIANGLES, pMat->count, GL_UNSIGNED_SHORT, (const GLvoid*)(&indices[pMat->offset]));
				}
			//unbind the first step shader			
			ssaoFirstShader.UnUse();
		} 

	 	GL_CHECK_ERRORS

		//set the filtering FBO
		glBindFramebuffer(GL_FRAMEBUFFER,filterFBOID);
		//set colour attachment 0 as the draw buffer
		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		//bind the fullscreen quad vertex array object
		glBindVertexArray(quadVAOID);
			//bind the second step SSAO shader
			ssaoSecondShader.Use();
				//set shader uniforms
				glUniform1f(ssaoSecondShader("radius"), sampling_radius);
				//draw fullscreen quad
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
			//unbind the second step SSAO shader
			ssaoSecondShader.UnUse();

		GL_CHECK_ERRORS

		//set colour attachment 1 as the draw buffer
		glDrawBuffer(GL_COLOR_ATTACHMENT1);

		//render full screen quad again with the vertical Gaussian smoothing shader
		glBindVertexArray(quadVAOID);
			gaussianV_shader.Use();
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

		//set colour attachment 2 as the draw buffer and render full screen quad 
		//again with the horizontal Gaussian smoothing shader
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
			gaussianH_shader.Use();
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

		//unbind FBO, restore the defaul viewport and draw buffer
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		glViewport(0,0,WIDTH, HEIGHT);
		glDrawBuffer(GL_BACK_LEFT);

		//now draw the final filtered SSAO result with blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		finalShader.Use();
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
		finalShader.UnUse();

		//disable blending
		glDisable(GL_BLEND);
	}

	//disable depth test
	glDisable(GL_DEPTH_TEST);

	//draw the light gizmo
	glBindVertexArray(lightVAOID); {
		//set the modelling transform for the light crosshair gizmo
		glm::mat4 T = glm::translate(glm::mat4(1), lightPosOS);
		//bind the shader
		flatShader.Use();
			//set shader uniforms and draw lines
			glUniformMatrix4fv(flatShader("MVP"), 1, GL_FALSE, glm::value_ptr(P*MV*T));
				glDrawArrays(GL_LINES, 0, 6);
		//unbind the shader
		flatShader.UnUse();
	}
	//enable depth test
	glEnable(GL_DEPTH_TEST);

	//swap front and back buffers to show the rendered result
	glutSwapBuffers();
}

//mouse wheel callback to move the light source on mouse wheel scroll event
void OnMouseWheel(int button, int dir, int x, int y) {

	if (dir > 0)
    {
        radius += 0.1f;
    }
    else
    {
        radius -= 0.1f;
    }

	//update the light position
	lightPosOS.x = radius * cos(theta)*sin(phi);
	lightPosOS.y = radius * cos(phi);
	lightPosOS.z = radius * sin(theta)*sin(phi);

	//recall display function
	glutPostRedisplay();
}

//keyboard event handler
void OnKey(unsigned char k, int x, int y) {
	switch(k) {
		case '-': sampling_radius-=0.01f; break;
		case '+': sampling_radius+=0.01f; break;
		case ' ': bUseSSAO = !bUseSSAO; break;
	}
	sampling_radius = min(5.0f,max(0.0f,sampling_radius));
	std::cout<<"rad: "<<sampling_radius<<std::endl;
	glutPostRedisplay();
}

int main(int argc, char** argv) {
	//freeglut initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Screen space ambient occludion (SSAO) - OpenGL 3.3");

	//initialize glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)	{
		cerr<<"Error: "<<glewGetErrorString(err)<<endl;
	} else {
		if (GLEW_VERSION_3_3)
		{
			cout<<"Driver supports OpenGL 3.3\nDetails:"<<endl;
		}
	}
	err = glGetError(); //this is to ignore INVALID ENUM error 1282
	GL_CHECK_ERRORS

	//output hardware information
	cout<<"\tUsing GLEW "<<glewGetString(GLEW_VERSION)<<endl;
	cout<<"\tVendor: "<<glGetString (GL_VENDOR)<<endl;
	cout<<"\tRenderer: "<<glGetString (GL_RENDERER)<<endl;
	cout<<"\tVersion: "<<glGetString (GL_VERSION)<<endl;
	cout<<"\tGLSL: "<<glGetString (GL_SHADING_LANGUAGE_VERSION)<<endl;
	
	GL_CHECK_ERRORS

	//OpenGL initialization
	OnInit();

	//callback hooks
	glutCloseFunc(OnShutdown);
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize);
	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);
	glutMouseWheelFunc(OnMouseWheel);
	glutKeyboardFunc(OnKey);

	//main loop call
	glutMainLoop();

	return 0;
}
