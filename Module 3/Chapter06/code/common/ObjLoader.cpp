//=======================================================================
// Authors: Raymond Lo and William Lo
// Chapter 6 - Rendering Stereoscopic 3D Models using OpenGL
// Copyrights & Licenses:
//=======================================================================

#include <ObjLoader.hpp>

/**
 * Constructor
 */
ObjLoader::ObjLoader() {
	g_scale=1.0f;
	scene = NULL; //empty scene
	scene_list = 0;
	num_vertices = 0;
}

/**
 * Load the Asset file (OBJ or all supported by Assimp) into memory
 * @param path
 * @return 0 if loaded successfully.
 */
int ObjLoader::loadAsset(const char *path){
	scene = aiImportFile(path, aiProcessPreset_TargetRealtime_MaxQuality);
	if (scene) {

		get_bounding_box(&scene_min,&scene_max);
		scene_center.x = (scene_min.x + scene_max.x) / 2.0f;
		scene_center.y = (scene_min.y + scene_max.y) / 2.0f;
		scene_center.z = (scene_min.z + scene_max.z) / 2.0f;

		printf("Loaded 3D file %s\n", path);
		g_scale =4.0/(scene_max.x-scene_min.x);

		//count the vertices for later use
		num_vertices = recursiveGetNumVertices(scene->mRootNode);
		printf("This Scene has %d vertices.\n", num_vertices);
		printf("Center: x, y, z %lf %lf %lf\n",scene_center.x, scene_center.y, scene_center.z );
		return 0;
	}
	return 1;
}

/**
 * Override the scale for the rendering model
 */
void ObjLoader::setScale(float scale){
	g_scale = scale;
}

/**
 * Get the bounding box for the scene
 */
void ObjLoader::get_bounding_box (aiVector3D* min, aiVector3D* max)
{
	aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);
	min->x = min->y = min->z =  1e10f;
	max->x = max->y = max->z = -1e10f;
	get_bounding_box_for_node(scene->mRootNode,min,max,&trafo);
}

/**
 * Helper function for getting the bounding box for the scene by recursively
 * walking through all nodes
 */
void ObjLoader::get_bounding_box_for_node (const struct aiNode* nd,
	aiVector3D* min,
	aiVector3D* max,
	aiMatrix4x4* trafo
){
	aiMatrix4x4 prev;
	unsigned int n = 0, t;

	prev = *trafo;
	aiMultiplyMatrix4(trafo,&nd->mTransformation);

	for (; n < nd->mNumMeshes; ++n) {
		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		for (t = 0; t < mesh->mNumVertices; ++t) {

			aiVector3D tmp = mesh->mVertices[t];
			aiTransformVecByMatrix4(&tmp,trafo);

			min->x = aisgl_min(min->x,tmp.x);
			min->y = aisgl_min(min->y,tmp.y);
			min->z = aisgl_min(min->z,tmp.z);

			max->x = aisgl_max(max->x,tmp.x);
			max->y = aisgl_max(max->y,tmp.y);
			max->z = aisgl_max(max->z,tmp.z);
		}
	}

	for (n = 0; n < nd->mNumChildren; ++n) {
		get_bounding_box_for_node(nd->mChildren[n],min,max,trafo);
	}
	*trafo = prev;
}

/**
 * Draw the scene
 * @param: GLenum draw_mode
 */
void ObjLoader::draw(const GLenum draw_mode){
	recursiveDrawing(scene->mRootNode, 0, draw_mode);
}

/**
 * Helper function which recursively draws all parts from a scene
 */
unsigned int ObjLoader::recursiveDrawing(const struct aiNode* nd, unsigned int v_counter, const GLenum draw_mode){
	//break up the drawing, and shifting the pointer to draw different part of the scene
	unsigned int i;
	unsigned int n = 0, t;
	unsigned int total_count = v_counter;
	// draw all meshes assigned to this node
	for (; n < nd->mNumMeshes; ++n) {
		unsigned int count=0;
		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		for (t = 0; t < mesh->mNumFaces; ++t) {
			const struct aiFace* face = &mesh->mFaces[t];
			count+=3*face->mNumIndices;
		}
		glDrawArrays(draw_mode, total_count, count);
		total_count+=count;
	}
	v_counter = total_count;
	// draw all children
	for (n = 0; n < nd->mNumChildren; ++n) {
		v_counter = recursiveDrawing(nd->mChildren[n], v_counter, draw_mode);
	}
	return v_counter;
}

/**
 * Load the vertices into the vertex buffer recursively.
 */
void ObjLoader::loadVertices(GLfloat *g_vertex_buffer_data){
	recursiveVertexLoading(scene->mRootNode, g_vertex_buffer_data, 0);
}

/**
 * Helper function for loading the vertex into vertex array.
 */
unsigned int ObjLoader::recursiveVertexLoading(const struct aiNode *nd,
		GLfloat *g_vertex_buffer_data, unsigned int v_counter){
	unsigned int i;
	unsigned int n = 0, t;

	// draw all meshes assigned to this node
	for (; n < nd->mNumMeshes; ++n) {
		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		for (t = 0; t < mesh->mNumFaces; ++t) {
			const struct aiFace* face = &mesh->mFaces[t];
			for(i = 0; i < face->mNumIndices; i++) {
				int index = face->mIndices[i];
				g_vertex_buffer_data[v_counter]=(mesh->mVertices[index].x-scene_center.x)*g_scale;
				g_vertex_buffer_data[v_counter+1]=(mesh->mVertices[index].y-scene_center.y)*g_scale;
				g_vertex_buffer_data[v_counter+2]=(mesh->mVertices[index].z-scene_center.z)*g_scale;
				v_counter+=3;
			}
		}
	}

	// draw all children
	for (n = 0; n < nd->mNumChildren; ++n) {
		v_counter = recursiveVertexLoading(nd->mChildren[n], g_vertex_buffer_data, v_counter);
	}
	return v_counter;
}

/**
 * Get the number of vertices in the entire scene
 */
unsigned int ObjLoader::getNumVertices(){
	return num_vertices;
}

/**
 * Helper function to recursively walk through the nodes
 */
unsigned int ObjLoader::recursiveGetNumVertices(const struct aiNode *nd){
	unsigned int counter=0;
	unsigned int i;
	unsigned int n = 0, t;
	// draw all meshes assigned to this node
	for (; n < nd->mNumMeshes; ++n) {
		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		for (t = 0; t < mesh->mNumFaces; ++t) {
			const struct aiFace* face = &mesh->mFaces[t];
			counter+=3*face->mNumIndices;
		}
	}
	// draw all children
	for (n = 0; n < nd->mNumChildren; ++n) {
		counter+=recursiveGetNumVertices(nd->mChildren[n]);
	}
	return counter;
}


/**
 * Destructor
 */
ObjLoader::~ObjLoader() {
	//free the memory if the scene was loaded
	if(scene)
		aiReleaseImport(scene);
    g_scale = 1.0f;
    num_vertices = 0;
}

