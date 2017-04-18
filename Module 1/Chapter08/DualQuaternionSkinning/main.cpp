#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "..\src\GLSLShader.h"
#include <vector>
#include "Ezm.h"

#include <SOIL.h>


#include <sstream>
#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "SOIL.lib")

using namespace std;

//screen size
const int WIDTH  = 1280;
const int HEIGHT = 960;

//shaders
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

//all submeshes in the EZMesh file
vector<SubMesh> submeshes;

std::map<std::string, GLuint> materialMap;					//material name, texture id map
std::map<std::string, std::string> material2ImageMap;		//material name, image name map
typedef std::map<std::string, std::string>::iterator iter;	//material2map iterator

//All material names in the EZMesh model file in a linear list
vector<std::string> materialNames;

//linear list of all mesh vertices and indices 
//in the EZMesh model
vector<Vertex> vertices;
vector<unsigned short> indices;

//light gizmo vertex arrary and buffer object
GLuint lightVAOID;
GLuint lightVerticesVBO;

//objectspace light position
glm::vec3 lightPosOS=glm::vec3(0, 2,0); //objectspace light position

//spherical coordinates for rotating the light source
float theta = 5.4f;
float phi = 0.86f;
float radius = 30;

int currentFrame = -1; //-1 -> bindPose, 0 to frameCount-1 -> skinned
bool bLoop = true;	   //enable/disable loop playback

//camera transformation variables
int state = 0, oldX=0, oldY=0;
float rX=16, rY=180, dist = -95;

//mesh filename to load
const std::string mesh_filename = "../media/dwarf_anim.ezm";

//vectors to store EZMesh skeleton (Bone information), 
//bind pose, inverse bind pose, animated transforms and mesh animations
vector<Bone> skeleton;
vector<glm::mat4> bindPose;
vector<glm::mat4> invBindPose;
vector<glm::mat4> animatedXform;
vector<NVSHARE::MeshAnimation> animations;

//flag which shows if the model is Yup or Zup
bool bYup=false;

//high performance timer variables
LARGE_INTEGER freq, last, current;
double dt;

//a simple dual quaternion class
class dual_quat {
public:
	glm::quat ordinary, dual;	//the two quaternion ordinary and its dual

	//a simple function that create a dual quaternion from the given 
	//orientation (q0) and translation (t)
	void QuatTrans2UDQ(const glm::quat& q0, const glm::vec3& t) {
		ordinary = q0;
		dual.w = -0.5f * ( t.x * q0.x + t.y * q0.y + t.z * q0.z);
		dual.x =  0.5f * ( t.x * q0.w + t.y * q0.z - t.z * q0.y);
        dual.y =  0.5f * (-t.x * q0.z + t.y * q0.w + t.z * q0.x);
		dual.z =  0.5f * ( t.x * q0.y - t.y * q0.x + t.z * q0.w);
	}

	//converts the dual quaternion to a matrix
	void UDQToMatrix(glm::mat4& m) {
		float len2 = glm::dot(ordinary, ordinary);
        float w = ordinary.w , x = ordinary.x, y = ordinary.y, z = ordinary.z;
        float t0 = dual.w, t1 = dual.x, t2 = dual.y, t3 = dual.z;
		m[0][0] = w*w + x*x - y*y - z*z;
		m[1][0] = 2 * x * y - 2 * w * z;
		m[2][0] = 2 * x * z + 2 * w * y;
		m[0][1] = 2 * x * y + 2 * w * z;
		m[1][1] = w * w + y * y - x * x - z * z;
		m[2][1] = 2 * y * z - 2 * w * x;
		m[0][2] = 2 * x * z - 2 * w * y;
		m[1][2] = 2 * y * z + 2 * w * x;
		m[2][2] = w * w + z * z - x * x - y * y;

		m[3][0] = -2 * t0 * x + 2 * w * t1 - 2 * t2 * z + 2 * y * t3;
        m[3][1] = -2 * t0 * y + 2 * t1 * z - 2 * x * t3 + 2 * w * t2;
        m[3][2] = -2 * t0 * z + 2 * x * t2 + 2 * w * t3 - 2 * t1 * y;

		m[0][3] = 0;
        m[1][3] = 0;
        m[2][3] = 0;
		m[3][3] = len2;
	    m /= len2;
	}

};

