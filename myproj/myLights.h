#pragma once

#include <math.h>
#include <GL/glew.h>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>     

using namespace std;

class myLights
{
public:
	vector<glm::vec4> positions;
	vector<glm::vec4> colors;
	vector<glm::vec3> directions;
	vector<int> types; //0 = point light, 1 = directional light, 2 = spot light.
	GLuint shaderprogram1;


	myLights(GLuint shaderprogram)
	{
		shaderprogram1 = shaderprogram;
	}

	void addLight(glm::vec4 position, glm::vec4 color, glm::vec3 direction, GLuint type)
	{
		positions.push_back(position);
		colors.push_back(color);
		directions.push_back(direction);
		types.push_back(type);
	}

	void drawLight(int i)
	{
		glm::vec3 direction = directions[i];
		glm::vec4 color = colors[i];
		int type = types[i];
		glm::vec4 position = positions[i];
		glUniform1i(glGetUniformLocation(shaderprogram1, "to_draw"), 2);
		float length = sqrt(direction[0] * direction[0] + direction[1] * direction[1]
			+ direction[2] * direction[2]);
		if (length != 0) for (int i = 0; i<3; i++) direction[i] /= length;
		glPointSize(6.0);
		glUniform4fv(glGetUniformLocation(shaderprogram1, "kd"), 1, glm::value_ptr(color));
		if (type == 0 || type == 2) {
			glBegin(GL_POINTS);
			glVertex3f(position[0], position[1], position[2]);
			glEnd();
		}
		if (type == 1) {
			glBegin(GL_LINES);
			glVertex3f(0, 0, 0);
			glVertex3f(direction[0], direction[1], direction[2]);
			glEnd();
		}
		if (type == 2) {
			glBegin(GL_LINES);
			glVertex3f(position[0], position[1], position[2]);
			glVertex3f(position[0] + direction[0] / 2.0, position[1] + direction[1] / 2.0,
				position[2] + direction[2] / 2.0);
			glEnd();
		}
		glUniform1i(glGetUniformLocation(shaderprogram1, "to_draw"), 0);
	}
};