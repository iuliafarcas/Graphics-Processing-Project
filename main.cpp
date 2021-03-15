//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//


//////////////////PROIECT BUN
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"
#include "Window.h"

#include <iostream>

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;


gps::Window myWindow;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;


glm::vec3 Lposition;
GLuint LpositionLoc;
glm::vec3 lightColor2;
GLuint lightColorLoc2;

glm::vec3 Lposition2;
GLuint LpositionLoc2;
glm::vec3 lightColor3;
GLuint lightColorLoc3;

int yellow_light;
GLuint nightLoc;

gps::Camera myCamera(
				glm::vec3(0.0f, 10.0f, 70.3f), 
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
GLfloat cameraSpeed = 3.0f;

GLboolean pressedKeys[1024];
float angleY = 0.0f;
GLfloat lightAngle;


gps::Model3D ground;
gps::Model3D screenQuad;
gps::Model3D fish1;
gps::Model3D fish2;
gps::Model3D fish3;
gps::Model3D fish4;
gps::Model3D fish5;
gps::Model3D fish6;
gps::Model3D alga;
gps::Model3D alga2;
gps::Model3D piatra2;
gps::Model3D turtle;
gps::Model3D seahorse;
gps::Model3D submarine;
gps::Model3D helmet;

gps::Shader myCustomShader;
gps::Shader screenQuadShader;
gps::Shader myShader;


GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;

std::vector<const GLchar*> faces;
gps::SkyBox mySkyBox;
std::vector<const GLchar*> faces2;
gps::SkyBox mySkyBox2;
gps::Shader skyboxShader;

GLenum glCheckError_(const char *file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
	WindowDimensions dim;
	dim.height = 715;
	dim.width = 1366;
	myWindow.setWindowDimensions(dim);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key == GLFW_KEY_P && action == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	if (key == GLFW_KEY_O && action == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (key == GLFW_KEY_I && action == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

	if (key == GLFW_KEY_N && action == GLFW_PRESS)
	{
		if (yellow_light == 0)
			yellow_light = 1;
		else yellow_light = 0;
	}


	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}



float lastX = 400, lastY = 300;

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {

	static bool firstMouse = true;
	static double lastX = 0.0f, lastY = 0.0f;
	static float yaw, pitch;

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	myCamera.rotate(pitch, yaw);
	view = myCamera.getViewMatrix();
	myCustomShader.useShaderProgram();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

}

bool move_submarine = false;

float delta = -80.0f;
float delta_angle;

void processMovement() {
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_Q]) {
		angleY -= 2.0f;
		// update model matrix for teapot
		model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0, 1, 0));
		// update normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angleY += 2.0f;

		// update model matrix for teapot
		model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0, 1, 0));
		// update normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
	}
}

