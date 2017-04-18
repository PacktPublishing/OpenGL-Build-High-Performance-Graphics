#version 330 core
layout (points) in;
layout (points, max_vertices=3) out; 
 
//uniforms			
uniform mat4 MVP;				//combined modelview projection matrix
uniform vec4 FrustumPlanes[6];	//view frustum planes

//check if the given point (p) is inside the plane. The code simply finds the point to plane distance
//if it is less than 0, the point is clearly below the plane
bool PointInFrustum(in vec3 p) {
	for(int i=0; i < 6; i++) 
	{
		vec4 plane=FrustumPlanes[i];
		if ((dot(plane.xyz, p)+plane.w) < 0)
			return false;
	}
	return true;
}

void main()
{
	//get the vertices
	for(int i=0;i<gl_in.length(); i++) { 
		vec4 vInPos = gl_in[i].gl_Position;
		//convert the points from 0 to 1 range to -5 to 5 range
		vec2 tmp = (vInPos.xz*2-1.0)*5;
		//take the Y value from the vertex shafer output
		vec3 V = vec3(tmp.x, vInPos.y, tmp.y);

		//multiply the combined MVP matrix with object space position to get clip space position 
		gl_Position = MVP*vec4(V,1);
		
		//only emit vertex if it is in view frustum	  
		if(PointInFrustum(V)) { 
			EmitVertex();		
		} 
	}
	EndPrimitive();
}