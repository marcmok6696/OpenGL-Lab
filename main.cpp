#define _CRT_SECURE_NO_WARNINGS
#define GLM_FORCE_RADIANS

#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"
#include "Dependencies\glm\glm.hpp"
#include "Dependencies\glm\gtc\matrix_transform.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
using namespace std;
using glm::vec3;
using glm::mat4;

GLint programID, programID2;
// Could define the Vao&Vbo and interaction parameter here
struct Particle {
	glm::vec3 pos;
	unsigned char r, g, b, a;
	float size, life, cameradistance;

	bool operator<(const Particle& that) const {
		return this->cameradistance > that.cameradistance;
	}
};

glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
glm::mat4 projectionMatrix = glm::mat4(1.0f);
glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
GLuint planeVao, carVao, boxVao, gsVao;
GLuint planeTex, planeTex2, planeTex3 , planeTex4 , carTex, carTex2, boxTex, gsTex, gsTex2;
size_t planeSize, carSize, boxSize, gsSize;
GLint viewport[4], prevTime, curX, curY, centerX, centerY;
GLuint rotateZ = 0, viewControl = 0, planeChange = 0, carChange = 0, gsChange = 0;
GLfloat horAng = 3.14f, verAng = 0.0f, turnAng = 0.0f, posX = 0.0f, posZ = 0.0f, zAng = 0.0f, diffuseLightIntensity = 0.2f, specularLightIntensity = 0.0f;
GLuint particle_vertex_buffer, particleVao, particles_position_buffer, particles_color_buffer;
GLint curTime, lastTime, mostRecentParticle = 0;
const int maxParticles = 10000;
Particle particlesSet[maxParticles];
static GLfloat* particle_position_size_data = new GLfloat[maxParticles * 4];
static GLubyte* particle_color_data = new GLubyte[maxParticles * 4];

int findAvailableParticle() {
	for (int i = mostRecentParticle; i < maxParticles; i++) {
		if (particlesSet[i].life < 0) {
			mostRecentParticle = i;
			return mostRecentParticle;
		}
	}
	for (int i = 0; i < mostRecentParticle; i++) {
		if (particlesSet[i].life < 0) {
			mostRecentParticle = i;
			return mostRecentParticle;
		}
	}
	return 0;
}

bool checkStatus(
	GLuint objectID,
	PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
	PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
	GLenum statusType)
{
	GLint status;
	objectPropertyGetterFunc(objectID, statusType, &status);
	if (status != GL_TRUE)
	{
		GLint infoLogLength;
		objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* buffer = new GLchar[infoLogLength];

		GLsizei bufferSize;
		getInfoLogFunc(objectID, infoLogLength, &bufferSize, buffer);
		cout << buffer << endl;

		delete[] buffer;
		return false;
	}
	return true;
}

