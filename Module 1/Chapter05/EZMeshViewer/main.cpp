#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "..\src\GLSLShader.h"
#include <vector>
#include "Ezm.h"

#include <SOIL.h>

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "SOIL.lib")

using namespace std;

//screen size
const int WIDTH  = 1280;
const int HEIGHT = 960;

//shaders for mesh drawing and light gizmo
GLSLShader shader, flatShader;

//vertex arrray and buffer object ids
GLuint vaoID;
GLuint vboVerticesID; 
GLuint vboIndicesID;

//projection and modelview matrices
glm::mat4  P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);

//position of light source
glm::vec3 center;

//EZMesh loader instance
EzmLoader ezm;
vector<SubMesh> submeshes;	//all submeshes in the EZMesh file
std::map<std::string, GLuint> materialMap; //material name, texture id map
std::map<std::string, std::string> material2ImageMap; //material name, image name map
typedef std::map<std::string, std::string>::iterator iter;	//material2map iterator

//mesh vertices and indices
vector<Vertex> vertices;   
vector<unsigned short> indices;

//All material names in the EZMesh model file in a linear list
vector<std::string> materialNames;

//light gizmo vertex arrary and buffer object
GLuint lightVAOID;
GLuint lightVerticesVBO; 

//objectspace light position
glm::vec3 lightPosOS=glm::vec3(0, 2,0); //objectspace light position 

//spherical coordinates for rotating the light source
float theta = 1.1f;
float phi = 0.85f;
float radius = 70;

//camera transformation variables
int state = 0, oldX=0, oldY=0;
float rX=0, rY=0, dist = -120;

const std::string mesh_filename = "../media/dudeMesh.ezm";

//mouse down event handler
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

//mouse move event handler
void OnMouseMove(int x, int y)
{
	if (state == 0)
		dist *= (1 + (y - oldY)/60.0f); 
	else if(state ==2) {
		theta += (oldX - x)/60.0f; 
		phi += (y - oldY)/60.0f; 
	
		//modified theta and phi are used to create the new light position
		lightPosOS.x = center.x + radius * cos(theta)*sin(phi);
		lightPosOS.y = center.y + radius * cos(phi);
		lightPosOS.z = center.z + radius * sin(theta)*sin(phi);

	} else {
		rY += (x - oldX)/5.0f; 
		rX += (y - oldY)/5.0f; 
	}  
	oldX = x; 
	oldY = y; 

	//call display function
	glutPostRedisplay(); 
}

