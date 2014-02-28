#ifndef _D3DObject_H_
#define _D3DObject_H_

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <stdio.h>

class D3DObject {
	public:
		std::vector<float> verticies, textures, normals;
		int verticies_count;

		D3DObject(std::string filename);
};

class D3DWorldObject {
	private:
		float x, y, z;
		glm::mat4 transform_matrix;
		D3DObject *mesh;
		GLuint vao;
		GLuint vbo[2];
	public:
		int oid;

		D3DWorldObject(std::string model_filepath, float x, float y, float z); 
		D3DWorldObject(D3DObject *mesh);

		~D3DWorldObject();
		void draw(GLuint shader_program, glm::mat4 projection);
		void update_coord_x(float val);
		void update_coord_y(float val);
		void update_coord_z(float val);

		float get_x() { return this->x; }
		float get_y() { return this->y; }
		float get_z() { return this->z; }

};

#endif
