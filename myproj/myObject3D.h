#include <math.h>
#include <GL/glew.h>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <map>
#define PI 3.14159265

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>

#include "myMaterial.h"
#include "mySubObject3D.h"
#include "myTexture.h"

using namespace std;

bool sortPart(mySubObject3D* part1, mySubObject3D* part2)
{
	return (part1->material->opacity > part2->material->opacity);
}

class myObject3D
{
public:
	GLuint buffers[6];
	enum { VERTEX_BUFFER = 0, INDEX_BUFFER, NORMAL_BUFFER };

	myMaterial *material; //contains the color description of the object.

	vector<glm::vec3> vertices;
	vector<glm::ivec3> indices;
	vector<glm::vec3> normals;
	map<string, myMaterial> materials;
	vector<mySubObject3D *> parts;
	vector<glm::vec2> texturecoordinates;
	map<string, myTexture> textures;

	glm::mat4 model_matrix;

	myObject3D() {
		model_matrix = glm::mat4(1.0f);
		clear();
	}

	~myObject3D()
	{
		clear();
	}

	void clear() {		
		vertices.clear();
		indices.clear();
		normals.clear();
		parts.clear();
		texturecoordinates.clear();
		materials.clear();
		textures.clear();
		glDeleteBuffers(6, buffers);
	}

	void normalize()
	{
		unsigned int tmpxmin = 0, tmpymin = 0, tmpzmin = 0, tmpxmax = 0, tmpymax = 0, tmpzmax = 0;

		for (unsigned i = 0; i<vertices.size(); i++) {
			if (vertices[i].x < vertices[tmpxmin].x) tmpxmin = i;
			if (vertices[i].x > vertices[tmpxmax].x) tmpxmax = i;

			if (vertices[i].y < vertices[tmpymin].y) tmpymin = i;
			if (vertices[i].y > vertices[tmpymax].y) tmpymax = i;

			if (vertices[i].z < vertices[tmpzmin].z) tmpzmin = i;
			if (vertices[i].z > vertices[tmpzmax].z) tmpzmax = i;
		}

		float xmin = vertices[tmpxmin].x, xmax = vertices[tmpxmax].x,
			   ymin = vertices[tmpymin].y, ymax = vertices[tmpymax].y,
			   zmin = vertices[tmpzmin].z, zmax = vertices[tmpzmax].z;

		float scale = (xmax - xmin) <= (ymax - ymin) ? (xmax - xmin) : (ymax - ymin);
		scale = scale >= (zmax - zmin) ? scale : (zmax - zmin);

		for (unsigned int i = 0; i<vertices.size(); i++) {
			vertices[i].x -= (xmax + xmin) / 2;
			vertices[i].y -= (ymax + ymin) / 2;
			vertices[i].z -= (zmax + zmin) / 2;

			vertices[i].x /= scale;
			vertices[i].y /= scale;
			vertices[i].z /= scale;
		}
	}

	//Reads a .PPM image file and returns it in a byte array together with its image width and height.
	GLubyte * readPPMfile(char *filename, int & width, int & height)
	{
		FILE *inFile;
		char buffer[100];
		unsigned char c;
		int maxVal;

		GLubyte *mytexture;
		int pixelsize;

		if ((inFile = fopen(filename, "rb")) == NULL) {
			return 0;
		}

		//Read file type identifier (magic number)
		fgets(buffer, sizeof(buffer), inFile);
		if ((buffer[0] != 'P') || (buffer[1] != '6')) {
			fprintf(stderr, "not a binary ppm file %s\n", filename);
			return 0;
		}

		if (buffer[2] == 'A')
			pixelsize = 4;
		else
			pixelsize = 3;

		//Read image size
		do fgets(buffer, sizeof(buffer), inFile);
		while (buffer[0] == '#');
		sscanf(buffer, "%d %d", &width, &height);

		//Read maximum pixel value (usually 255)
		do fgets(buffer, sizeof(buffer), inFile);
		while (buffer[0] == '#');
		sscanf(buffer, "%d", &maxVal);

		//Allocate RGBA texture buffer
		int memSize = width * height * 4 * sizeof(GLubyte);
		mytexture = new GLubyte[memSize];

		// read RGB data and set alpha value
		for (int i = 0; i < memSize; i++) {
			if ((i % 4) < 3 || pixelsize == 4) {
				c = fgetc(inFile);
				mytexture[i] = (GLubyte)c;
			}
			else mytexture[i] = (GLubyte)255; //Set alpha to opaque
		}
		fclose(inFile);
		return mytexture;
	}