//OpenGL initialization
void OnInit() { 
 
	//get mesh path 
	std::string mesh_path = mesh_filename.substr(0, mesh_filename.find_last_of("//")+1);

	glm::vec3 min, max;
	//load the EZmesh file
	if(!ezm.Load(mesh_filename.c_str(), submeshes, vertices, indices, material2ImageMap, min, max)) { 
		cout<<"Cannot load the 3ds mesh"<<endl;
		exit(EXIT_FAILURE);
	} 
	GL_CHECK_ERRORS

	//store the loaded material names into a vector
	for(iter i = material2ImageMap.begin();i!=material2ImageMap.end();++i) {
		materialNames.push_back(i->second);
	}

	//calculate the distance the camera has to be moved to properly view the EZMesh model	
	center = (max + min) * 0.5f; 
	glm::vec3 diagonal = (max-min);
	radius = glm::length(center- diagonal * 0.5f);
	dist = -glm::length(diagonal);
	 	 
	//generate OpenGL textures from the loaded material names  
	for(size_t k=0;k<materialNames.size();k++) { 
		if(materialNames[k].length()==0)
			continue;

		//generate OpenGL texture object
		GLuint id = 0;
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		int texture_width = 0, texture_height = 0, channels=0;	

		//get the full image name
		const string& filename =  materialNames[k];
		std::string full_filename = mesh_path;
		full_filename.append(filename);

		//use SOIL to load the image
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

		//determine the image format
		GLenum format = GL_RGBA;
			switch(channels) {
				case 2:	format = GL_RG32UI; break;
				case 3: format = GL_RGB;	break;
				case 4: format = GL_RGBA;	break;
			}
		//allocate the texture 
		glTexImage2D(GL_TEXTURE_2D, 0, format, texture_width, texture_height, 0, format, GL_UNSIGNED_BYTE, pData);

		//delete the SOIL image data
		SOIL_free_image_data(pData);

		//store the texture id into the material map. Refer to the texture by name 
		//will give us its OpenGL texture id
		materialMap[filename] = id ;
	} 
	GL_CHECK_ERRORS 
	
	//load flat shader
	flatShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/flat.vert");
	flatShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/flat.frag");
	//compile and link flat shader
	flatShader.CreateAndLinkProgram();
	flatShader.Use();	
		//add shader attributes and uniforms
		flatShader.AddAttribute("vVertex");
		flatShader.AddUniform("MVP"); 
	flatShader.UnUse();

	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/shader.frag");
	//compile and link flat shader
	shader.CreateAndLinkProgram();
	shader.Use();	
		//add shader attributes and uniforms
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
		//set values of constant shader uniforms at initialization
		glUniform1i(shader("textureMap"), 0);
	shader.UnUse();

	GL_CHECK_ERRORS

 
	//setup geometry  
	//setup vao and vbo stuff
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	glGenBuffers(1, &vboIndicesID); 
	 
	glBindVertexArray(vaoID);	
		glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
		//pass vertices to buffer object memory
		glBufferData (GL_ARRAY_BUFFER, sizeof(Vertex)*vertices.size(), &(vertices[0].pos.x), GL_DYNAMIC_DRAW);
		GL_CHECK_ERRORS
		//enable vertex attribute for position
		glEnableVertexAttribArray(shader["vVertex"]);
		glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE,sizeof(Vertex),0);
		GL_CHECK_ERRORS 
		//enable vertex attribute for normals
		glEnableVertexAttribArray(shader["vNormal"]);
		glVertexAttribPointer(shader["vNormal"], 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, normal)) );
 
		GL_CHECK_ERRORS
		//enable vertex attribute array for texture coordinate 
		glEnableVertexAttribArray(shader["vUV"]);
		glVertexAttribPointer(shader["vUV"], 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, uv)) );

		GL_CHECK_ERRORS
 
		//glBindVertexArray(0); 

	//setup vao and vbo stuff for the light position crosshair
	glm::vec3 crossHairVertices[6];
	crossHairVertices[0] = glm::vec3(-0.5f,0,0);
	crossHairVertices[1] = glm::vec3(0.5f,0,0);
	crossHairVertices[2] = glm::vec3(0, -0.5f,0);
	crossHairVertices[3] = glm::vec3(0, 0.5f,0);
	crossHairVertices[4] = glm::vec3(0,0, -0.5f);
	crossHairVertices[5] = glm::vec3(0,0, 0.5f);

	//generate light vertex array and buffer object
	glGenVertexArrays(1, &lightVAOID);
	glGenBuffers(1, &lightVerticesVBO);   
	glBindVertexArray(lightVAOID);	
		glBindBuffer (GL_ARRAY_BUFFER, lightVerticesVBO);
		glBufferData (GL_ARRAY_BUFFER, sizeof(crossHairVertices), &(crossHairVertices[0].x), GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//enable vertex attribute array
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0,0);
	 
		GL_CHECK_ERRORS 
	
	//get the light position using the center and the spherical coordinates	
	lightPosOS.x = center.x + radius * cos(theta)*sin(phi);
	lightPosOS.y = center.y + radius * cos(phi);
	lightPosOS.z = center.z + radius * sin(theta)*sin(phi);

	//enable depth test and culling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE); 

	//set clear color to corn blue
	glClearColor(0.5,0.5,1,1);
	cout<<"Initialization successfull"<<endl;
}

//release all allocated resources here
void OnShutdown() {
	 
	//delete all textures
	size_t total_textures = materialMap.size();
	for(size_t i=0;i<total_textures;i++) {
		glDeleteTextures(1, &materialMap[materialNames[i]]);
	}
	materialNames.clear();
	materialMap.clear(); 
	submeshes.clear(); 
	vertices.clear();
	indices.clear();
	 
	//Destroy shader
	shader.DeleteShaderProgram();
	flatShader.DeleteShaderProgram();

	//Destroy vao and vbo
	glDeleteBuffers(1, &vboVerticesID); 
	glDeleteBuffers(1, &vboIndicesID);
	glDeleteVertexArrays(1, &vaoID); 

	glDeleteVertexArrays(1, &lightVAOID); 
	glDeleteVertexArrays(1, &lightVerticesVBO); 

	cout<<"Shutdown successfull"<<endl;
}

