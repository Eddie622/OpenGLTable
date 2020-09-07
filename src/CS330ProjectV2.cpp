/*
 * CS330Project.cpp
 *
 *  Created on: Jul 30, 2020
 *      Author: Admin
 */

/*Header Inclusions */
#include <Windows.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

//GLM Math Header Inclusions */
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// SOIL Image Loader Inclusion
#include "../SOIL2/SOIL2.h"

using namespace std; // Standard namespace

#define WINDOW_TITLE "CS 330 Final Project"

/* Shader program Macro */
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version "\n" #Source
#endif

/* Variable declarations for shader, window size initialization, buffer and array objects*/
GLint shaderProgram, WindowWidth = 800, WindowHeight = 600;
GLuint VBO, VAO, texture;
GLfloat degrees = glm::radians(-45.0f); // Converts float to degrees

GLfloat cameraSpeed = 0.0005f; //Movement speed per frame
GLchar currentKey; //Will store key pressed
int keymod;  //check for alt

GLfloat scale_by_y=2.0f;
GLfloat scale_by_z=2.0f;
GLfloat scale_by_x=2.0f;

GLfloat lastMouseX = 400, lastMouseY = 300;  //locks mouse cursor at the center of the screen
GLfloat mouseXoffset, mouseYoffset , yaw = 0.0f ,pitch = 0.0f; //mouse offset , yaw and pitch variables
GLfloat sensitivity = 0.01f; //used for mouse and camera sensitivity
bool mouseDetected = true; //initially true when mouse is detected

bool rotate = false;

bool checkMotion = false;

bool checkZoom = false;

//Global vector declarations
glm::vec3 CameraPosition = glm::vec3(0.0f, 0.0f, 0.0f); // Initial camera position. Placed units in Z
glm::vec3 CameraUpY = glm::vec3(0.0f, 1.0f, 0.0f); //Temporary y unit vector
glm::vec3 CameraForwardZ = glm::vec3(0.0f, 0.0f, -1.0f); //Temporary z unit vector
glm::vec3 front;  //temporary z unit vector for mouse

/*Function prototypes */
void UResizeWindow(int, int);
void URenderGraphics(void);
void UCreateShader(void);
void UCreateBuffers(void);
void UGenerateTexture(void);
void UMouseMove(int x , int y);
void OnMouseClicks(int button, int state, int x , int y);
void onMotion(int x, int y);



// Vertex shader source code
const GLchar* vertexShaderSource = GLSL(330,
	layout(location = 0) in vec3 position;
	layout(location = 2) in vec2 textureCoordinates;
	out vec2 mobileTextureCoordinate;

	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;
	void main() {
		gl_Position = projection * view * model * vec4(position, 1.0f);
		mobileTextureCoordinate = vec2(textureCoordinates.x, 1.0f - textureCoordinates.y);
	}
);

// Fragment shader source code
const GLchar* fragmentShaderSource = GLSL(330,
	in vec2 mobileTextureCoordinate;
	out vec4 gpuTexture;
	uniform sampler2D uTexture;
	void main() {
		gpuTexture = texture(uTexture, mobileTextureCoordinate);
	}
);


/* Main Program */
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutCreateWindow(WINDOW_TITLE);

	glutReshapeFunc(UResizeWindow);

	glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK)
		{
			std::cout << "Failed to initialize GLEW" << std::endl;
			return -1;
		}

		UCreateShader();
		UCreateBuffers();
		UGenerateTexture();

		// Use the Shader program
		glUseProgram(shaderProgram);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color

	glutDisplayFunc(URenderGraphics);


	glutPassiveMotionFunc(UMouseMove); // detect mouse move

    glutMotionFunc(onMotion);


	glutMouseFunc(OnMouseClicks);


	glutMainLoop();


	// Destroys Buffer objects once used
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	return 0;
}


/* Resizes the Window*/
void UResizeWindow(int w, int h)
{
	WindowWidth = w;
	WindowHeight = h;
	glViewport(0, 0, WindowWidth, WindowHeight);

}