	void computeTexturecoordinates_plane()
	{
		texturecoordinates.assign(vertices.size(), glm::vec2(0.0f, 0.0f));
		for (unsigned int i = 0; i < vertices.size(); i++)
		{
			texturecoordinates[i].x = vertices[i].x * 4;
			texturecoordinates[i].y = vertices[i].y * 4;
		}
	}

	void computeTexturecoordinates_cylinder()
	{
		texturecoordinates.assign(vertices.size(), glm::vec2(0.0f, 0.0f));
		GLfloat x, y, z;
		for (unsigned int i = 0; i < vertices.size(); i++)
		{
			x = vertices[i].x;
			y = vertices[i].y;
			z = vertices[i].z;

			texturecoordinates[i].t = y - 0.5f;
			texturecoordinates[i].s = (z >= 0.0f) ? atan2(z, x) / (PI) : (-atan2(z, x)) / (PI);
		}
	}

	void computeTexturecoordinates_sphere()
	{
		texturecoordinates.assign(vertices.size(), glm::vec2(0.0f, 0.0f));
		GLfloat x, y, z;
		for (unsigned int i = 0; i < vertices.size(); i++)
		{
			x = vertices[i].x;
			y = vertices[i].y;
			z = vertices[i].z;

			texturecoordinates[i].t = (y >= 0.0f) ? atan2(y, z) / (PI) : (-atan2(y, z)) / (PI);
			texturecoordinates[i].s = (z >= 0.0f) ? atan2(z, x) / (PI) : (-atan2(z, x)) / (PI);
		}
	}


	bool readMesh(string filename)
	{
		clear();
		string s, t;
		float x, y, z;
		int index1, index2, index3;

		ifstream fin(filename);
		if (!fin.is_open())
		{
			cout << "Error: unable to open file in readMesh().\n";
			return false;
		}

		while (getline(fin, s))
		{
			stringstream myline(s);
			myline >> t;
			if (t == "v")
			{
				myline >> x;  myline >> y; myline >> z;
				vertices.push_back(glm::vec3(x, y, z));
			}
			else if (t == "f")
			{
				myline >> t; index1 = atoi((t.substr(0, t.find("/"))).c_str());
				myline >> t; index2 = atoi((t.substr(0, t.find("/"))).c_str());
				while (myline >> t)
				{
					index3 = atoi((t.substr(0, t.find("/"))).c_str());
					indices.push_back(glm::ivec3(index1-1, index2-1, index3-1));
					index2 = index3;
				}
			}
		}
		return true;
	}

	bool readScene(string filename)
	{
		clear();
		string s, t, name;
		float x, y, z;
		int index1, index2, index3;
		int tindex1, tindex2, tindex3;
		myMaterial *material = NULL;
		myTexture *texture = NULL;
		mySubObject3D *part = NULL;

		ifstream finmtl(filename + ".mtl");
		if (finmtl.is_open())
		{

			while (getline(finmtl, s))
			{
				stringstream myline(s);
				myline >> t;
				if (t == "newmtl")
				{
					if (material != NULL)
					{
						materials[material->material_name] = *material;
					}
					myline >> name;
					material = new myMaterial(name);
				}
				else if (t == "Ka")
				{
					myline >> x;  myline >> y; myline >> z;
					material->material_Ka = glm::vec4(x, y, z, 1.0f);
				}
				else if (t == "Kd")
				{
					myline >> x;  myline >> y; myline >> z;
					material->material_Kd = glm::vec4(x, y, z, 1.0f);
				}
				else if (t == "Ks")
				{
					myline >> x;  myline >> y; myline >> z;
					material->material_Ks = glm::vec4(x, y, z, 1.0f);
				}
				else if (t == "Ni")
				{
					myline >> x;
					material->material_Sh = x;
				}
				else if (t == "opacity")
				{
					myline >> x;
					material->opacity = x;
				}
				if (t == "map_Kd")
				{
					myline >> name;
					char* cstr = new char[name.length() + 1];
					strcpy(cstr, name.c_str());

					texture = new myTexture(0, 0, 10);
					GLubyte *ppm = readPPMfile(cstr, texture->width, texture->height);
					texture->texName = createTextureBuffer(ppm, texture->width, texture->height);

					textures[name] = *texture;
					material->tex = texture;
				}
			}

			materials[material->material_name] = *material;

			finmtl.close();
		}

		ifstream finobj(filename + ".obj");
		if (!finobj.is_open())
		{
			cout << "Error: unable to open .obj file in readScene().\n";
			return false;
		}

		while (getline(finobj, s))
		{
			stringstream myline(s);
			myline >> t;
			if (t == "g")
			{
				if (part != NULL)
				{
					part->end_index = indices.size();
					if (part->end_index - part->start_index > 0)
						parts.push_back(part);
				}
				part = new mySubObject3D();
				part->start_index = indices.size();
			}
			else if (t == "usemtl")
			{
				myline >> name;
				part->material = &materials[name];
			}
			else if (t == "v")
			{
				myline >> x;  myline >> y; myline >> z;
				vertices.push_back(glm::vec3(x, y, z));
			}
			else if (t == "vt")
			{
				myline >> x;  myline >> y; myline >> z;
				texturecoordinates.push_back(glm::vec2(x, y));
			}
			else if (t == "f")
			{
				myline >> t; 
				index1 = atoi((t.substr(0, t.find("/"))).c_str());
				myline >> t; 
				index2 = atoi((t.substr(0, t.find("/"))).c_str());
				while (myline >> t)
				{
					index3 = atoi((t.substr(0, t.find("/"))).c_str());
					//cout << index1 << "," << index2 << "," << index3 << endl;
					indices.push_back(glm::ivec3(index1 - 1, index2 - 1, index3 - 1));
					index2 = index3;
				}
			}
		}

		part->end_index = indices.size();
		if (part->end_index - part->start_index > 0)
			parts.push_back(part);

		finobj.close();

		// Sorting parts by transparency
		sort(parts.begin(), parts.end(), sortPart);

		return true;
	}
 