//resize event handler
void OnResize(int w, int h) {
	//set the viewport
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//setup the projection matrix
	P = glm::perspective(60.0f,(float)w/h, 0.1f,1000.0f);
}

//display function
void OnRender() {
	//clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//setup the camera transform
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(-center.x, -center.y, -center.z+dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));  
 
	//bind the mesh vertex array object
	glBindVertexArray(vaoID); {
		//bind the mesh's shader
		shader.Use();
			//pass the shader uniforms
			glUniformMatrix4fv(shader("MV"), 1, GL_FALSE, glm::value_ptr(MV));	
			glUniformMatrix3fv(shader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));	
			glUniformMatrix4fv(shader("P"), 1, GL_FALSE, glm::value_ptr(P));	
			glUniform3fv(shader("light_position"),1, &(lightPosOS.x)); 

			//for all submeshes
			for(size_t i=0;i<submeshes.size();i++) {
				//if the material name is not empty
				if(strlen(submeshes[i].materialName)>0) {
					//get the OpenGL texture id from the material map using the material name
					GLuint id = materialMap[material2ImageMap[submeshes[i].materialName]];
					GLint whichID[1];
					glGetIntegerv(GL_TEXTURE_BINDING_2D, whichID);
					//if the currently bound texture id is not the same is the current texture id
					//if so bind the current texture
					if(whichID[0] != id)
						glBindTexture(GL_TEXTURE_2D, id);
					//let the shader know that we have a texture so no default colour needs to be used					
					glUniform1f(shader("useDefault"), 0.0);
				} else {  
					//there is no texture in submesh, use a default colour
					glUniform1f(shader("useDefault"), 1.0);
				}
				//draw the triangles using the submesh indices
 				glDrawElements(GL_TRIANGLES, submeshes[i].indices.size(), GL_UNSIGNED_INT, &submeshes[i].indices[0]);
			} //end for

		//unbind shader
		shader.UnUse(); 
	}
 
	//disable depth testing
	glDisable(GL_DEPTH_TEST);

	//bind the light vertex array object to show the light crosshair gizmo
	glBindVertexArray(lightVAOID); {
		//get the modeling transform of the light gizmo
		glm::mat4 T = glm::translate(glm::mat4(1), lightPosOS); 
		//set the shader of the light crosshair gizmo
		flatShader.Use();
			//pass uniforms to the shader and render the crosshair gizmo as lines
			glUniformMatrix4fv(flatShader("MVP"), 1, GL_FALSE, glm::value_ptr(P*MV*T));
				glDrawArrays(GL_LINES, 0, 6);
		//unbind the shader
		flatShader.UnUse();
	}

	//enable depth testing
	glEnable(GL_DEPTH_TEST);

	//swap the back buffer and front buffer to display the result on screen
	glutSwapBuffers();
}

//mouse wheel scroll handler which changes the radius of the light source
//since the position is given in spherical coordinates, the radius contols 
//how far from the center the light source is.
void OnMouseWheel(int button, int dir, int x, int y) {
			
	if (dir > 0)
    {
        radius += 0.1f;
    }
    else
    {
        radius -= 0.1f;
    }

	//get the new light position
	lightPosOS.x = center.x + radius * cos(theta)*sin(phi);
	lightPosOS.y = center.y + radius * cos(phi);
	lightPosOS.z = center.z + radius * sin(theta)*sin(phi);

	//call the display function
	glutPostRedisplay();
}
 

int main(int argc, char** argv) {
	//freeglut initialization calls
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);	
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("EZMesh Skeletal Mesh Viewer - OpenGL 3.3");

	//glew initialization
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

	//print information on screen
	cout<<"\tUsing GLEW "<<glewGetString(GLEW_VERSION)<<endl;
	cout<<"\tVendor: "<<glGetString (GL_VENDOR)<<endl;
	cout<<"\tRenderer: "<<glGetString (GL_RENDERER)<<endl;
	cout<<"\tVersion: "<<glGetString (GL_VERSION)<<endl;
	cout<<"\tGLSL: "<<glGetString (GL_SHADING_LANGUAGE_VERSION)<<endl;
	
	GL_CHECK_ERRORS

	//initialization of OpenGL
	OnInit();

	//callback hooks
	glutCloseFunc(OnShutdown);
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize); 
	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);
	glutMouseWheelFunc(OnMouseWheel);

	//main loop call
	glutMainLoop();	

	return 0;
}