void initOpenGLWindow()
{
	myWindow.Create(1024, 768, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
	glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
	glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
	glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}


void initOpenGLState()
{
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	
	ground.LoadModel("objects/ground/ground.obj");
	screenQuad.LoadModel("objects/quad/quad.obj");
	fish1.LoadModel("objects/fishes/TropicalFish01.obj");
	fish2.LoadModel("objects/fishes/TropicalFish02.obj");
	fish3.LoadModel("objects/fishes/TropicalFish05.obj");
	fish4.LoadModel("objects/fishes/TropicalFish10.obj");
	fish5.LoadModel("objects/fishes/TropicalFish12.obj");
	fish6.LoadModel("objects/fishes/TropicalFish15.obj");

	alga.LoadModel("objects/alge/seaweedList.obj");
	alga2.LoadModel("objects/alge2/seaweedList.obj");
	piatra2.LoadModel("objects/piatra2/004.obj");

	turtle.LoadModel("objects/turtle/10042_Sea_Turtle_V2_iterations-2.obj");
	seahorse.LoadModel("objects/seahorse/10044_SeaHorse_v1_iterations-2.obj");
	submarine.LoadModel("objects/submarine/submarine.obj");
	helmet.LoadModel("objects/helmet/helmet.obj");

	faces.push_back("textures/whirlpool1/whirlpool_rt.tga");
	faces.push_back("textures/whirlpool1/whirlpool_lf.tga");
	faces.push_back("textures/whirlpool1/whirlpool_up.tga");
	faces.push_back("textures/whirlpool1/whirlpool_dn.tga");
	faces.push_back("textures/whirlpool1/whirlpool_bk.tga");
	faces.push_back("textures/whirlpool1/whirlpool_ft.tga");
	mySkyBox.Load(faces);

	faces2.push_back("textures/whirlpool2/whirlpool_rt.tga");
	faces2.push_back("textures/whirlpool2/whirlpool_lf.tga");
	faces2.push_back("textures/whirlpool2/whirlpool_up.tga");
	faces2.push_back("textures/whirlpool2/whirlpool_dn.tga");
	faces2.push_back("textures/whirlpool2/whirlpool_bk.tga");
	faces2.push_back("textures/whirlpool2/whirlpool_ft.tga");
	mySkyBox2.Load(faces2);
}

void initShaders() {
	myCustomShader.loadShader("shaders/basic.vert", "shaders/basic.frag");
	myCustomShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	myShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag");
	myShader.useShaderProgram();
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	projection = glm::perspective(glm::radians(45.0f), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 50.0f, 30.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");	
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(0.3f, 0.75f, 0.75f); //turqoise light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
	
	//second light
	Lposition = glm::vec3(0.0f, 10.0f, 0.0f);
	LpositionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "Lposition");
	glUniform3fv(LpositionLoc, 1, glm::value_ptr(Lposition));

	//set light color
	lightColor2 = glm::vec3(0.0f, 0.7f, 0.5f); //greenish
	lightColorLoc2 = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor2");
	glUniform3fv(lightColorLoc2, 1, glm::value_ptr(lightColor2));

	//third light
	Lposition2 = glm::vec3(0.0f, -0.3f, -35.0f);
	LpositionLoc2 = glGetUniformLocation(myCustomShader.shaderProgram, "Lposition2");
	glUniform3fv(LpositionLoc2, 1, glm::value_ptr(Lposition2));

	//set light color
	lightColor3 = glm::vec3(1.0f, 1.0f, 0.0f); //yellow
	lightColorLoc3 = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor3");
	glUniform3fv(lightColorLoc3, 1, glm::value_ptr(lightColor3));


	//lightShader.useShaderProgram();
	//glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	glGenFramebuffers(1, &shadowMapFBO);

	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);


	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,0);
	
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

glm::mat4 computeLightSpaceTrMatrix() {
	//TODO - Return the light-space transformation matrix
	glm::mat4 lightView = glm::lookAt(glm::mat3(lightRotation) * lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = 0.1f, far_plane = 100.0f;
	glm::mat4 lightProjection = glm::ortho(-300.0f, 300.0f, -300.0f, 300.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
	return lightSpaceTrMatrix;
}

float updateDelta(double elapsedSeconds, float delta, float speed) {
	return delta = delta + speed * elapsedSeconds;
}
float updateDeltaNegative(double elapsedSeconds, float delta, float speed) {
	return delta = delta - speed * elapsedSeconds;
}

float deltaArray[3] = {-80.0f, -75.0f, 3.0f};
double lastTimeStampArray[3] = { glfwGetTime(), glfwGetTime(), glfwGetTime()};
double currentTimeStampArray[3];

glm::mat4 fishOne[10];
bool ok1 = true;

void drawfish1(gps::Shader shader, bool depthPass)
{
	shader.useShaderProgram();

	for (int i = 0; i < 10; i++)
	{
		if (ok1)
		{
			model = glm::mat4(1.0f);

			float posx = (float)(rand() % 50);
			float posy = (float)(rand() % 5 + 5);
			float posz = (float)(rand() % 50);
			
			if(i % 2 == 0)
				model = glm::translate(model, glm::vec3(posx, posy, posz));
			else
				model = glm::translate(model, glm::vec3(-posx, posy, -posz));

			model = glm::rotate(model, glm::radians((float)(i * 72)), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
			fishOne[i] = model;
		}
		
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(fishOne[i]));

		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * fishOne[i]));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		fish1.Draw(shader);
	}

	ok1 = false;
}

