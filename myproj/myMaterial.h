#pragma once

#include <math.h>
#include <GL/glew.h>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "myTexture.h"

using namespace std;

class myMaterial
{
public:
	glm::vec4 material_Kd; //diffuse color
	glm::vec4 material_Ks; //specular color
	glm::vec4 material_Ka; //ambient color
	GLfloat material_Sh; //shininess coefficient
	string material_name; //name of the material
	myTexture *tex = nullptr;
	GLfloat opacity = 1.0f;

	myMaterial()
	{}

	myMaterial(string name)
	{
		material_name = name;
		material_Ka = { 0, 1.0f, 0, 0 };
		material_Kd = { 0, 0, 0, 0 };
		material_Ks = { 0, 0, 0, 0 };
		material_Sh = 0;
	}

	myMaterial(glm::vec4 Ka, glm::vec4 Kd, glm::vec4 Ks, GLfloat Sh)
	{
		material_Ka = Ka;
		material_Kd = Kd;
		material_Ks = Ks;
		material_Sh = Sh;
	}

	void glMaterial()
	{
		glMaterialfv(GL_FRONT, GL_AMBIENT, glm::value_ptr(material_Ka));
		glMaterialfv(GL_FRONT, GL_DIFFUSE, glm::value_ptr(material_Kd));
		glMaterialfv(GL_FRONT, GL_SPECULAR, glm::value_ptr(material_Ks));
		glMaterialf(GL_FRONT, GL_SHININESS, material_Sh);
	}
};
