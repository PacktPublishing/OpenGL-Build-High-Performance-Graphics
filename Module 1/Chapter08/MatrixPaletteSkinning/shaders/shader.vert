#version 330 core
 
layout(location = 0) in vec3 vVertex;				//vertex position
layout(location = 1) in vec3 vNormal;				//vertex normal
layout(location = 2) in vec2 vUV;					//vertex uv coordinates
layout(location = 3) in vec4 vBlendWeights;			//4 vertex blend weights
layout(location = 4) in ivec4 viBlendIndices;		//4 vertex blend indices


//uniforms for projection, modelview and normal matrices
uniform mat4 P; 
uniform mat4 MV;
uniform mat3 N;

//shader outputs to the fragment shader
smooth out vec2 vUVout;					//texture coordinates
smooth out vec3 vEyeSpaceNormal;		//eye space normals
smooth out vec3 vEyeSpacePosition;		//eye space positions

void main()
{
	//initialize local variables
	vec4 blendVertex=vec4(0);
	vec3 blendNormal=vec3(0);
	vec4 vVertex4 = vec4(vVertex,1);

	//get the first index
	int index = viBlendIndices.x;
	
	//get the bone matrix for the first index. 
	//multiply with the given vertex and the bones blend weight
	//do the same for the normal
	blendVertex = (Bones[index] * vVertex4) *  vBlendWeights.x;
    blendNormal = (Bones[index] * vec4(vNormal, 0.0)).xyz *  vBlendWeights.x;
   	 
	//get the bone matrix for the second index. 
	//multiply with the given vertex and the bones blend weight but also add to the previous 
	//blendedVertex  
	//do the same for the normal (also add the previous blendNormal)
	index = viBlendIndices.y;        
	blendVertex = ((Bones[index] * vVertex4) * vBlendWeights.y) + blendVertex;
    blendNormal = (Bones[index] * vec4(vNormal, 0.0)).xyz * vBlendWeights.y  + blendNormal;

	//get the bone matrix for the third index. 
	//multiply with the given vertex and the bones blend weight but also add to the previous 
	//blendedVertex  
	//do the same for the normal (also add the previous blendNormal)
	index = viBlendIndices.z;        
	blendVertex = ((Bones[index] * vVertex4) *  vBlendWeights.z)  + blendVertex;
    blendNormal = (Bones[index] * vec4(vNormal, 0.0)).xyz *  vBlendWeights.z  + blendNormal;

	//get the bone matrix for the fourth index. 
	//multiply with the given vertex and the bones blend weight but also add to the previous 
	//blendedVertex  
	//do the same for the normal (also add the previous blendNormal)
	index = viBlendIndices.w;        
	blendVertex = ((Bones[index] * vVertex4) *  vBlendWeights.w)   + blendVertex;
    blendNormal = (Bones[index] * vec4(vNormal, 0.0)).xyz *  vBlendWeights.w  + blendNormal;

	//finally multiply the blendVertex with the modelview matrix to get the eye space position
    vEyeSpacePosition = (MV*blendVertex).xyz; 

	//multiply the blendNormal with the normal matrix to get the eye space normal
    vEyeSpaceNormal   = normalize(N*blendNormal);  
	 
	//output the texture coordinates
	vUVout=vUV; 

	//get the clipspace position by multiplying the eye space position with the projection matrix
    gl_Position = P*vec4(vEyeSpacePosition,1);
}