//a vector of qual quaternions
vector<dual_quat> dualQuaternions;

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

	glutPostRedisplay();
}

//function to generate absolute transforms for all bones
//it simply loops through the entire skeleton and multiplies
//the current bones local transform with its parent's absolute
//transform. If the bone has no parent, the local transform is 
//the absolute transform.
void UpdateCombinedMatrices() {
	for(size_t i=0;i<skeleton.size();i++) {
		Bone& b = skeleton[i];
		if(b.parent==-1)
			b.comb = b.xform;
		else
			b.comb = skeleton[b.parent].comb * b.xform;
	}
}

//OpenGL initialization
void OnInit() {

	//init high performance timer
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&last);

	//get mesh path
	std::string mesh_path = mesh_filename.substr(0, mesh_filename.find_last_of("//")+1);

	glm::vec3 min, max;

	//load the EZmesh file
	if(!ezm.Load(mesh_filename.c_str(), skeleton, animations, submeshes, vertices, indices, material2ImageMap, min, max)) {
		cout<<"Cannot load the EZMesh file"<<endl;
		exit(EXIT_FAILURE);
	}

	//check the absolute value y and z dimensions of the bounding box
	float dy = fabs(max.y-min.y);
	float dz = fabs(max.z-min.z);
	bYup = (dy>dz);


	//get the combined bone transform
	UpdateCombinedMatrices();

	//resize bind pose, inverse bind pose, animatedXform and dualQuaternion vectors
	bindPose.resize(skeleton.size());
	invBindPose.resize(skeleton.size());
	animatedXform.resize(skeleton.size());
	dualQuaternions.resize(skeleton.size());

	//store the bind pose matrices which are the absolute transform of
	//each bone. Also store their inverse which is used in skinning
	for(size_t i=0;i<skeleton.size();i++) {
		bindPose[i] = (skeleton[i].comb);
		invBindPose[i] = glm::inverse(bindPose[i]);
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

		//get the full image name
		int texture_width = 0, texture_height = 0, channels=0;
		const string& filename =  materialNames[k];
		std::string full_filename = mesh_path;
		full_filename.append(filename);

		//pass the full image name including the path and use SOIL library to load the image		
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
		
		GLuint id = 0;
		//generate new texture id
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		//set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//allocate the texture 
		glTexImage2D(GL_TEXTURE_2D, 0, format, texture_width, texture_height, 0, format, GL_UNSIGNED_BYTE, pData);
		
		//delete the SOIL image data
		SOIL_free_image_data(pData);

		//store the texture id into the material map. Refer to the texture by name 
		//will give us its OpenGL texture id
		materialMap[filename] = id ;
	}
	GL_CHECK_ERRORS
	//setup shaders
	flatShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/flat.vert");
	flatShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/flat.frag");
	//compile and link flat shader
	flatShader.CreateAndLinkProgram();
	flatShader.Use();
		//add shader attributes and uniforms
		flatShader.AddAttribute("vVertex");
		flatShader.AddUniform("MVP");
	flatShader.UnUse();

	//For the skinning vertex shader, we pass the Bones array dynamcially
	//since we may not know the total number of bones in the model at compile
	//time. Since the GLSL arrays have to be a compile time constant, we 
	//dynamically generate the shader string to add the uniform in the shader.
	//To achieve this, we overload the GLSLShader::LoadFromFile function with
	//a thid parameter, the string we want to add before the shader main function.
	stringstream str( ios_base::app | ios_base::out);
	str<<"\nconst int NUM_BONES="<<skeleton.size()*2<<";"<<endl;
	str<<"uniform vec4 Bones[NUM_BONES];"<<endl;
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert", str.str());
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/shader.frag");
		
	//compile and link shader
	shader.CreateAndLinkProgram();
	shader.Use();
		//add shader attributes and uniforms
		shader.AddAttribute("vVertex");
		shader.AddAttribute("vNormal");
		shader.AddAttribute("vUV");
		shader.AddAttribute("vBlendWeights");
		shader.AddAttribute("viBlendIndices");

		shader.AddUniform("Bones");
		shader.AddUniform("MV");
		shader.AddUniform("N");
		shader.AddUniform("P");
		shader.AddUniform("textureMap");
		shader.AddUniform("useDefault");

		shader.AddUniform("light_position");
		shader.AddUniform("diffuse_color");

		glUniform1i(shader("textureMap"), 0);
		//pass values to uniforms at initialization, since we have a dual quaternion we pass
		//it as 2 vec4 variables hence the multiplication by 2
		glUniform4fv(shader("Bones"), skeleton.size()*2, &(dualQuaternions[0].ordinary.x));

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
		//enable vertex attribute 
		glEnableVertexAttribArray(shader["vVertex"]);
		glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE,sizeof(Vertex),0);
		GL_CHECK_ERRORS
		//enable normal attribute 
		glEnableVertexAttribArray(shader["vNormal"]);
		glVertexAttribPointer(shader["vNormal"], 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, normal)) );

		GL_CHECK_ERRORS
		//enable texture coordinate attribute 
		glEnableVertexAttribArray(shader["vUV"]);
		glVertexAttribPointer(shader["vUV"], 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, uv)) );

		GL_CHECK_ERRORS
		//enable blend weights attribute array
		glEnableVertexAttribArray(shader["vBlendWeights"]);
		glVertexAttribPointer(shader["vBlendWeights"], 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, blendWeights)) );

		GL_CHECK_ERRORS
		//enable blend indices attribute array
		glEnableVertexAttribArray(shader["viBlendIndices"]);
		glVertexAttribIPointer(shader["viBlendIndices"], 4, GL_INT, sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, blendIndices)) );

	GL_CHECK_ERRORS
		 

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

	//pass the cross hair data to the buffer object memory
	glBindBuffer (GL_ARRAY_BUFFER, lightVerticesVBO);
	glBufferData (GL_ARRAY_BUFFER, sizeof(crossHairVertices), &(crossHairVertices[0].x), GL_DYNAMIC_DRAW);
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
		if(materialNames[i].length()>0)
			glDeleteTextures(1, &materialMap[materialNames[i]]);
	}
	materialNames.clear();
	materialMap.clear();
	submeshes.clear();
	vertices.clear();
	indices.clear();
	skeleton.clear();
	animations.clear();

	dualQuaternions.clear();
	animatedXform.clear();
	skeleton.clear();
	bindPose.clear();
	invBindPose.clear();

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

