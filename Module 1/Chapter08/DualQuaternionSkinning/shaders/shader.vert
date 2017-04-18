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
smooth out vec2 vUVout;						//texture coordinates
smooth out vec3 vEyeSpaceNormal;    		//eye space normals
smooth out vec3 vEyeSpacePosition;			//eye space positions

//function that returns a matrix given the dual quaternion, ordiary (Qn) and dual (Qd)
mat4 dualQuatToMatrix(vec4 Qn, vec4 Qd)
{	
	mat4 M;
    float len2 = dot(Qn, Qn);
    float w = Qn.w, x = Qn.x, y = Qn.y, z = Qn.z;
    float t0 = Qd.w, t1 = Qd.x, t2 = Qd.y, t3 = Qd.z;
				
    M[0][0] = w*w + x*x - y*y - z*z;
	M[0][1] = 2 * x * y + 2 * w * z;
	M[0][2] = 2 * x * z - 2 * w * y;
	M[0][3] = 0;

    M[1][0] = 2 * x * y - 2 * w * z;
    M[1][1] = w * w + y * y - x * x - z * z;
	M[1][2] = 2 * y * z + 2 * w * x;
	M[1][3] = 0;

	M[2][0] = 2 * x * z + 2 * w * y;
    M[2][1] = 2 * y * z - 2 * w * x;
    M[2][2] = w * w + z * z - x * x - y * y;
	M[2][3] = 0;

    M[3][0] = -2 * t0 * x + 2 * w * t1 - 2 * t2 * z + 2 * y * t3;
    M[3][1] = -2 * t0 * y + 2 * t1 * z - 2 * x * t3 + 2 * w * t2;
    M[3][2] = -2 * t0 * z + 2 * x * t2 + 2 * w * t3 - 2 * t1 * y;
	M[3][3] = len2;

    M /= len2;

    return M;	
}

void main()
{
	//initialize local variables
	vec4 blendVertex=vec4(0);
	vec3 blendNormal=vec3(0); 
	vec4 blendDQ[2];
	
	//here we check the dot product between the two quaternions
	float yc = 1.0, zc = 1.0, wc = 1.0;
    
	//if the dot product is < 0 they are opposite to each other
	//hence we multiply the -1 which would subtract the blended result
    if (dot(Bones[viBlendIndices.x * 2], Bones[viBlendIndices.y * 2]) < 0.0)
		yc = -1.0;
    
    if (dot(Bones[viBlendIndices.x * 2], Bones[viBlendIndices.z * 2]) < 0.0)
       	zc = -1.0;
	
    if (dot(Bones[viBlendIndices.x * 2], Bones[viBlendIndices.w * 2]) < 0.0)
		wc = -1.0;
	
    //get the dual quaternions for the first index
	//multiply with the given blend weight
	blendDQ[0] = Bones[viBlendIndices.x * 2] * vBlendWeights.x;
    blendDQ[1] = Bones[viBlendIndices.x * 2 + 1] * vBlendWeights.x;
    
	//get the dual quaternions for the second index
	//multiply with the given blend weight and add to the existing dual quaternion
    blendDQ[0] += yc*Bones[viBlendIndices.y * 2] * vBlendWeights.y;
    blendDQ[1] += yc*Bones[viBlendIndices.y * 2 + 1] * vBlendWeights.y;
    
	//get the dual quaternions for the third index
	//multiply with the given blend weight and add to the existing dual quaternion
    blendDQ[0] += zc*Bones[viBlendIndices.z * 2] * vBlendWeights.z;
    blendDQ[1] += zc*Bones[viBlendIndices.z * 2 + 1] * vBlendWeights.z;
    
	//get the dual quaternions for the fourth index
	//multiply with the given blend weight and add to the existing dual quaternion
    blendDQ[0] += wc*Bones[viBlendIndices.w * 2] * vBlendWeights.w;
    blendDQ[1] += wc*Bones[viBlendIndices.w * 2 + 1] * vBlendWeights.w;

	//get the skinning matrix from the dual quaternion
	mat4 skinTransform = dualQuatToMatrix(blendDQ[0], blendDQ[1]);

	//multiply the skinning matrix with the given vertex and normals to get the 
	//blended vertex and normal 
    blendVertex = skinTransform*vec4(vVertex,1);
	blendNormal = (skinTransform*vec4(vNormal,0)).xyz;

	//finally multiply the blendVertex with the modelview matrix to get the eye space position
    vEyeSpacePosition = (MV*blendVertex).xyz; 

	//multiply the blendNormal with the normal matrix to get the eye space normal
    vEyeSpaceNormal   = N*blendNormal;  
	 
	//output the texture coordinates
	vUVout=vUV; 

	//get the clipspace position by multiplying the eye space position with the projection matrix
    gl_Position = P*vec4(vEyeSpacePosition,1);
}