/* Renders graphics */
void URenderGraphics(void)
{
	glEnable(GL_DEPTH_TEST); // Enable z-depth

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clears the screen

	glBindVertexArray(VAO); // Active the Vertex Array object before rendering and transforming them

	glBindTexture(GL_TEXTURE_2D, texture);

	//* Create Movement Logic */

    CameraForwardZ = front;

	// Transforms the Object
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // Place the object at the center of the viewport
	model = glm::rotate(model, 45.0f, glm::vec3(0.0, 1.0f, 0.0f)); // Rotate the object 45 degrees on the X
	model = glm::scale(model, glm::vec3(scale_by_x,scale_by_y,scale_by_z)); // Increase the object size by a scale of 2

	//Transforms the camera
	glm::mat4 view;
	view = glm::lookAt(CameraForwardZ, CameraPosition, CameraUpY);

	// Creates a perspective projection
	glm::mat4 projection;
	projection = glm::perspective(45.0f, (GLfloat)WindowWidth / (GLfloat)WindowHeight, 0.1f, 100.0f);

	// Retrieves and passes transform matrices to the Shader program
	GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
	GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
	GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


	glutPostRedisplay();

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, 156);
	glBindVertexArray(0); // Deactivates the Vertex Array Object
	glutSwapBuffers(); // Flips the back buffer with the front buffer every frame
}


/* Creates the Shader program */
void UCreateShader()
{
	//Vertex shader
	GLint vertexShader = glCreateShader(GL_VERTEX_SHADER); // Creates the Vertex shader
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); // Attaches the Vertex shader to the source code
	glCompileShader(vertexShader); // Compiles the Vertex shader

	// Fragment shader
	GLint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Creates the Fragment shader
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL); //Attaches the Fragment shader to the source code
	glCompileShader(fragmentShader); // Compiles the Fragment shader

	//Shader program
	shaderProgram = glCreateProgram(); //Create the Shader program and returns an id
	glAttachShader(shaderProgram, vertexShader); //Attach Vertex shader to the Shader program
	glAttachShader(shaderProgram, fragmentShader);; // Attach Fragment shader to the shader program
	glLinkProgram(shaderProgram); // Link vertex and fragment shader to shader program

	//Delete the Vertex and Fragment shaders once linked
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}


