//=======================================================================
// Authors: Raymond Lo and William Lo
// Chapter 6 - Rendering Stereoscopic 3D Models using OpenGL
// Copyrights & Licenses:
//=======================================================================

#ifndef OBJLOADER_H_
#define OBJLOADER_H_

/* assimp include files. These three are usually needed. */
#include <cimport.h>
#include <scene.h>
#include <postprocess.h>

#include <common.h>

#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)

class ObjLoader {
public:
	ObjLoader();
	virtual ~ObjLoader();
	int loadAsset(const char* path);
	void setScale(float scale);
	unsigned int getNumVertices();
	void draw(const GLenum draw_mode);
	void loadVertices(GLfloat *g_vertex_buffer_data);

private:
	//helper functions & variables
	const struct aiScene* scene;
	GLuint scene_list;
	aiVector3D scene_min, scene_max, scene_center;
	float g_scale;
	unsigned int num_vertices;

	unsigned int recursiveDrawing(const struct aiNode* nd, unsigned int v_count, const GLenum); //drawing only
	unsigned int recursiveVertexLoading(const struct aiNode *nd, GLfloat *g_vertex_buffer_data, unsigned int v_counter); //loading scene to memory
	unsigned int recursiveGetNumVertices(const struct aiNode *nd);

	void get_bounding_box (aiVector3D* min, aiVector3D* max);
	void get_bounding_box_for_node (const struct aiNode* nd,
		aiVector3D* min,
		aiVector3D* max,
		aiMatrix4x4* trafo
	);
};

#endif /* OBJLOADER_H_ */
