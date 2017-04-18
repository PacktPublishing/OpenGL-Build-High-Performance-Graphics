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

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "SOIL.lib")

using namespace std;

//output screen resolution
const int WIDTH  = 1280;
const int HEIGHT = 960;

//shaders for use in the recipe
//mesh rendering shader and flat shader
GLSLShader shader, flatShader;

//IDs for vertex array and buffer object
GLuint vaoID;
GLuint vboVerticesID; 
GLuint vboIndicesID;

//projection and modelview matrices
glm::mat4  P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);

//Objloader instance
ObjLoader obj;					
vector<Mesh*> meshes;					//all meshes 
vector<Material*> materials; 			//all materials 
vector<unsigned short> indices;			//all mesh indices 
vector<Vertex> vertices; 				//all mesh vertices  
vector<GLuint> textures;				//all textures

//light crosshair gizmo vetex array and buffer object IDs
GLuint lightVAOID;
GLuint lightVerticesVBO; 

glm::vec3 lightPosOS=glm::vec3(0, 2,0); //objectspace light position

//spherical cooridate variables for light rotation
float theta = 0.66f;
float phi = -1.0f;
float radius = 70;

//camera transformation variables
int state = 0, oldX=0, oldY=0;
float rX=22, rY=116, dist = -120;

//OBJ mesh filename to load
const std::string mesh_filename = "../media/blocks.obj";

//mouse click handler
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

	//recall display function
	glutPostRedisplay(); 
}

//OpenGL initialization function
void OnInit() { 
	//get the mesh path for loading of textures	
	std::string mesh_path = mesh_filename.substr(0, mesh_filename.find_last_of("/")+1);

	//load the obj model
	if(!obj.Load(mesh_filename.c_str(), meshes, vertices, indices, materials)) { 
		cout<<"Cannot load the 3ds mesh"<<endl;
		exit(EXIT_FAILURE);
	} 
	GL_CHECK_ERRORS

	//load material textures  
	for(size_t k=0;k<materials.size();k++) {
		//if the diffuse texture name is not empty
		if(materials[k]->map_Kd != "") { 
			//generate a new OpenGL array texture
			GLuint id = 0;
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_2D, id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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
			//allocate texture 
			glTexImage2D(GL_TEXTURE_2D, 0, format, texture_width, texture_height, 0, format, GL_UNSIGNED_BYTE, pData);

			//release the SOIL image data
			SOIL_free_image_data(pData);

			//add loaded texture ID to vector
			textures.push_back(id);
		}
	} 
	GL_CHECK_ERRORS

	//load flat shader
	flatShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/flat.vert");
	flatShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/flat.frag");
	//compile and link shader
	flatShader.CreateAndLinkProgram();
	flatShader.Use();	
		//add attribute and uniform
		flatShader.AddAttribute("vVertex");
		flatShader.AddUniform("MVP"); 
	flatShader.UnUse();

	//load mesh rendering shader
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/shader.frag");
	//compile and link shader
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

	GL_CHECK_ERRORS

	//setup the vertex array object and vertex buffer object for the mesh
	//geometry handling 
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	glGenBuffers(1, &vboIndicesID); 
	 
	//here we are using interleaved attributes so we can just push data to one
	//buffer object and then assign different atribute pointer to identofy the 
	//different attributes
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
 
		GL_CHECK_ERRORS
			
		//if we have a single material, it means the 3ds model contains one mesh
		//we therefore load it into an element array buffer
		if(materials.size()==1) {
			//pass indices to the element array buffer if there is a single material			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*indices.size(), &(indices[0]), GL_STATIC_DRAW);
		}
		GL_CHECK_ERRORS
	glBindVertexArray(0); 

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

	//set the background colour to corn blue 
	glClearColor(0.5,0.5,1,1);
	cout<<"Initialization successfull"<<endl;
}
//release all allocated resources
void OnShutdown() {
	 
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

	//Destroy shader
	shader.DeleteShaderProgram();
	flatShader.DeleteShaderProgram();

	//Destroy vao and vbo
	glDeleteBuffers(1, &vboVerticesID); 
	glDeleteBuffers(1, &vboIndicesID);
	glDeleteVertexArrays(1, &vaoID); 
		
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
	//clear colour and depth buffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//set the camera transformation
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));  
	 
	//bind the mesh vertex array object
	glBindVertexArray(vaoID); {
		//bind the mesh rendering shader
		shader.Use();		
			//set the shader uniforms
			glUniformMatrix4fv(shader("MV"), 1, GL_FALSE, glm::value_ptr(MV));	
			glUniformMatrix3fv(shader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));	
			glUniformMatrix4fv(shader("P"), 1, GL_FALSE, glm::value_ptr(P));	
			glUniform3fv(shader("light_position"),1, &(lightPosOS.x)); 

			//loop through all materials
			for(size_t i=0;i<materials.size();i++) {
				Material* pMat = materials[i];
				//if material texture filename is not empty
				//dont use the default colour
				if(pMat->map_Kd !="") {
					glUniform1f(shader("useDefault"), 0.0);
					GLint whichID[1];
					glGetIntegerv(GL_TEXTURE_BINDING_2D, whichID);
					if(whichID[0] != textures[i])
						glBindTexture(GL_TEXTURE_2D, textures[i]);
				}
				else
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

	//disable depth testing
	glDisable(GL_DEPTH_TEST);

	//draw the light gizmo, set the light vertexx array object
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
 

int main(int argc, char** argv) {
	//freeglut initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);	
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Obj Viewer - OpenGL 3.3");

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

	//mainloop call
	glutMainLoop();	

	return 0;
}