glm::mat4 fishTwo[10];
bool ok2 = true;

void drawfish2(gps::Shader shader, bool depthPass)
{
	shader.useShaderProgram();

	for (int i = 0; i < 10; i++)
	{
		if (ok2)
		{
			model = glm::mat4(1.0f);

			float posx = (float)(rand() % 50);
			float posy = (float)(rand() % 7 + 3);
			float posz = (float)(rand() % 50);

			if (i % 2 == 0)
				model = glm::translate(model, glm::vec3(-posx, posy, -posz));
			else
				model = glm::translate(model, glm::vec3(posx, posy, posz));

			model = glm::rotate(model, glm::radians((float)(i * 50)), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
			fishTwo[i] = model;
		}

		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(fishTwo[i]));

		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * fishTwo[i]));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		fish2.Draw(shader);
	}

	ok2 = false;
}

glm::mat4 fishThree[5];
bool ok3 = true;

void drawfish3(gps::Shader shader, bool depthPass)
{
	shader.useShaderProgram();

	for (int i = 0; i < 5; i++)
	{
		if (ok3)
		{
			model = glm::mat4(1.0f);

			float posx = (float)(rand() % 50);
			float posy = (float)(rand() % 15);
			float posz = (float)(rand() % 50);

			//if (i % 2 == 0)
				model = glm::translate(model, glm::vec3(-posx, posy, -posz));
			//else
				//model = glm::translate(model, glm::vec3(posx, posy, posz));

			model = glm::rotate(model, glm::radians((float)(i * 80)), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
			fishThree[i] = model;
		}

		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(fishThree[i]));

		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * fishThree[i]));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		fish3.Draw(shader);
	}

	ok3 = false;
}

glm::mat4 fishFour[10];
bool ok4 = true;

void drawfish4(gps::Shader shader, bool depthPass)
{
	shader.useShaderProgram();

	for (int i = 0; i < 10; i++)
	{
		if (ok4)
		{
			model = glm::mat4(1.0f);

			float posx = (float)(rand() % 50);
			float posy = (float)(rand() % 15);
			float posz = (float)(rand() % 50);

			if (i % 2 == 0)
				model = glm::translate(model, glm::vec3(posx, posy, -posz));
			else
				model = glm::translate(model, glm::vec3(-posx, posy, posz));

			model = glm::rotate(model, glm::radians((float)(i * 80)), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
			fishFour[i] = model;
		}

		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(fishFour[i]));

		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * fishFour[i]));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		fish4.Draw(shader);
	}

	ok4 = false;
}

glm::mat4 fishFive[10];
bool ok5 = true;

void drawfish5(gps::Shader shader, bool depthPass)
{
	shader.useShaderProgram();

	for (int i = 0; i < 10; i++)
	{
		if (ok5)
		{
			model = glm::mat4(1.0f);

			float posx = (float)(rand() % 50);
			float posy = (float)(rand() % 12);
			float posz = (float)(rand() % 50);

			if (i % 2 == 0)
				model = glm::translate(model, glm::vec3(-posx, posy, posz));
			else
				model = glm::translate(model, glm::vec3(posx, posy, -posz));

			model = glm::rotate(model, glm::radians((float)(i * 80)), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
			fishFive[i] = model;
		}

		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(fishFive[i]));

		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * fishFive[i]));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		fish5.Draw(shader);
	}

	ok5 = false;
}

glm::mat4 fishSix[10];
bool ok6 = true;

