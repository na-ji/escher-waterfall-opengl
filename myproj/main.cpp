#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>     
#include <glm/ext.hpp> 

#include <GL/glew.h>

#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#undef main

// #include "NFD/nfd.h"

#include "shaders.h"
#include "helperFunctions.h"
#include "myLights.h"
#include "myMaterial.h"

#include "myObject3D.h"

using namespace std;

// SDL variables
SDL_Window* window;
SDL_GLContext glContext;

int window_width = 1280;
int window_height = 720;

int mouse_position[2];
bool mouse_button_pressed = false;
bool quit = false;

// Camera parameters.
glm::vec3 camera_eye = glm::vec3(-1.39f, 0.74f, 1.21f);
glm::vec3 camera_up = glm::vec3(0.34f, 0.89f, -0.30f);
glm::vec3 camera_forward = glm::vec3(0.68f, -0.44f, -0.59f);

// Projection parameters.
float fovy = 45.0f;
float zNear = 0.1f;
float zFar = 4000;

// OpenGL shader variables
GLuint vertexshader, fragmentshader, shaderprogram1;

 // Mesh object
myObject3D *obj1;

// Lights
myLights *current_lights;


// Update the title of the window
void updateWindowTitle()
{
	string title = "IT-5102E-16 - ";
	title += glm::to_string(camera_eye);
	title += " " + glm::to_string(camera_up);
	title += " " + glm::to_string(camera_forward);
	char title2[1024];
	strncpy(title2, title.c_str(), sizeof(title2));
	SDL_SetWindowTitle(window, title2);
}