void UCreateBuffers()
{

	GLfloat vertices[] = {

					  // Vertex Positions   // Texture Coordinates

					  // base back red
					 -0.5f, -0.3f, -0.8f, 	0.5f, 1.0f,
					  0.5f, -0.3f, -0.8f, 	1.0f, 0.0f,
					  0.5f,  0.0f, -0.8f, 	0.0f, 0.0f,
					  0.5f,  0.0f, -0.8f, 	0.5f, 1.0f,
					 -0.5f,  0.0f, -0.8f, 	1.0f, 0.0f,
					 -0.5f, -0.3f, -0.8f, 	0.0f, 0.0f,

					 // base front green
					 -0.5f, -0.3f,  0.8f, 	0.5f, 1.0f,
					 0.5f, -0.3f,  0.8f,  	1.0f, 0.0f,
					 0.5f,  0.0f,  0.8f, 	0.0f, 0.0f,
					 0.5f,  0.0f,  0.8f, 	0.5f, 1.0f,
					 -0.5f,  0.0f,  0.8f,	1.0f, 0.0f,
					 -0.5f, -0.3f,  0.8f, 	0.0f, 0.0f,

					 // base left blue
					 -0.5f,  0.0f,  0.8f, 	0.5f, 1.0f,
					 -0.5f,  0.0f, -0.8f, 	1.0f, 0.0f,
					 -0.5f, -0.3f, -0.8f,  	0.0f, 0.0f,
					 -0.5f, -0.3f, -0.8f,  	0.5f, 1.0f,
					 -0.5f, -0.3f,  0.8f,  	1.0f, 0.0f,
					 -0.5f,  0.0f,  0.8f, 	0.0f, 0.0f,

					// base right yellow
					 0.5f,  0.0f,  0.8f, 	0.5f, 1.0f,
					 0.5f,  0.0f, -0.8f, 	1.0f, 0.0f,
					 0.5f, -0.3f, -0.8f,  	0.0f, 0.0f,
					 0.5f, -0.3f, -0.8f,  	0.5f, 1.0f,
					 0.5f, -0.3f,  0.8f,  	1.0f, 0.0f,
					 0.5f,  0.0f,  0.8f, 	0.0f, 0.0f,

					// base bottom light blue
					-0.5f, -0.3f, -0.8f, 	0.5f, 1.0f,
					 0.5f, -0.3f, -0.8f, 	1.0f, 0.0f,
					 0.5f, -0.3f,  0.8f, 	0.0f, 0.0f,
					 0.5f, -0.3f,  0.8f, 	0.5f, 1.0f,
					-0.5f, -0.3f,  0.8f, 	1.0f, 0.0f,
					-0.5f, -0.3f, -0.8f, 	0.0f, 0.0f,

					// base top pink
					-0.5f,  0.0f, -0.8f, 	0.5f, 1.0f,
					 0.5f,  0.0f, -0.8f, 	1.0f, 0.0f,
					 0.5f,  0.0f,  0.8f, 	0.0f, 0.0f,
					 0.5f,  0.0f,  0.8f, 	0.5f, 1.0f,
					-0.5f,  0.0f,  0.8f, 	1.0f, 0.0f,
					-0.5f,  0.0f, -0.8f, 	0.0f, 0.0f,

					// leg front left front red
					-0.5f, -0.3f,  0.8f, 	0.5f, 1.0f,
					-0.2f, -0.3f,  0.8f, 	1.0f, 0.0f,
					-0.5f, -1.0f,  0.8f, 	0.0f, 0.0f,
					-0.2f, -0.3f,  0.8f, 	0.5f, 1.0f,
					-0.5f, -1.0f,  0.8f, 	1.0f, 0.0f,
					-0.2f, -1.0f,  0.8f, 	0.0f, 0.0f,

					// leg front right front red
					 0.5f, -0.3f,  0.8f, 	0.5f, 1.0f,
					 0.2f, -0.3f,  0.8f, 	1.0f, 0.0f,
					 0.5f, -1.0f,  0.8f, 	0.0f, 0.0f,
					 0.2f, -0.3f,  0.8f, 	0.5f, 1.0f,
					 0.5f, -1.0f,  0.8f, 	1.0f, 0.0f,
					 0.2f, -1.0f,  0.8f, 	0.0f, 0.0f,

					 // leg front left back green
					 -0.5f, -0.3f,  0.5f, 	0.5f, 1.0f,
					 -0.2f, -0.3f,  0.5f, 	1.0f, 0.0f,
					 -0.5f, -1.0f,  0.5f, 	0.0f, 0.0f,
					 -0.2f, -0.3f,  0.5f, 	0.5f, 1.0f,
					 -0.5f, -1.0f,  0.5f, 	1.0f, 0.0f,
					 -0.2f, -1.0f,  0.5f, 	0.0f, 0.0f,

					 // leg front right back green
					 0.5f, -0.3f,  0.5f, 	0.5f, 1.0f,
					 0.2f, -0.3f,  0.5f, 	1.0f, 0.0f,
					 0.5f, -1.0f,  0.5f, 	0.0f, 0.0f,
					 0.2f, -0.3f,  0.5f, 	0.5f, 1.0f,
					 0.5f, -1.0f,  0.5f, 	1.0f, 0.0f,
					 0.2f, -1.0f,  0.5f, 	0.0f, 0.0f,

					 // leg back left front red
					 -0.5f, -0.3f, -0.5f, 	0.5f, 1.0f,
					 -0.2f, -0.3f, -0.5f, 	1.0f, 0.0f,
					 -0.5f, -1.0f, -0.5f, 	0.0f, 0.0f,
					 -0.2f, -0.3f, -0.5f, 	0.5f, 1.0f,
					 -0.5f, -1.0f, -0.5f, 	1.0f, 0.0f,
					 -0.2f, -1.0f, -0.5f, 	0.0f, 0.0f,

					 // leg back right front red
					 0.5f, -0.3f, -0.5f, 	0.5f, 1.0f,
					 0.2f, -0.3f, -0.5f, 	1.0f, 0.0f,
					 0.5f, -1.0f, -0.5f, 	0.0f, 0.0f,
					 0.2f, -0.3f, -0.5f, 	0.5f, 1.0f,
					 0.5f, -1.0f, -0.5f, 	1.0f, 0.0f,
					 0.2f, -1.0f, -0.5f, 	0.0f, 0.0f,


					  // leg back left back green
					 -0.5f, -0.3f, -0.8f, 	0.5f, 1.0f,
					 -0.2f, -0.3f, -0.8f, 	1.0f, 0.0f,
					 -0.5f, -1.0f, -0.8f, 	0.0f, 0.0f,
					 -0.2f, -0.3f, -0.8f, 	0.5f, 1.0f,
					 -0.5f, -1.0f, -0.8f, 	1.0f, 0.0f,
					 -0.2f, -1.0f, -0.8f, 	0.0f, 0.0f,

					 // leg back right back green
					 0.5f, -0.3f, -0.8f, 	0.5f, 1.0f,
					 0.2f, -0.3f, -0.8f, 	1.0f, 0.0f,
					 0.5f, -1.0f, -0.8f, 	0.0f, 0.0f,
					 0.2f, -0.3f, -0.8f, 	0.5f, 1.0f,
					 0.5f, -1.0f, -0.8f, 	1.0f, 0.0f,
					 0.2f, -1.0f, -0.8f, 	0.0f, 0.0f,

					 // leg front right right blue
					 0.5f,  -1.0f,  0.8f, 	0.5f, 1.0f,
					 0.5f,  -0.3f,  0.8f, 	1.0f, 0.0f,
					 0.5f, -0.3f,  0.5f, 	0.0f, 0.0f,
					 0.5f,  -0.3f,  0.5f, 	0.5f, 1.0f,
					 0.5f,  -1.0f,  0.8f, 	1.0f, 0.0f,
					 0.5f, -1.0f,  0.5f, 	0.0f, 0.0f,

					 // leg front left right blue
					 -0.2f,  -1.0f,  0.8f, 	0.5f, 1.0f,
					 -0.2f,  -0.3f,  0.8f, 	1.0f, 0.0f,
					 -0.2f, -0.3f,  0.5f, 	0.0f, 0.0f,
					 -0.2f,  -0.3f,  0.5f, 	0.5f, 1.0f,
					 -0.2f,  -1.0f,  0.8f, 	1.0f, 0.0f,
					 -0.2f, -1.0f,  0.5f, 	0.0f, 0.0f,

					 // leg back right right blue
					 0.5f,  -1.0f, -0.8f, 	0.5f, 1.0f,
					 0.5f,  -0.3f, -0.8f, 	1.0f, 0.0f,
					 0.5f, -0.3f, -0.5f, 	0.0f, 0.0f,
					 0.5f,  -0.3f, -0.5f, 	0.5f, 1.0f,
					 0.5f,  -1.0f, -0.8f, 	1.0f, 0.0f,
					 0.5f, -1.0f, -0.5f, 	0.0f, 0.0f,

					 // leg back left right blue
					 -0.2f,  -1.0f, -0.8f, 	0.5f, 1.0f,
					 -0.2f,  -0.3f, -0.8f, 	1.0f, 0.0f,
					 -0.2f, -0.3f, -0.5f, 	0.0f, 0.0f,
					 -0.2f,  -0.3f, -0.5f, 	0.5f, 1.0f,
					 -0.2f,  -1.0f, -0.8f, 	1.0f, 0.0f,
					 -0.2f, -1.0f, -0.5f, 	0.0f, 0.0f,

					 // leg front right left yellow
					 0.2f,  -1.0f,  0.8f, 	0.5f, 1.0f,
					 0.2f,  -0.3f,  0.8f, 	1.0f, 0.0f,
					 0.2f, -0.3f,  0.5f, 	0.0f, 0.0f,
					 0.2f,  -0.3f,  0.5f, 	0.5f, 1.0f,
					 0.2f,  -1.0f,  0.8f, 	1.0f, 0.0f,
					 0.2f, -1.0f,  0.5f, 	0.0f, 0.0f,

					 // leg front left left yellow
					 -0.5f,  -1.0f,  0.8f, 	0.5f, 1.0f,
					 -0.5f,  -0.3f,  0.8f, 	1.0f, 0.0f,
					 -0.5f, -0.3f,  0.5f, 	0.0f, 0.0f,
					 -0.5f,  -0.3f,  0.5f, 	0.5f, 1.0f,
					 -0.5f,  -1.0f,  0.8f, 	1.0f, 0.0f,
					 -0.5f, -1.0f,  0.5f, 	0.0f, 0.0f,

					 // leg back right left yellow
					 0.2f,  -1.0f, -0.8f, 	0.5f, 1.0f,
					 0.2f,  -0.3f, -0.8f, 	1.0f, 0.0f,
					 0.2f, -0.3f, -0.5f, 	0.0f, 0.0f,
					 0.2f,  -0.3f, -0.5f, 	0.5f, 1.0f,
					 0.2f,  -1.0f, -0.8f, 	1.0f, 0.0f,
					 0.2f, -1.0f, -0.5f, 	0.0f, 0.0f,

					  // leg back left left yellow
					 -0.5f,  -1.0f, -0.8f, 	0.5f, 1.0f,
					 -0.5f,  -0.3f, -0.8f, 	1.0f, 0.0f,
					 -0.5f, -0.3f, -0.5f, 	0.0f, 0.0f,
					 -0.5f,  -0.3f, -0.5f, 	0.5f, 1.0f,
					 -0.5f,  -1.0f, -0.8f, 	1.0f, 0.0f,
					 -0.5f, -1.0f, -0.5f, 	0.0f, 0.0f,

					  // leg front right bottom pink
					  0.2f, -1.0f,  0.8f, 	0.5f, 1.0f,
					  0.5f, -1.0f,  0.8f, 	1.0f, 0.0f,
					  0.5f, -1.0f,  0.5f, 	0.0f, 0.0f,
					  0.2f, -1.0f,  0.8f, 	0.5f, 1.0f,
					  0.2f, -1.0f,  0.5f, 	1.0f, 0.0f,
					  0.5f, -1.0f,  0.5f, 	0.0f, 0.0f,

					   // leg front left bottom pink
					  -0.2f, -1.0f,  0.8f, 	0.5f, 1.0f,
					  -0.5f, -1.0f,  0.8f, 	1.0f, 0.0f,
					  -0.5f, -1.0f,  0.5f, 	0.0f, 0.0f,
					  -0.2f, -1.0f,  0.8f, 	0.5f, 1.0f,
					  -0.2f, -1.0f,  0.5f, 	1.0f, 0.0f,
					  -0.5f, -1.0f,  0.5f, 	0.0f, 0.0f,

					  // leg back right bottom pink
					  0.2f, -1.0f, -0.8f, 	0.5f, 1.0f,
					  0.5f, -1.0f, -0.8f, 	1.0f, 0.0f,
					  0.5f, -1.0f, -0.5f, 	0.0f, 0.0f,
					  0.2f, -1.0f, -0.8f, 	0.5f, 1.0f,
					  0.2f, -1.0f, -0.5f, 	1.0f, 0.0f,
					  0.5f, -1.0f, -0.5f, 	0.0f, 0.0f,

					  // leg back left bottom pink
					  -0.2f, -1.0f, -0.8f, 	0.5f, 1.0f,
					  -0.5f, -1.0f, -0.8f,	 1.0f, 0.0f,
					  -0.5f, -1.0f, -0.5f, 	0.0f, 0.0f,
					  -0.2f, -1.0f, -0.8f,	 0.5f, 1.0f,
					  -0.2f, -1.0f, -0.5f, 	1.0f, 0.0f,
					  -0.5f, -1.0f, -0.5f, 	0.0f, 0.0f,

};


	//Generate buffer ids
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	// Active the vertex Array object before binding and setting any VBOs and vertex Attribute Pointers
	glBindVertexArray(VAO);

	// Active the VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //Copy vertices to VBO


	//Set attribute pointer 0 to hold Position data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0); // Enable vertex attribute


	//Set attribute pointer 1 to hold Color data
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2); // Enable vertex attribute


	glBindVertexArray(0); // Deactivates the VAO which is good practice


}