void drawfish6(gps::Shader shader, bool depthPass)
{
	shader.useShaderProgram();

	for (int i = 0; i < 10; i++)
	{
		if (ok6)
		{
			model = glm::mat4(1.0f);

			float posx = (float)(rand() % 50);
			float posy = (float)(rand() % 10);
			float posz = (float)(rand() % 50);

			if (i % 2 == 0)
				model = glm::translate(model, glm::vec3(posx, posy, posz));
			else
				model = glm::translate(model, glm::vec3(-posx, posy, -posz));

			model = glm::rotate(model, glm::radians((float)(i * 80)), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
			fishSix[i] = model;
		}

		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(fishSix[i]));

		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * fishSix[i]));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		fish6.Draw(shader);
	}

	ok6 = false;
}

glm::mat4 alge[20];
bool okAlga = true;

void drawAlga(gps::Shader shader, bool depthPass)
{
	shader.useShaderProgram();

	for (int i = 0; i < 20; i++)
	{
		if (okAlga)
		{
			model = glm::mat4(1.0f);

			float posx = (float)(rand() % 40);
			float posz = (float)(rand() % 40);

			if (i % 2 == 0)
				model = glm::translate(model, glm::vec3(posx, -1.0f, posz));
			else
				model = glm::translate(model, glm::vec3(-posx, -1.0f, -posz));

			model = glm::rotate(model, glm::radians((float)(i * 72)), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(2.0f, 5.0f, 2.0f));
			alge[i] = model;
		}

		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(alge[i]));

		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * fishSix[i]));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		alga.Draw(shader);
	}

	okAlga = false;
}
void drawAlga1(gps::Shader shader, bool depthPass)
{
	for (int i = -150; i < -10; i++)
	{
		model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(float(i * 10)), glm::vec3(0.0f, 1.0f, 0.0f));
		if (i % 2 == 0)
		{
			model = glm::translate(model, glm::vec3((float)(-i * 0.3), -1.0f, (float)(i * 0.3)));
		}
		else model = glm::translate(model, glm::vec3((float)(i * 0.3), -1.0f, (float)(-i * 0.3)));


		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}
		alga.Draw(shader);
	}
}
void drawAlga2(gps::Shader shader, bool depthPass)
{
	for (int i = -100; i < -10; i++)
	{
		model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(float(i * 10)), glm::vec3(0.0f, 1.0f, 0.0f));
		if (i % 2 == 0)
		{
			model = glm::translate(model, glm::vec3((float)(i * 0.45), -1.0f, (float)(-i * 0.45)));
		}
		else model = glm::translate(model, glm::vec3((float)(-i * 0.45), -1.0f, (float)(i * 0.45)));

		model = glm::scale(model, glm::vec3(1.5f, 3.0f, 1.5f));

		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}
		alga2.Draw(shader);
	}
}
void drawGround(gps::Shader shader, bool depthPass)
{
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(7.5f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	ground.Draw(shader);
}
void drawPiatra2(gps::Shader shader, bool depthPass)
{
	for (int i = -2; i < 3; i++)
	{
		model = glm::mat4(1.0f);
		if (i % 2 == 0)
			model = glm::translate(model, glm::vec3((float)(i * 25), -1.0f, (float)(-i * 25)));
		else model = glm::translate(model, glm::vec3((float)(-i * 25), -1.0f, (float)(-i * 25)));

		model = glm::rotate(model, glm::radians((float)(i * 25)), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}
		piatra2.Draw(shader);
	}
}
void drawHelmet(gps::Shader shader, bool depthPass)
{
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -0.3f, -35.0f));
	model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.1f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	helmet.Draw(shader);
}

