#ifndef SHADOWSHADER_H
#define SHADOWSHADER_H

class Matrix4f;
class Vector3f;

#include <glad/glad.h>


class ShadowShader
{
private:
	GLuint programID;
	GLuint vertexShaderID;
	GLuint fragmentShaderID;

	int location_mvpMatrix;

public:
	ShadowShader(char* vertFile, char* fragFile);

	void start();

	void stop();

	void cleanUp();

	void loadMvpMatrix(Matrix4f* mvpMatrix);


protected:
	void bindAttributes();

	void bindAttribute(int, char* attrName);

	void getAllUniformLocations();

	int getUniformLocation(char* uniName);

	void loadMatrix(int, Matrix4f* mat);
};

#endif