bool checkShaderStatus(GLuint shaderID)
{
	return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool checkProgramStatus(GLuint programID)
{
	return checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

string readShaderCode(const char* fileName)
{
	ifstream meInput(fileName);
	if (!meInput.good())
	{
		cout << "File failed to load..." << fileName;
		exit(1);
	}
	return std::string(
		std::istreambuf_iterator<char>(meInput),
		std::istreambuf_iterator<char>()
	);
}

void installShaders()
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint vertexShaderID2 = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint fragmentShaderID2 = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	string temp = readShaderCode("VertexShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	temp = readShaderCode("ParticleVertexShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID2, 1, adapter, 0);
	temp = readShaderCode("FragmentShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);
	temp = readShaderCode("ParticleFragmentShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID2, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(vertexShaderID2);
	glCompileShader(fragmentShaderID);
	glCompileShader(fragmentShaderID2);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(vertexShaderID2) || !checkShaderStatus(fragmentShaderID) || !checkShaderStatus(fragmentShaderID2))
		return;

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	if (!checkProgramStatus(programID))
		return;


	programID2 = glCreateProgram();
	glAttachShader(programID2, vertexShaderID2);
	glAttachShader(programID2, fragmentShaderID2);
	glLinkProgram(programID2);

	if (!checkProgramStatus(programID2))
		return;

	glDeleteShader(vertexShaderID);
	glDeleteShader(vertexShaderID2);
	glDeleteShader(fragmentShaderID);
	glDeleteShader(fragmentShaderID2);
}

void keyboard(unsigned char key, int x, int y) {
	//TODO: Use keyboard to do interactive events and animation
	if (key == ' ') {
		viewControl = 1 - viewControl;
		if (viewControl == 1) glutSetCursor(GLUT_CURSOR_NONE);
		else glutSetCursor(GLUT_CURSOR_INHERIT);
	}
	if (key == 'q') diffuseLightIntensity = diffuseLightIntensity + 0.1f;
	if (key == 'w') diffuseLightIntensity = diffuseLightIntensity - 0.1f;
	if (key == 'z') specularLightIntensity = specularLightIntensity + 0.1f;
	if (key == 'x') specularLightIntensity = specularLightIntensity - 0.1f;
	if (key == 's') rotateZ = 1 - rotateZ;
	if (key == '1') planeChange = 1;
	if (key == '2') planeChange = 2;
	if (key == '3') planeChange = 3;
	if (key == '4') planeChange = 0;
	if (key == 'v') gsChange = 1 - gsChange;
	if (key == 'b') carChange = 1 - carChange;
}

void move(int key, int x, int y) {
	//TODO: Use arrow keys to do interactive events and animation
	if (key == GLUT_KEY_UP) {
		posX = posX + 0.1f * cos(turnAng);
		posZ = posZ - 0.1f * sin(turnAng);
	}
	if (key == GLUT_KEY_DOWN) {
		posX = posX - 0.1f * cos(turnAng);
		posZ = posZ + 0.1f * sin(turnAng);
	}
	if (key == GLUT_KEY_LEFT) turnAng = turnAng + 0.1f;
	if (key == GLUT_KEY_RIGHT) turnAng = turnAng - 0.1f;
}

void PassiveMouse(int x, int y) {
	//TODO: Use Mouse to do interactive events and animation
	curX = x;
	curY = y;
}

bool loadOBJ(
	const char * path,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE * file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

				   // else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}

	return true;
}

GLuint loadBMP_custom(const char * imagepath) {

	printf("Reading image %s\n", imagepath);

	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	unsigned char * data;

	FILE * file = fopen(imagepath, "rb");
	if (!file) { printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); return 0; }

	if (fread(header, 1, 54, file) != 54) {
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (*(int*)&(header[0x1E]) != 0) { printf("Not a correct BMP file\n");    return 0; }
	if (*(int*)&(header[0x1C]) != 24) { printf("Not a correct BMP file\n");    return 0; }

	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);
	if (imageSize == 0)    imageSize = width * height * 3;
	if (dataPos == 0)      dataPos = 54;

	data = new unsigned char[imageSize];
	fread(data, 1, imageSize, file);
	fclose(file);


	GLuint textureID;
	//TODO: Create one OpenGL texture and set the texture parameter 
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	delete[]data;

	return textureID;
}

void sendDataToOpenGL()
{
	//TODO:
	//Load objects and bind to VAO & VBO
	//Load texture
	std::vector<glm::vec3> planeV, carV, boxV, gsV;
	std::vector<glm::vec2> planeUV, carUV, boxUV, gsUV;
	std::vector<glm::vec3> planeN, carN, boxN, gsN;
	bool res;

	//plane
	res = loadOBJ("plane.obj", planeV, planeUV, planeN);
	glGenVertexArrays(1, &planeVao);
	glBindVertexArray(planeVao);
	GLuint planeVertexVbo;
	glGenBuffers(1, &planeVertexVbo);
	glBindBuffer(GL_ARRAY_BUFFER, planeVertexVbo);
	glBufferData(GL_ARRAY_BUFFER, planeV.size() * sizeof(glm::vec3), &planeV[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	GLuint planeUvVbo;
	glGenBuffers(1, &planeUvVbo);
	glBindBuffer(GL_ARRAY_BUFFER, planeUvVbo);
	glBufferData(GL_ARRAY_BUFFER, planeUV.size() * sizeof(glm::vec2), &planeUV[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	GLuint planeNormalVbo;
	glGenBuffers(1, &planeNormalVbo);
	glBindBuffer(GL_ARRAY_BUFFER, planeNormalVbo);
	glBufferData(GL_ARRAY_BUFFER, planeN.size() * sizeof(glm::vec3), &planeN[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	planeSize = planeV.size();
	planeTex = loadBMP_custom("plane_texture.bmp");
	planeTex2 = loadBMP_custom("theme1.bmp");
	planeTex3 = loadBMP_custom("theme2.bmp");
	planeTex4 = loadBMP_custom("theme3.bmp");

	//car
	res = loadOBJ("car.obj", carV, carUV, carN);
	glGenVertexArrays(1, &carVao);
	glBindVertexArray(carVao);
	GLuint carVertexVbo;
	glGenBuffers(1, &carVertexVbo);
	glBindBuffer(GL_ARRAY_BUFFER, carVertexVbo);
	glBufferData(GL_ARRAY_BUFFER, carV.size() * sizeof(glm::vec3), &carV[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	GLuint carUvVbo;
	glGenBuffers(1, &carUvVbo);
	glBindBuffer(GL_ARRAY_BUFFER, carUvVbo);
	glBufferData(GL_ARRAY_BUFFER, carUV.size() * sizeof(glm::vec2), &carUV[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	GLuint carNormalVbo;
	glGenBuffers(1, &carNormalVbo);
	glBindBuffer(GL_ARRAY_BUFFER, carNormalVbo);
	glBufferData(GL_ARRAY_BUFFER, carN.size() * sizeof(glm::vec3), &carN[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	carSize = carV.size();
	carTex = loadBMP_custom("car_texture.bmp");
	carTex2 = loadBMP_custom("black.bmp");

	//box
	res = loadOBJ("block.obj", boxV, boxUV, boxN);
	glGenVertexArrays(1, &boxVao);
	glBindVertexArray(boxVao);
	GLuint boxVertexVbo;
	glGenBuffers(1, &boxVertexVbo);
	glBindBuffer(GL_ARRAY_BUFFER, boxVertexVbo);
	glBufferData(GL_ARRAY_BUFFER, boxV.size() * sizeof(glm::vec3), &boxV[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	GLuint boxUvVbo;
	glGenBuffers(1, &boxUvVbo);
	glBindBuffer(GL_ARRAY_BUFFER, boxUvVbo);
	glBufferData(GL_ARRAY_BUFFER, boxUV.size() * sizeof(glm::vec2), &boxUV[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	GLuint boxNormalVbo;
	glGenBuffers(1, &boxNormalVbo);
	glBindBuffer(GL_ARRAY_BUFFER, boxNormalVbo);
	glBufferData(GL_ARRAY_BUFFER, boxN.size() * sizeof(glm::vec3), &boxN[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	boxSize = boxV.size();
	boxTex = loadBMP_custom("block_texture.bmp");


	//gas station
	res = loadOBJ("gas_station.obj", gsV, gsUV, gsN);
	glGenVertexArrays(1, &gsVao);
	glBindVertexArray(gsVao);
	GLuint gsVertexVbo;
	glGenBuffers(1, &gsVertexVbo);
	glBindBuffer(GL_ARRAY_BUFFER, gsVertexVbo);
	glBufferData(GL_ARRAY_BUFFER, gsV.size() * sizeof(glm::vec3), &gsV[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	GLuint gsUvVbo;
	glGenBuffers(1, &gsUvVbo);
	glBindBuffer(GL_ARRAY_BUFFER, gsUvVbo);
	glBufferData(GL_ARRAY_BUFFER, gsUV.size() * sizeof(glm::vec2), &gsUV[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	GLuint gsNormalVbo;
	glGenBuffers(1, &gsNormalVbo);
	glBindBuffer(GL_ARRAY_BUFFER, gsNormalVbo);
	glBufferData(GL_ARRAY_BUFFER, gsN.size() * sizeof(glm::vec3), &gsN[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	gsSize = gsV.size();
	gsTex = loadBMP_custom("metal.bmp");
	gsTex2 = loadBMP_custom("gas_station_texture.bmp");

	//trail
	glGenVertexArrays(1, &particleVao);
	glBindVertexArray(particleVao);
	static const GLfloat g_vertex_buffer_data[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
	};
	glGenBuffers(1, &particle_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particle_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	glGenBuffers(1, &particles_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glBufferData(GL_ARRAY_BUFFER, maxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
	glGenBuffers(1, &particles_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, maxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
}

void SortParticles() {
	std::sort(&particlesSet[0], &particlesSet[maxParticles]);
}

void paintGL(void)
{
	glUseProgram(programID);
	curTime = glutGet(GLUT_ELAPSED_TIME);
	double delta = (curTime - lastTime) / (1000.0);
	lastTime = curTime;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//TODO:
	//Set lighting information, such as position and color of lighting source
	//Set transformation matrix
	//Bind different textures

	//initialization
	glClearColor(0.6f, 0.8f, 0.9f, 1.0f);
	GLint modelTransformMatrixUniformLocation, textureID;

	//projection matrix
	GLint projectionMatrixUniformLocation = glGetUniformLocation(programID, "projectionMatrix");
	glGetIntegerv(GL_VIEWPORT, viewport);
	projectionMatrix = glm::perspective(1.0f, viewport[2] * 1.0f / viewport[3], 5.0f, 100.0f);
	glUniformMatrix4fv(projectionMatrixUniformLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

	//view matrix
	if (viewControl == 1) {
		GLint curTime = glutGet(GLUT_ELAPSED_TIME);
		if (curTime - prevTime > 100) {
			centerX = viewport[2] / 2;
			centerY = viewport[3] / 2;
			prevTime = curTime;
			horAng += 0.0005f * GLfloat(centerX - curX);
			verAng += 0.0005f * GLfloat(centerY - curY);
			glm::vec3 direction(cos(verAng)*sin(horAng), sin(verAng), cos(verAng)*cos(horAng));
			glm::vec3 right = glm::vec3(sin(horAng - 3.14f / 2.0f), 0, cos(horAng - 3.14f / 2.0f));
			glm::vec3 up = glm::cross(right, direction);
			viewMatrix = glm::lookAt(glm::vec3(0, 0, 0), direction, up);
			glutWarpPointer(centerX, centerY);
		}
	}
	GLint viewMatrixUniformLocation = glGetUniformLocation(programID, "viewMatrix");
	glUniformMatrix4fv(viewMatrixUniformLocation, 1, GL_FALSE, &viewMatrix[0][0]);

	//ambient light
	GLint ambientLightUniformLocation = glGetUniformLocation(programID, "ambientLight");
	glm::vec4 ambientLight(0.5f, 0.5f, 0.5f, 1.0f);
	glUniform3fv(ambientLightUniformLocation, 1, &ambientLight[0]);

	//specularLight
	GLint lightPositionUniformLocation1 = glGetUniformLocation(programID, "lightPositionWorld1");
	vec3 lightPosition1(+0.01f, 0.01f, +0.01f);
	glUniform3fv(lightPositionUniformLocation1, 1, &lightPosition1[0]);
	GLint specularLightIntensityUniformLocation = glGetUniformLocation(programID, "n");
	glUniform1f(specularLightIntensityUniformLocation, specularLightIntensity);

	//specularLight 2
	GLint lightPositionUniformLocation2 = glGetUniformLocation(programID, "lightPositionWorld2");
	vec3 lightPosition2(+0.0f, 0.0f, -50.0f);
	glUniform3fv(lightPositionUniformLocation2, 1, &lightPosition2[0]);
	GLint lightIntensityUniformLocation = glGetUniformLocation(programID, "lightIntensity");
	glm::vec4 lightIntensity(diffuseLightIntensity, diffuseLightIntensity, diffuseLightIntensity, 0);
	glUniform4fv(lightIntensityUniformLocation, 1, &lightIntensity[0]);

	//eye position
	GLint eyePositionUniformLocation = glGetUniformLocation(programID, "eyePositionWorld");
	vec3 eyePosition(0.0f, 0.0f, 0.0f);
	glUniform3fv(eyePositionUniformLocation, 1, &eyePosition[0]);

	//plane model transformation
	glBindVertexArray(planeVao);
	modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::translate(glm::mat4(), glm::vec3(0, -10.0f, -20.0f));
	modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(10, 10, 10));
	modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	
	//plane texture
	textureID = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE0);
	if (planeChange == 0) {
		glBindTexture(GL_TEXTURE_2D, planeTex);
		glUniform1i(textureID, 0);
		glDrawArrays(GL_TRIANGLES, 0, planeSize);
	}else if (planeChange == 1) {
		glBindTexture(GL_TEXTURE_2D, planeTex2);
		glUniform1i(textureID, 0);
		glDrawArrays(GL_TRIANGLES, 0, planeSize);
	}else if (planeChange == 2) {
		glBindTexture(GL_TEXTURE_2D, planeTex3);
		glUniform1i(textureID, 0);
		glDrawArrays(GL_TRIANGLES, 0, planeSize);
	}else if (planeChange == 3) {
		glBindTexture(GL_TEXTURE_2D, planeTex4);
		glUniform1i(textureID, 0);
		glDrawArrays(GL_TRIANGLES, 0, planeSize);
	}


	//car model transformation
	glBindVertexArray(carVao);
	modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::translate(glm::mat4(), glm::vec3(-8.7f + posX, -3.9f, -17.4f + posZ));
	modelTransformMatrix = glm::rotate(modelTransformMatrix, turnAng, glm::vec3(0, 1, 0));
	modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(1.2f, 1.2f, 1.2f));
	modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);

	//car texture
	textureID = glGetUniformLocation(programID, "myTextureSampler");
	if (carChange == 0) {		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, carTex);
		glUniform1i(textureID, 1);
		glDrawArrays(GL_TRIANGLES, 0, carSize);
	}
	else if (carChange ==1){
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, carTex2);
		glUniform1i(textureID, 1);
		glDrawArrays(GL_TRIANGLES, 0, carSize);
	}

	//box model transformation
	if (rotateZ == 1) zAng = zAng + 0.05f;
	glBindVertexArray(boxVao);
	modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::translate(glm::mat4(), glm::vec3(+5.5f, -2.5f, -15.0f));
	modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(1.5, 1.5, 1.5));
	modelTransformMatrix = glm::rotate(modelTransformMatrix, zAng, vec3(0, 0, 1));
	modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);

	//box texture
	textureID = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, boxTex);
	glUniform1i(textureID, 2);
	glDrawArrays(GL_TRIANGLES, 0, boxSize);

	//gas station model transformation
	glBindVertexArray(gsVao);
	modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::translate(glm::mat4(), glm::vec3(-15.0f, -3.9f, -35.0f));
	modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(0.025f, 0.025f, 0.025f));
	modelTransformMatrix = glm::rotate(modelTransformMatrix, -3.1f, glm::vec3(0, 1, 0));
	modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);

	//gas station texture
	textureID = glGetUniformLocation(programID, "myTextureSampler");
	if (gsChange == 0) {
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, gsTex);
		glUniform1i(textureID, 3);
		glDrawArrays(GL_TRIANGLES, 0, gsSize);
	}
	else if (gsChange ==1 ){
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, gsTex2);
		glUniform1i(textureID, 3);
		glDrawArrays(GL_TRIANGLES, 0, gsSize);
	}

	//trail
	glUseProgram(programID2);
	viewMatrixUniformLocation = glGetUniformLocation(programID2, "viewMatrix");
	glUniformMatrix4fv(viewMatrixUniformLocation, 1, GL_FALSE, &viewMatrix[0][0]);
	projectionMatrixUniformLocation = glGetUniformLocation(programID2, "projectionMatrix");
	glUniformMatrix4fv(projectionMatrixUniformLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
	glBindVertexArray(particleVao);
	modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrixUniformLocation = glGetUniformLocation(programID2, "modelTransformMatrix");
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);

	int newparticles = 1;
	if (newparticles > (int)(0.016f*10000.0)) newparticles = (int)(0.016f*10000.0);

	for (int i = 0; i < newparticles; i++) {
		int particleIndex = findAvailableParticle();
		particlesSet[particleIndex].life = 5.0f;
		particlesSet[particleIndex].pos = glm::vec3(posX - 9.0f, -3.0f, posZ - 17.2f);

		particlesSet[particleIndex].r = 255;
		particlesSet[particleIndex].g = 30;
		particlesSet[particleIndex].b = 30;
		particlesSet[particleIndex].a = 255;

		particlesSet[particleIndex].size = 0.3f;
	}

	int ParticlesCount = 0;
	for (int i = 0; i < maxParticles; i++) {
		Particle& p = particlesSet[i];
		if (p.life > 0.0f) {
			p.life -= delta;
			if (p.life > 0.0f) {
				p.cameradistance = glm::length(p.pos - eyePosition);

				particle_position_size_data[4 * ParticlesCount + 0] = p.pos.x;
				particle_position_size_data[4 * ParticlesCount + 1] = p.pos.y;
				particle_position_size_data[4 * ParticlesCount + 2] = p.pos.z;

				particle_position_size_data[4 * ParticlesCount + 3] = p.size;

				particle_color_data[4 * ParticlesCount + 0] = p.r;
				particle_color_data[4 * ParticlesCount + 1] = p.g;
				particle_color_data[4 * ParticlesCount + 2] = p.b;
				particle_color_data[4 * ParticlesCount + 3] = p.a;
			}
			else p.cameradistance = -1.0f;
			ParticlesCount++;
		}
	}
	SortParticles();

	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glBufferData(GL_ARRAY_BUFFER, maxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, particle_position_size_data);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, maxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte) * 4, particle_color_data);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, particle_vertex_buffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);

	glVertexAttribDivisor(0, 0);
	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);

	glFlush();
	glutPostRedisplay();
}

void initializedGL(void)
{
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	installShaders();
	sendDataToOpenGL();
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	//TODO:
	//Create a window with title specified
	glutCreateWindow("OpenGLPractice");

	for (int i = 0; i < maxParticles; i++) {
		particlesSet[i].life = -1.0f;
		particlesSet[i].cameradistance = -1.0f;
	}
	initializedGL();
	glutDisplayFunc(paintGL);

	glutKeyboardFunc(keyboard);
	glutSpecialFunc(move);
	glutPassiveMotionFunc(PassiveMouse);

	prevTime = glutGet(GLUT_ELAPSED_TIME);

	glutMainLoop();

	return 0;
}
