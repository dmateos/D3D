#include "main.h"

using namespace std;

// Sets the FPS text on a glfwindow
void _update_fps_counter (GLFWwindow* window) {
	static double previous_seconds = glfwGetTime ();
	static int frame_count;
	double current_seconds = glfwGetTime ();
	double elapsed_seconds = current_seconds - previous_seconds;
	if (elapsed_seconds > 0.25) {
		previous_seconds = current_seconds;
		double fps = (double)frame_count / elapsed_seconds;
		char tmp[128];
		sprintf (tmp, "opengl @ fps: %.2lf", fps);
		glfwSetWindowTitle (window, tmp);
		frame_count = 0;
	}
	frame_count++;
}

//Compiles a GLSL shader from a source file
GLuint make_shader(const char *path, GLenum stype) {
	int length;
	GLchar *data = (GLchar*)file_contents(path, &length);
	GLint shader_ok;
	GLuint shader = glCreateShader(stype);

	glShaderSource(shader, 1, (const GLchar**)&data, &length);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
	if(!shader_ok) {
		printf("shader %s did not compile\n", path);
		glDeleteShader(shader);
		return 1;
	}

	free(data);
	return shader;
}

//Makes a gl program out of the given shader programs
GLint make_program(GLint vshader, GLint fshader) {
	GLint shader_program = glCreateProgram();
	glAttachShader(shader_program, vshader);
	glAttachShader(shader_program, fshader);
	glLinkProgram(shader_program);

	GLint program_ok;
	glGetProgramiv(shader_program, GL_LINK_STATUS, &program_ok);
	if(!program_ok) {
		printf("program did not compile\n");
		return 1;
	}

	return shader_program;
}

//Store some persistant global variables other functions need access too
std::vector<D3DWorldObject*> bullets;
D3DWorldObject *playerptr;
float x,y,z, xc, yc, zc;
bool keypress;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	float step = 0.5;
	switch(key) {
		case GLFW_KEY_W:
			z -= step;
			break;
		case GLFW_KEY_S:
			z += step;
			break;
		case GLFW_KEY_A:
			x -= step;
			break;
		case GLFW_KEY_D:
			x += step;
			break;
		case GLFW_KEY_DOWN:
			zc -= step;
			break;
		case GLFW_KEY_UP:
			zc += step;
			break;
		case GLFW_KEY_LEFT:
			xc -= step;
			break;
		case GLFW_KEY_RIGHT:
			xc += step;
			break;
		case GLFW_KEY_E:
			y += step;
			break;
		case GLFW_KEY_Q:
			y -= step;
			break;
		case GLFW_KEY_SPACE:
			bullets.push_back(new D3DWorldObject("meshes/circle.dae",
								playerptr->get_x(),
								playerptr->get_y(),
								playerptr->get_z()));
			break;
	}
	keypress = true;
}

std::vector<D3DWorldObject*> network_objects;

void new_coords(int oid, float x, float y, float z) {
	//Here we must make a new object if it dont exsist
	//else we update the coords.
	//TODO destruct

	D3DWorldObject *found_obj = NULL;
	for (std::vector<D3DWorldObject*>::iterator it = network_objects.begin() ; it != network_objects.end(); ++it) {
		if((*it)->oid == oid) {
			found_obj = *it;
		}
	}

	if(found_obj == NULL) {
		D3DWorldObject *obj = new D3DWorldObject("meshes/monkey.dae", x, y ,z);
		obj->oid = oid;
		network_objects.push_back(obj);
	} else {
		found_obj->update_coord_x(x);
		found_obj->update_coord_y(y);
		found_obj->update_coord_z(z);
	}
}

int main(int argc, char **argv) {
	srand(time(NULL));
	if(!glfwInit()) {
		fprintf(stderr, "error: could not start glfw3\n");
		return 1;
	}

	// Lets try to tell the Operating System what version of openGL we like
	glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Make a window and some more gl init stuff.
	GLFWwindow *window = glfwCreateWindow(1024, 768, "Hello world", NULL, NULL);
	if(!window) {
		fprintf(stderr, "erorr: could not open window with glfw3\n");

		return 1;
	}

	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	glewInit();

	//Enable the opengl depth buffers for 3d.
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	//glEnable(GL_CULL_FACE);

	// Lets print the version of opengl
	const GLubyte* renderer = glGetString (GL_RENDERER);
	const GLubyte* version = glGetString (GL_VERSION);
	printf("Renderer: %s\n", renderer);
	printf ("OpenGL version supported %s\n", version);

	//Compile our vertex and fragment shader and setup a
	//perspective matrix (the lense of the camera)
	GLint vshader = make_shader("shaders/vshader.glsx", GL_VERTEX_SHADER);
	GLint fshader = make_shader("shaders/fshader.glsx", GL_FRAGMENT_SHADER);
	GLint shader_program = make_program(vshader, fshader);
	glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);

	//Socketsss
	Connection server_connection("127.0.0.1");
	server_connection.new_coords = &new_coords;

	D3DWorldObject player("meshes/monkey.dae", 0.0f, 0.0f, 5.0f);
	D3DWorldObject obj2("meshes/torus.dae", 0.0f, 0.0f, -5.0f);
	D3DWorldObject obj3("meshes/torus.dae", -3.0f, 0.0f, -5.0f);
	D3DWorldObject obj4("meshes/torus.dae", 3.0f, 0.0f, -5.0f);
	playerptr = &player;

	while(!glfwWindowShouldClose(window)) {
		_update_fps_counter(window);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//This is our camera position matrix
		glm::mat4 view = glm::lookAt(
			glm::vec3(xc ,zc, +10.0f), // Camera is at (4,3,3), in World Space
			glm::vec3(0.0f,0.0f,0.0f), // and looks at the origin
			glm::vec3(0.0f, 1.0f,0.0f)  // Head is up (set to 0,-1,0 to look upside-down)
		);

		glm::mat4 vp = projection * view;

		//Update the coords of our main charecter
		if(keypress == true) {
			player.update_coord_x(x);
			player.update_coord_y(y);
			player.update_coord_z(z);
			//TODO check for con to see if we should bother doing this.
			server_connection.send_coord_update(player.oid, player.get_x(), player.get_y(), player.get_z());
			keypress = false;
		}

		//Render
		player.draw(shader_program, vp);
		obj2.draw(shader_program, vp);
		obj3.draw(shader_program, vp);
		obj4.draw(shader_program, vp);

	  	for (std::vector<D3DWorldObject*>::iterator it = bullets.begin() ; it != bullets.end(); ++it) {
		//	(*it)->update_coord_z((*it)->get_z() - 0.10);
			(*it)->draw(shader_program, vp);
			//TODO can we cull these as they move too far
		}

		for(std::vector<D3DWorldObject*>::iterator it = network_objects.begin(); it != network_objects.end(); ++it) {
			(*it)->draw(shader_program, vp);
		}

		server_connection.get_message();

		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}