void drawSubmarine(gps::Shader shader, bool depthPass)
{
	model = glm::mat4(1.0f);

	if (deltaArray[0] > 80)
		deltaArray[0] = -80.0f;

	// get current time
	currentTimeStampArray[0] = glfwGetTime();
	deltaArray[0] = updateDelta(currentTimeStampArray[0] - lastTimeStampArray[0], deltaArray[0], 1.5);
	lastTimeStampArray[0] = currentTimeStampArray[0];

	model = glm::translate(model, glm::vec3(deltaArray[0], 20.0f, -100.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	submarine.Draw(shader);
}

void drawTurtle(gps::Shader shader, bool depthPass)
{

	if (deltaArray[1] > 80)
		deltaArray[1] = -75.0f;

	currentTimeStampArray[1] = glfwGetTime();
	deltaArray[1] = updateDelta(currentTimeStampArray[1] - lastTimeStampArray[1], deltaArray[1], 3.5);
	lastTimeStampArray[1] = currentTimeStampArray[1];

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(deltaArray[1], 15.0f, deltaArray[1]));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	turtle.Draw(shader);
}

bool up = true;
void drawSeaHorse(gps::Shader shader, bool depthPass)
{
		
	model = glm::mat4(1.0f);

	currentTimeStampArray[2] = glfwGetTime();
	if (yellow_light == 1)
	{
		if (!up)
		{
			deltaArray[2] = updateDeltaNegative(currentTimeStampArray[2] - lastTimeStampArray[2], deltaArray[2], 2.0f);
			if (deltaArray[2] <= 3)
			{
				up = true;
				deltaArray[2] = 3.0f;
			}
		}	
		else
		{
			deltaArray[2] = updateDelta(currentTimeStampArray[2] - lastTimeStampArray[2], deltaArray[2], 2.0f);
			if (deltaArray[2] >= 15)
			{
				up = false;
				deltaArray[2] = 15.0f;
			}
		}
		
		lastTimeStampArray[2] = currentTimeStampArray[2];

		model = glm::translate(model, glm::vec3(0.0f, deltaArray[2], -38.0f));
	}
	else model = glm::translate(model, glm::vec3(0.0f, 3.0f, -38.0f));
		

	
	
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	seahorse.Draw(shader);
}


void drawObjects(gps::Shader shader, bool depthPass) {
		
	shader.useShaderProgram();
	
	{
		drawfish1(shader, depthPass);
		drawfish2(shader, depthPass);
		drawfish3(shader, depthPass);
		drawfish4(shader, depthPass);
		drawfish5(shader, depthPass);
		drawfish6(shader, depthPass);
	}

	drawAlga(shader, depthPass);
	drawAlga1(shader, depthPass);
	drawAlga2(shader, depthPass);
	drawPiatra2(shader, depthPass);

	
	drawTurtle(shader, depthPass);
	drawGround(shader, depthPass);
	drawSubmarine(shader, depthPass);

	drawHelmet(shader, depthPass);
	drawSeaHorse(shader, depthPass);
	
}


void renderScene() {

	//Send the light-space transformation matrix to the depth map creation shader and render the scene in the depth map

	myShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(myShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	drawObjects(myShader, true);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// render depth map on screen - toggled with the M key

	if (showDepthMap) {
		glViewport(0, 0, myWindow.getWindowDimensions().width ,myWindow.getWindowDimensions().height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {

		// final scene rendering pass (with shadows)

		glViewport(0, 0, myWindow.getWindowDimensions().width ,myWindow.getWindowDimensions().height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		nightLoc = glGetUniformLocation(myCustomShader.shaderProgram, "yellow_light");
		glUniform1i(nightLoc, yellow_light);


		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
				
		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader, false);


		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
			glm::value_ptr(view));

		projection = glm::perspective(glm::radians(45.0f), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 1000.0f);
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
			glm::value_ptr(projection));
		if (yellow_light == 0)
			mySkyBox2.Draw(skyboxShader, view, projection);
		else mySkyBox.Draw(skyboxShader, view, projection);
	}
}

void cleanup() {

	myWindow.Delete();
}

int main(int argc, const char * argv[]) {

	try {
		initOpenGLWindow();
	}
	catch (const std::exception & e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();

	setWindowCallbacks();


	glCheckError();

	while (!glfwWindowShouldClose(myWindow.getWindow())) {
		processMovement();
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());


		glCheckError();
	}

	cleanup();

	return EXIT_SUCCESS;
}
