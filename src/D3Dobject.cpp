#include "D3DObject.h"

//
// D3DObject
//
D3DObject::D3DObject(std::string file_name) {
	const aiScene *scene = aiImportFile(file_name.c_str(), aiProcess_Triangulate | aiProcess_MakeLeftHanded);
	//const aiScene *scene = aiImportFile(file_name.c_str(), aiProcess_Triangulate);
	unsigned int g_point_count = 0;
	if(!scene) {
		fprintf(stderr, "error reading mesh %s\n", file_name.c_str());
	}

	/*
	printf("%i animations\n", scene->mNumAnimations);
	printf("%i cameras\n", scene->mNumCameras);
	printf("%i lights\n", scene->mNumLights);
	printf("%i materials\n", scene->mNumMaterials);
	printf("%i meshes\n", scene->mNumMeshes);
	printf("%i textures\n", scene->mNumTextures);
	*/

	for(unsigned int mi = 0; mi < scene->mNumMeshes; mi++) {
		const aiMesh *mesh = scene->mMeshes[mi];
		//printf("%i verts in mesh\n", mesh->mNumVertices);

		g_point_count = mesh->mNumVertices;
		for(unsigned int vi = 0; vi < mesh->mNumVertices; vi++) {
			if(mesh->HasPositions()) {
				const aiVector3D *vp = &(mesh->mVertices[vi]);
				//printf ("vp %i (%f,%f,%f)\n", vi, vp->x, vp->y, vp->z);
				this->verticies.push_back(vp->x);
				this->verticies.push_back(vp->y);
				this->verticies.push_back(vp->z);
			}
			if(mesh->HasNormals()) {
				const aiVector3D *vn = &(mesh->mNormals[vi]);
				//printf("vn %i (%f,%f,%f)\n", vi, vn->x, vn->y, vn->z);
				this->normals.push_back(vn->x);
				this->normals.push_back(vn->y);
				this->normals.push_back(vn->z);
			}
			if(mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][vi]);
				//printf("vt %i (%f, %f)\n", vi, vt->x, vt->y);
				this->textures.push_back(vt->x);
				this->textures.push_back(vt->y);
			}
			if(mesh->HasTangentsAndBitangents()) {

			}
		}
	}
	this->verticies_count = g_point_count;
	aiReleaseImport(scene);
	printf("new mesh loaded from %s\n", file_name.c_str());
}

//
// D3DWorldObject
//
D3DWorldObject::D3DWorldObject(std::string model_filepath, float x, float y, float z) {
	oid = rand() % 1000;
	mesh = new D3DObject(model_filepath);
	transform_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));

	//Generate a new buffer for our mesh and the normals for some kind of lightning 
	glGenBuffers(2, this->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, this->mesh->verticies_count * 3 * sizeof(float), &this->mesh->verticies[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, this->mesh->verticies_count * 3 * sizeof(float), &this->mesh->normals[0], GL_STATIC_DRAW);

	//Make a vertex attrib object to give to opengl when we want to render our above buffer
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL); //This is our vertex's
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL); //This sets our normals.

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

D3DWorldObject::D3DWorldObject(D3DObject *mesh) {
	this->mesh = mesh;
}

D3DWorldObject::~D3DWorldObject() {
	delete mesh;
}

void D3DWorldObject::draw(GLuint shader_program, glm::mat4 projection) {
	//We pass our transformation matrix to the shader each render, this is
	//what we use to position our mesh in world space.
	GLint transform = glGetUniformLocation(shader_program, "transform");
	glUseProgram(shader_program);
	glUniformMatrix4fv(transform, 1, GL_FALSE, glm::value_ptr(projection * this->transform_matrix));

	//Draw baby!
	glBindVertexArray(this->vao);
	glDrawArrays(GL_TRIANGLES, 0, this->mesh->verticies_count);
	glBindVertexArray(0);
}

void D3DWorldObject::update_coord_x(float val) {
	this->x = val;
	transform_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(x,y,z));
}

void D3DWorldObject::update_coord_y(float val) {
	this->y = val;
	transform_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(x,y,z));
}

void D3DWorldObject::update_coord_z(float val) {
	this->z = val;
	transform_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(x,y,z));
}