	void computeNormals()
	{
		normals.assign(vertices.size(), glm::vec3(0.0f, 0.0f, 0.0f));
		for (unsigned int i = 0; i<indices.size(); i++)
		{
			glm::vec3 face_normal = glm::cross( vertices[indices[i][1]] - vertices[indices[i][0]], vertices[indices[i][2]] - vertices[indices[i][1]] );
			normals[indices[i][0]] += face_normal;
			normals[indices[i][1]] += face_normal;
			normals[indices[i][2]] += face_normal;
		}
		for (unsigned int i = 0; i < vertices.size(); i++)  normals[i] = glm::normalize(normals[i]);
	}

	//Creates a texture buffer on the GPU, uploads the image file and returns the opengl id of the buffer.
	GLuint createTextureBuffer(GLubyte *mytexture, int width, int height)
	{
		GLuint texName;
		glGenTextures(1, &texName);
		glBindTexture(GL_TEXTURE_2D, texName);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLuint)width, (GLuint)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, mytexture);

		glBindTexture(GL_TEXTURE_2D, 0);
		return texName;
	}

	void createObjectBuffers()
	{
		glGenBuffers(6, buffers);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[3]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(glm::ivec3), &indices.front(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[4]);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[5]);
		glBufferData(GL_ARRAY_BUFFER, texturecoordinates.size() * sizeof(glm::vec2), &texturecoordinates[0], GL_STATIC_DRAW);
	}

	void displayObject(GLuint shaderprogram, glm::mat4 viewmatrix)
	{
		// Materials
		glUniform4fv(glGetUniformLocation(shaderprogram, "kd"), 1, glm::value_ptr(material->material_Kd));
		glUniform4fv(glGetUniformLocation(shaderprogram, "ks"), 1, glm::value_ptr(material->material_Ks));
		glUniform4fv(glGetUniformLocation(shaderprogram, "ka"), 1, glm::value_ptr(material->material_Ka));
		glUniform1f(glGetUniformLocation(shaderprogram, "sh"), material->material_Sh);

		// vertices
		glEnableVertexAttribArray(0 /* position 0 */);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
		glVertexAttribPointer(0 /* position 0 */, 3, GL_FLOAT, GL_FALSE, 0, 0);

		// Textures
		//glUniform1i(glGetUniformLocation(shaderprogram, "tex"), 8);
		//glActiveTexture(GL_TEXTURE8);
		//glBindTexture(GL_TEXTURE_2D, texture->texName);

		//// Textures coordinates
		//glEnableVertexAttribArray(2 /* position 2 */);
		//glBindBuffer(GL_ARRAY_BUFFER, buffers[5]);
		//glVertexAttribPointer(2 /* position 2 */, 2, GL_FLOAT, GL_FALSE, 0, 0);

		// indices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[3]);
		glDrawElements(GL_TRIANGLES, indices.size() * 3, GL_UNSIGNED_INT, (GLvoid *) (sizeof(glm::vec3) * 1));

		// normals
		glEnableVertexAttribArray(1 /* position 1 */);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[4]);
		glVertexAttribPointer(1 /* position 1 */, 3, GL_FLOAT, GL_FALSE, 0, 0);


		glUniformMatrix4fv(glGetUniformLocation(shaderprogram, "mymodel_matrix"),
			1, GL_FALSE, &model_matrix[0][0]);

		glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(viewmatrix * model_matrix)));
		glUniformMatrix3fv(glGetUniformLocation(shaderprogram, "mynormal_matrix"), 1,
			GL_FALSE, &normal_matrix[0][0]);
		 
	}

	void displayScene(GLuint shaderprogram, glm::mat4 viewmatrix)
	{

		// vertices
		glEnableVertexAttribArray(0 /* position 0 */);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
		glVertexAttribPointer(0 /* position 0 */, 3, GL_FLOAT, GL_FALSE, 0, 0);

		// normals
		glEnableVertexAttribArray(1 /* position 1 */);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[4]);
		glVertexAttribPointer(1 /* position 1 */, 3, GL_FLOAT, GL_FALSE, 0, 0);

		// Textures 
		glUniform1i(glGetUniformLocation(shaderprogram, "tex"), 8);
		glActiveTexture(GL_TEXTURE8);

		// Textures coordinates
		glEnableVertexAttribArray(2 /* position 2 */);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[5]);
		glVertexAttribPointer(2 /* position 2 */, 2, GL_FLOAT, GL_FALSE, 0, 0);

		// indices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[3]);

		for (size_t i = 0; i < parts.size(); i++)
		{
			// Textures
			glUniform1f(glGetUniformLocation(shaderprogram, "opacity"), parts[i]->material->opacity);
			if (parts[i]->material->tex != NULL) {
				glUniform1i(glGetUniformLocation(shaderprogram, "draw_texture"), 2);
				glBindTexture(GL_TEXTURE_2D, parts[i]->material->tex->texName);
			}
			else {
				glUniform1i(glGetUniformLocation(shaderprogram, "draw_texture"), 0);
			}

			parts[i]->material->glMaterial();
			glUniform4fv(glGetUniformLocation(shaderprogram, "kd"), 1, glm::value_ptr(parts[i]->material->material_Kd));
			glUniform4fv(glGetUniformLocation(shaderprogram, "ks"), 1, glm::value_ptr(parts[i]->material->material_Ks));
			glUniform4fv(glGetUniformLocation(shaderprogram, "ka"), 1, glm::value_ptr(parts[i]->material->material_Ka));
			glUniform1f(glGetUniformLocation(shaderprogram, "sh"), parts[i]->material->material_Sh);

			glDrawElements(GL_TRIANGLES, (parts[i]->end_index - parts[i]->start_index) * 3, GL_UNSIGNED_INT, (GLvoid *)(parts[i]->start_index * sizeof(glm::ivec3)));
		}


		glUniformMatrix4fv(glGetUniformLocation(shaderprogram, "mymodel_matrix"),
			1, GL_FALSE, &model_matrix[0][0]);

		glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(viewmatrix * model_matrix)));
		glUniformMatrix3fv(glGetUniformLocation(shaderprogram, "mynormal_matrix"), 1,
			GL_FALSE, &normal_matrix[0][0]);
	}

	void displayNormals()
	{
		glBegin(GL_LINES);
		for (unsigned int i = 0; i<vertices.size(); i++)
		{
			glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
			glVertex3f(vertices[i].x + normals[i].x / 10.0f, vertices[i].y + normals[i].y / 10.0f, vertices[i].z + normals[i].z / 10.0f);
		}
		glEnd();
	}

	void translate(double x, double y, double z)
	{
		glm::mat4 tmp = glm::translate(model_matrix, glm::vec3(x, y, z));
		model_matrix = tmp * model_matrix;
	}

	void rotate(double axis_x, double axis_y, double axis_z, double angle)
	{
		glm::mat4 tmp = glm::rotate(model_matrix, (float) angle, glm::vec3(axis_x, axis_y, axis_z));
		model_matrix = tmp * model_matrix;
	}
};