// Process the event.  
void processEvents(SDL_Event current_event)
{
	switch (current_event.type)
	{
		// window close button is pressed
		case SDL_QUIT:
		{
			quit = true;
			break;
		}
		case SDL_KEYDOWN:
		{
			if (current_event.key.keysym.sym == SDLK_ESCAPE)
				quit = true;
			else if (current_event.key.keysym.sym == SDLK_UP)
				camera_eye += 0.1f * camera_forward;
			else if (current_event.key.keysym.sym == SDLK_DOWN)
				camera_eye -= 0.1f * camera_forward;
			else if (current_event.key.keysym.sym == SDLK_LEFT)
				rotate(camera_forward, camera_up, 0.05f, true);
			else if (current_event.key.keysym.sym == SDLK_RIGHT)
				rotate(camera_forward, camera_up, -0.05f, true);
			// else if (current_event.key.keysym.sym == SDLK_o)
			// {
			// 	nfdchar_t *outPath = NULL;
			// 	nfdresult_t result = NFD_OpenDialog("obj", NULL, &outPath);
			// 	if (result != NFD_OKAY) return;
			// 	myObject3D *obj_tmp = new myObject3D();
			// 	if (!obj_tmp->readMesh(outPath))
			// 	{
			// 		delete obj_tmp;
			// 		return;
			// 	}
			// 	delete obj1;
			// 	obj1 = obj_tmp;
			// 	obj1->normalize();
			// 	obj1->computeNormals();
			// 	obj1->createObjectBuffers();
			// }
			else if (current_event.key.keysym.sym == SDLK_l)
			{
				for (unsigned int i = 0; i < current_lights->types.size(); i++)
				{
					current_lights->types[i]++;
					if (current_lights->types[i] > 2)
						current_lights->types[i] = 0;
					cout << current_lights->types[i] << endl;
				}
			}
			else if (current_event.key.keysym.sym == SDLK_z)
			{
				camera_eye = glm::vec3(-1.39f, 0.74f, 1.21f);
				camera_up = glm::vec3(0.34f, 0.89f, -0.30f);
				camera_forward = glm::vec3(0.68f, -0.44f, -0.59f);
			}
			updateWindowTitle();
			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			mouse_position[0] = current_event.button.x;
			mouse_position[1] = window_height - current_event.button.y;
			mouse_button_pressed = true;
			break;
		}
		case SDL_MOUSEBUTTONUP:
		{
			mouse_button_pressed = false;
			break;
		}
		case SDL_MOUSEMOTION:
		{
			int x = current_event.motion.x;
			int y = window_height - current_event.motion.y;

			int dx = x - mouse_position[0];
			int dy = y - mouse_position[1];

			mouse_position[0] = x;
			mouse_position[1] = y;

			if ( (dx == 0 && dy == 0) || !mouse_button_pressed ) break;

			float vx = (float)dx / (float)window_width;
			float vy = (float)dy / (float)window_height;
			float theta = 4.0f * (fabs(vx) + fabs(vy));

			glm::vec3 camera_right = glm::normalize(glm::cross(camera_forward, camera_up));
			glm::vec3 tomovein_direction = -camera_right * vx + -camera_up * vy;

			if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT))
			{
				glm::vec3 rotation_axis = glm::normalize(glm::cross(tomovein_direction, camera_forward));

				rotate(camera_forward, rotation_axis, theta, true);
				rotate(camera_up, rotation_axis, theta, true);
				rotate(camera_eye, rotation_axis, theta, false);
			}
			else if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				camera_eye += 1.6f * tomovein_direction;
			}
			updateWindowTitle();
			break;
		}
		case SDL_WINDOWEVENT:
		{
			if (current_event.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				window_width = current_event.window.data1;
				window_height = current_event.window.data2;
			}
			break;
		}
		case SDL_MOUSEWHEEL:
		{
			if (current_event.wheel.y < 0)
				camera_eye -= 0.01f * camera_forward;
			else if (current_event.wheel.y > 0)
				camera_eye += 0.01f * camera_forward;
			updateWindowTitle();
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	// Use OpenGL 3.1 core
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	// Initialize video subsystem
	SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);
	// Create window
	window = SDL_CreateWindow("IT-5102E-16", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		window_width, window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	// Create OpenGL context
	glContext = SDL_GL_CreateContext(window);

	// Initialize glew
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);

	// Setting up OpenGL shaders
	vertexshader = initshaders(GL_VERTEX_SHADER, "shaders/light.vert.glsl");
	fragmentshader = initshaders(GL_FRAGMENT_SHADER, "shaders/light.frag.glsl");
	shaderprogram1 = initprogram(vertexshader, fragmentshader);

	updateWindowTitle();

	// Read up the scene
	myMaterial *caca = new myMaterial({ 0.1f, 0.1f, 0.1f, 1.0f } , { 1.0f, 0.0f, 0.0f, 0.0f }, { 1, 1, 1, 0 }, 10.0f);
	myMaterial *bronze = new myMaterial({ 0.2125, 0.1275, 0.054, 1.0 }, { 0.714, 0.4284, 0.18144, 1.0 }, { 0.393548, 0.271906, 0.166721, 1.0 }, 25.6f);
	myMaterial *gold = new myMaterial({ 0.24725, 0.2245, 0.0645, 1.0 }, { 0.34615, 0.3143, 0.0903, 1.0 }, { 0.797357, 0.723991, 0.208006, 1.0 }, 83.2f);
	myMaterial *emerald = new myMaterial({ 0.0215, 0.1745, 0.0215, 0.55 }, { 0.07568, 0.61424, 0.07568, 0.55 }, { 0.633, 0.727811, 0.633, 0.55 }, 76.8f);

	obj1 = new myObject3D();	
	obj1->material = emerald;
	//obj1->texture = texture;
	//if (!obj1->readMesh("apple.obj")) return 0;
	if (!obj1->readScene("escher_waterfall")) return 0;
	obj1->normalize();
	obj1->computeNormals();
	//obj1->computeTexturecoordinates_plane();
	obj1->createObjectBuffers();

	// Create a light
	current_lights = new myLights(shaderprogram1);
	current_lights->addLight(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f), 1);
	current_lights->addLight(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), 1);
	current_lights->addLight(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), 1);
	current_lights->addLight(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 1);
	//current_lights->addLight(glm::vec4(0.0f, 10.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), 0);
	//current_lights->addLight(glm::vec4(0.0f, 0.0f, 10.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), 0);
 
	// Game loop
	while (!quit)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glViewport(0, 0, window_width, window_height);

		glm::mat4 projection_matrix = glm::perspective(glm::radians(fovy), (float) window_width / (float)window_height, zNear, zFar);
		glUniformMatrix4fv(glGetUniformLocation(shaderprogram1, "myprojection_matrix"), 
			               1, GL_FALSE, &projection_matrix[0][0]);

		glm::mat4 view_matrix = glm::lookAt(camera_eye, camera_eye + camera_forward, camera_up);
		glUniformMatrix4fv(glGetUniformLocation(shaderprogram1, "myview_matrix"), 
			               1, GL_FALSE, &view_matrix[0][0]);

		glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(view_matrix)));
		glUniformMatrix3fv(glGetUniformLocation(shaderprogram1, "mynormal_matrix"), 
			               1, GL_FALSE, &normal_matrix[0][0]);

		// Lights
		glUniform4fv(glGetUniformLocation(shaderprogram1, "light_positions"), current_lights->positions.size(), glm::value_ptr(current_lights->positions[0]));
		glUniform4fv(glGetUniformLocation(shaderprogram1, "light_colors"), current_lights->colors.size(), glm::value_ptr(current_lights->colors[0]));
		glUniform3fv(glGetUniformLocation(shaderprogram1, "light_directions"), current_lights->directions.size(), glm::value_ptr(current_lights->directions[0]));
		glUniform1iv(glGetUniformLocation(shaderprogram1, "light_types"), current_lights->types.size(), &current_lights->types[0]);

		glUniform1i(glGetUniformLocation(shaderprogram1, "numberofLights_shader"), current_lights->positions.size());

		for (size_t i = 0; i < current_lights->types.size(); i++)
		{
			current_lights->drawLight(i);
		}

		//obj1->displayObject(shaderprogram1, view_matrix);
		obj1->displayScene(shaderprogram1, view_matrix);
		//obj1->displayNormals();

		SDL_GL_SwapWindow(window);

		SDL_Event current_event;
		while (SDL_PollEvent(&current_event) != 0)
			processEvents(current_event);
	}
	
	// Freeing resources before exiting.
	// Destroy window
	if (glContext) SDL_GL_DeleteContext(glContext);
	if (window) SDL_DestroyWindow(window);

	//Freeing up OpenGL resources.
	delete obj1;
	glDeleteProgram(shaderprogram1);

	// Quit SDL subsystems
	SDL_Quit();

	return 0;
}