#define GLM_FORCE_RADIANS

#define X_STRIDE 0.3f
#define Y_STRIDE 0.3f
#define GRID_SIZE 20

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <thread>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#include "draw.h"

class GLUint;

void print_compilation_error(unsigned int shader) {
	char buffer[512];
	glGetShaderInfoLog(shader, 512, NULL, buffer);
	printf("Compilation Error:\n\n%s\n", buffer);
}

std::string read_file_to_cstr(const char* filename)
{
	std::ifstream fs(filename);
	std::stringstream ss;
	ss << fs.rdbuf();
	std::string frag = ss.str();
	return frag;
}

unsigned int compile_fragment_shader()
{
	GLint status;
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	std::string frag = read_file_to_cstr("frag.glsl");
	const char* frag_src = frag.c_str();

	glShaderSource(fragmentShader, 1, &frag_src, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
	if (status == GL_TRUE) {
		printf("compiled fragment shader\n"); 
	} else {
		printf("fragment shader compilation failed!\n");
		print_compilation_error(fragmentShader);
		exit(1);
	}

	return fragmentShader;
}

unsigned int compile_vertex_shader()
{
	GLint status;
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

	std::string vert = read_file_to_cstr("vert.glsl");
	const char* vert_src = vert.c_str();

	glShaderSource(vertexShader, 1, &vert_src, NULL);
	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	if (status == GL_TRUE) {
		printf("compiled vertex shader\n"); 
	} else {
		printf("vertex shader compilation failed!\n");
		print_compilation_error(vertexShader);
		exit(1);
	}

	return vertexShader;
}


unsigned int compile_geometry_shader()
{
	GLint status;
	GLuint geometryShader = glCreateShader(GL_GEOMETRY_SHADER);

	std::string geom = read_file_to_cstr("geom.glsl");
	const char* geom_src = geom.c_str();

	glShaderSource(geometryShader, 1, &geom_src, NULL);
	glCompileShader(geometryShader);

	glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &status);
	if (status == GL_TRUE) {
		printf("compiled geometry shader\n"); 
	} else {
		printf("geometry shader compilation failed!\n");
		print_compilation_error(geometryShader);
		exit(1);
	}

	return geometryShader;
}

glm::vec3 get_translation(int x, int y, float time) {
	return glm::vec3(x*X_STRIDE, y*Y_STRIDE, 1.5f *sin(0.5f * time + x/7.0f + y/9.0f + 1.6f));
}

glm::vec3 get_color(int x, int y, float time) {
	return glm::vec3(sin(0.2f * time + x/20.0f + y/20.0f), 0.7f, 1.0f);
}

void init_metadata(char *filename, std::vector<unsigned> &triangles)
{
	std::ifstream in(filename);
	unsigned a, b, c;
	while (in >> a >> b >> c)
	{
		triangles.push_back(a);
		triangles.push_back(b);
		triangles.push_back(c);
	}
}

int main(int argc, char **argv)
{

    if (argc != 7) {
        fprintf(stderr, "need indir, metadata file, num_vertices, num_frames, frames_per_chunk, tolerance\n");
        exit(1);
    }

	// Load in all of the triangles
	std::vector<GLuint> triangles;
	init_metadata(argv[2], triangles);
	char *indir = argv[1];
	size_t num_vertices = (size_t) atoi(argv[3]);
	size_t num_frames = (size_t) atoi(argv[4]);
	size_t frames_per_chunk = (size_t) atoi(argv[5]);
	float tolerance = atof(argv[6]);
	init_decoder(indir, num_vertices, num_frames, frames_per_chunk, tolerance);

	float *frame;

	frame = (float *) malloc(sizeof(float) * 3 * num_vertices * frames_per_chunk);
	if (!frame)
	{
		fprintf(stderr, "couldn't frame vertex buffer");
		exit(1);
	}

	glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "ripples", nullptr, nullptr);

    glfwMakeContextCurrent(window);

    // Set up glew
    glewExperimental = GL_TRUE;
    glewInit();

    // Set up the vertex array object to save
    // how we set up attributes for our shader
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Set up the main vertex buffer
    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);

    // Compile the shaders
    GLuint fragmentShader = compile_fragment_shader();
    GLuint vertexShader = compile_vertex_shader();
    GLuint geometryShader = compile_geometry_shader();
 
    // Initializing the shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glAttachShader(shaderProgram, geometryShader);

    // select an output from the fragment shader (unnecessary here since there's only one)
    glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

	get_frame(frame);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, num_vertices*3, frame, GL_DYNAMIC_DRAW);

    // identify the position attribute in our vertex buffer
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);   
    glEnableVertexAttribArray(posAttrib);
 
    glm::mat4 view = glm::lookAt(
        glm::vec3(3.0f, 3.0f, 1.8f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );

    GLint uniView = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
    
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.01f, 10.0f);
    GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(GLuint), &triangles[0], GL_STATIC_DRAW);

    GLint uniModel = glGetUniformLocation(shaderProgram, "model");
    GLint uniFade = glGetUniformLocation(shaderProgram, "Fade");

    glEnable(GL_DEPTH_TEST);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	GLint colorAttrib = glGetAttribLocation(shaderProgram, "color");

	double curtime, lasttime = 0;

    while(!glfwWindowShouldClose(window))
    {
		lasttime = curtime;
		glBufferData(GL_ARRAY_BUFFER, num_vertices*3*sizeof(float), frame, GL_DYNAMIC_DRAW);
		get_frame(frame);

		glm::mat4 model;
		model = glm::rotate(model, (float)curtime*glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

		glfwSwapBuffers(window);
        glfwPollEvents();
        
		curtime = glfwGetTime();
		while (1.0f/(curtime - lasttime) > 60.0f) {
			curtime = glfwGetTime();
		}

		printf("%f\n", 1.0f/(curtime - lasttime));

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawElements(GL_TRIANGLES, triangles.size(), GL_UNSIGNED_INT, 0);

    }

	free(frame);
    glfwTerminate();
}