void UGenerateTexture(){

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	int width, height;

	unsigned char* image = SOIL_load_image("wood.png", &width, &height, 0, SOIL_LOAD_RGB); // Loads texture file

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
}


void UMouseMove(int x, int y){

        front.x = 10.0f * cos(yaw);
        front.y = 10.0f * sin(pitch);
        front.z = sin(yaw) * cos(pitch) * 10.0f;

}

void onMotion(int curr_x, int curr_y) {

	//if left alt and mouse down are set
	if(checkMotion){

	//gets the direction the mouse was moved
	   mouseXoffset = curr_x - lastMouseX;
	   mouseYoffset = lastMouseY - curr_y;

		  //updates with new mouse coorditaes
		   lastMouseX = curr_x;
		   lastMouseY = curr_y;

		   //Applies sensitivity to mouse direction
		   mouseXoffset *= sensitivity;
		   mouseYoffset *= sensitivity;


           //get the direction of the mouse
		   //if there is changes in yaw, then it is moving along X
		   if(yaw != yaw+mouseXoffset && pitch == pitch+mouseYoffset){

			   //INCREAMENT yaw
			   yaw += mouseXoffset;



			   //else movement in y
		   }else if(pitch != pitch+mouseYoffset && yaw == yaw+mouseXoffset ){


			   //increament y to move vertical
			    pitch += mouseYoffset;

		   }


		   front.x = 10.0f * cos(yaw);
		   front.y = 10.0f * sin(pitch);
		   front.z = sin(yaw) * cos(pitch) * 10.0f;


	}

	//check if user is zooming, alt, right mouse button and down

	if(checkZoom){


		//determine the direction of the , whether up or down
                    if(lastMouseY > curr_y){

                    	//increament scale values
                    	       scale_by_y += 0.1f;
                    		   scale_by_x += 0.1f;
                    		   scale_by_z += 0.1f;

                    		   //redisplay
                    		   glutPostRedisplay();


                    }else{

                    	//decreament scale values, zoom in
                       scale_by_y -= 0.1f;
					   scale_by_x -= 0.1f;
					   scale_by_z -= 0.1f;

					   //control zoom in size
					   if(scale_by_y < 0.2f){

						   scale_by_y = 0.2f;
						   scale_by_x = 0.2f;
						   scale_by_z = 0.2f;
					   }

					   glutPostRedisplay();
                    }

                    //update x and y
                    lastMouseY = curr_y;
                    lastMouseX = curr_x;
	}
}

void OnMouseClicks(int button, int state, int x, int y) {


   keymod = glutGetModifiers(); // checks for modifier keys like alt, shif and ctrl

   checkMotion = false; //set checkMotion to false

   //check if button is left, and mod is alt and state is down, all should be true
   if(button == GLUT_LEFT_BUTTON && keymod == GLUT_ACTIVE_ALT && state == GLUT_DOWN) {

	   //if true then set motion true
      checkMotion = true;

      //zooming to be false
      checkZoom = false;


   }else if(button == GLUT_RIGHT_BUTTON && keymod == GLUT_ACTIVE_ALT && state == GLUT_DOWN){

	   //zoom to be true and motion to be false
	   checkMotion = false;
	   checkZoom = true;
   }
}