//idle callback which is called when there is no other processing to do
void OnIdle() {
	//get the current timer value
	QueryPerformanceCounter(&current);
    dt = (double)(current.QuadPart - last.QuadPart) / (double)freq.QuadPart;
	last = current;

	static double t  = 0;
	//increment time
	t+=dt; 

	//get the current mesh animation and store the frame rate
	//if the current time is greater than the framerate, we move
	//to the next frame
	NVSHARE::MeshAnimation* pAnim = &animations[0];
	float fps = pAnim->GetFrameCount() / pAnim->GetDuration();
	if( t > 1.0f/fps) {
		currentFrame++;
		t=0;
	}

	//if looped playback is on, we do a modulus operation of the current frame with
	//the total number of frames
	if(bLoop) {
		currentFrame = currentFrame%pAnim->mFrameCount;
	} else {
		//otherwise, we just restrict the current frame to be in range
		currentFrame = max(-1, min(currentFrame, pAnim->mFrameCount-1));
	}

	//if the current frame is -1, means we are in bind pose
	//just use the bind  pose matrix and the bones absolute 
	//transforms to get the aniamted transform
	if(currentFrame == -1) {
		for(size_t i=0;i<skeleton.size();i++) {
			skeleton[i].comb = bindPose[i];
			animatedXform[i] = skeleton[i].comb*invBindPose[i];
		}
	} else {
		//otherwise, we loop through all tracks in the current animation
		//and determine the pose.
		for(int j=0;j<pAnim->mTrackCount;j++) {
			NVSHARE::MeshAnimTrack* pTrack = pAnim->mTracks[j];
			NVSHARE::MeshAnimPose* pPose = pTrack->GetPose(currentFrame);
			
			//using the pose, we estimate the local transform of the current bone
			//in the given animation track
			//first get positions
			skeleton[j].position.x = pPose->mPos[0];
			skeleton[j].position.y = pPose->mPos[1];
			skeleton[j].position.z = pPose->mPos[2];

			//then orientation
			glm::quat q;
			q.x = pPose->mQuat[0];
			q.y = pPose->mQuat[1];
			q.z = pPose->mQuat[2];
			q.w = pPose->mQuat[3];

			//then scale
			skeleton[j].scale  = glm::vec3(pPose->mScale[0], pPose->mScale[1], pPose->mScale[2]);

			//handle the Zup case
			if(!bYup) {
				skeleton[j].position.y = pPose->mPos[2];
				skeleton[j].position.z = -pPose->mPos[1];
				q.y = pPose->mQuat[2];
				q.z = -pPose->mQuat[1];

				skeleton[j].scale.y = pPose->mScale[2];
				skeleton[j].scale.z = -pPose->mScale[1];
			}

			skeleton[j].orientation = q;
			
			//use the stored scale, orientation and translation to obtain the 
			//local transform of bone
			glm::mat4 S = glm::scale(glm::mat4(1),skeleton[j].scale);
			glm::mat4 R = glm::toMat4(q);
			glm::mat4 T = glm::translate(glm::mat4(1), skeleton[j].position);
			skeleton[j].xform = T*R*S;

			//get the absolute transform of the bone
			Bone& b = skeleton[j];
			if(b.parent==-1)
				b.comb = b.xform;
			else
				b.comb = skeleton[b.parent].comb * b.xform;

			//multiply the absolute transform of the current bone with
			//the inverse bind pose of the bone to get the new animated 
			//transform
			animatedXform[j] = b.comb * invBindPose[j];

			//in case of dual quaternion, extract the position and orientation and then use
			//QuatTrans2UDQ function to obtain a dual quaternion for the given animatedXform
			glm::vec3 t = glm::vec3( animatedXform[j][3][0], animatedXform[j][3][1], animatedXform[j][3][2]);
			dualQuaternions[j].QuatTrans2UDQ(glm::toQuat(animatedXform[j]),  t); 

		}//for all animation tracks
	} //else

	//pass the new dual quaternions to the shader by updating the uniform
	shader.Use();
		//since we have a dual quaternion, we pass it as 2 vec4 variables 
		//hence the multiplication by 2	
		glUniform4fv(shader("Bones"), skeleton.size()*2, &(dualQuaternions[0].ordinary.x));

	shader.UnUse();

	//call the display callback
	glutPostRedisplay();
}

//keyboard event handler
void OnKey(unsigned char key, int x, int y) {
	if(animations.size()>0) {
		switch(key) {
			case 'l': bLoop = !bLoop; break;
		}
	}
	//recall display function
	glutPostRedisplay();
}

int main(int argc, char** argv) {
	//freeglut initialization calls
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("EZMesh Skeletal Animation Viewer (Dual Quaternion) - OpenGL 3.3");

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
	glutKeyboardFunc(OnKey);
	glutIdleFunc(OnIdle);
				
	//main loop call
	glutMainLoop();

	return 0;
}
