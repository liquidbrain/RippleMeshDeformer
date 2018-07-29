/**
 * Deforms a planar mesh using a vertex shader.
 *
 * A Vertex Shader in OpenGL is a piece of C like code written to the GLSL specification which
 * influences the attributes of a vertex. Vertex shaders can be used to modify properties of the
 * vertex such as position, color, and texture coordinates.
 *
 * A Fragment Shader is similar to a Vertex Shader, but is used for calculating individual fragment
 * colors. This is where lighting and bump-mapping effects are performed.
 *
 * Geometry Shaders create additional vertices.
 *
 * The shaders are compilied and then chained together into a Shader Program.
 *
 * The shader pipeline behaves as follows:
 * Vertex Shaders -> Geometry Shaders -> (Rasterizing Engine) -> Fragment Shaders.
 *
 * A Vertex Buffer Object (VBO) holds information about vertices. VBOs can also store information
 * such as color, normals, texcoords, etc.
 *
 * A Vertex Array Object (VAO) is an object which contains one or more Vertex Buffer Objects and is
 * designed to store the information for a complete rendered object. The shaders receive input data
 * from our VAO through a process of attribute binding, allowing us to perform the needed computations
 * to provide us with the desired results.
 */

#include <iostream>

// GLEW: OpenGL Extension Wrangler
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW: A simple API for creating windows, contexts and surfaces, and receiving input and events
#include <GLFW/glfw3.h>

// GLM: OpenGL Math
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLSLProgram.h"

// Function prototypes
GLFWwindow* InitGlfw();
void InitGlShaders();
void InitMesh();
void GlfwErrorCallback(int error, const char* description);
void GlfwFramebufferResizeCallback(GLFWwindow *window, int width, int height);
void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void GlfwWindowRefreshCallback(GLFWwindow *window);
void Render(GLFWwindow* window);

// Window dimensions when the application is started
const GLuint WIDTH = 1280;
const GLuint HEIGHT = 960;

static const char* VERTEX_SHADER_PATH =
        "/Users/john/Dev/OpenGL/RippleMeshDeformer/RippleMeshDeformer/Vertex.shader";
static const char* FRAGMENT_SHADER_PATH =
        "/Users/john/Dev/OpenGL/RippleMeshDeformer/RippleMeshDeformer/Fragment.shader";

// The GLSL program
GLSLProgram glslProgram;

// The vertex array and vertex buffer object IDs
GLuint vaoId;
GLuint vboVerticesId;
GLuint vboIndicesId;

// Size of plane in world space
const float SIZE_X = 4;
const float SIZE_Z = 4;
const float HALF_SIZE_X = SIZE_X / 2.0f;
const float HALF_SIZE_Z = SIZE_Z / 2.0f;

const int NBR_OF_QUADS_X = 40;
const int NBR_OF_QUADS_Z = 40;

// Ripple mesh vertices and indices
glm::vec3 vertices[(NBR_OF_QUADS_X + 1) * (NBR_OF_QUADS_Z + 1)];
const int TOTAL_INDICES = NBR_OF_QUADS_X * NBR_OF_QUADS_Z * 2 * 3;
GLushort indices[TOTAL_INDICES];

const GLfloat RIPPLE_DISPLACEMENT_SPEED = 2.0;

// Transformation variables
glm::mat4 projectionMatrix;
float rX = 500.0, rY = -75.0, distance = -5.0;

int main(int argc, const char* argv[])
{
    GLFWwindow* window = InitGlfw();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    projectionMatrix = glm::perspective(45.0f, static_cast<GLfloat>(width / height), 1.0f, 1000.0f);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    InitMesh();

    InitGlShaders();
    glslProgram.UseProgram();

    std::string s = glslProgram.ToString();
    std::cout << s << std::endl;

    while (!glfwWindowShouldClose(window))
    {
        // Check if any events have been activated (key pressed, mouse moved etc.) and call corresponding
        // response functions.
        glfwPollEvents();

        Render(window);
    }

    // Deallocate all resources once they've outlived their purpose.
    glUseProgram(0);
    glslProgram.DeleteProgram();
    glDeleteVertexArrays(1, &vaoId);
    glDeleteBuffers(1, &vboVerticesId);
    glDeleteBuffers(1, &vboIndicesId);

    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();

    return EXIT_SUCCESS;
}

void InitMesh()
{
    unsigned int index = 0;
    int i=0, j=0;

    // Create the plane vertices.
    for (j = 0; j <= NBR_OF_QUADS_Z; ++j) {
        for (i = 0; i <= NBR_OF_QUADS_X; ++i) {
            vertices[index++] = glm::vec3(
                ((static_cast<float>(i) / (NBR_OF_QUADS_X - 1)) * 2 - 1) * HALF_SIZE_X,
                0,
                ((static_cast<float>(j) / (NBR_OF_QUADS_Z - 1)) * 2 - 1) * HALF_SIZE_Z);
        }
    }

    // Fill the plane indices array.
    GLushort* id = &indices[0];
    for (i = 0; i < NBR_OF_QUADS_Z; ++i) {
        for (j = 0; j < NBR_OF_QUADS_X; ++j) {
            int i0 = i * (NBR_OF_QUADS_X + 1) + j;
            int i1 = i0 + 1;
            int i2 = i0 + (NBR_OF_QUADS_X + 1);
            int i3 = i2 + 1;

            if ((j + i) % 2) {
                *id++ = i0;
                *id++ = i2;
                *id++ = i1;
                *id++ = i1;
                *id++ = i2;
                *id++ = i3;
            }
            else {
                *id++ = i0;
                *id++ = i2;
                *id++ = i3;
                *id++ = i0;
                *id++ = i3;
                *id++ = i1;
            }
        }
    }
}

void Render(GLFWwindow* window)
{
    // Calculate the MVP matrix: model maps from an object's local coordinate space into world space,
    // view from world space to camera space, projection from camera to screen.
    glm::mat4 T	 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, distance));
    glm::mat4 Rx = glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 MV = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 modelViewProjectMatrix = projectionMatrix * MV;

    glBindVertexArray(vaoId);

    glUniformMatrix4fv(
            glslProgram.GetUniformLocation("modelViewProjectMatrix"),
            1,
            GL_FALSE,
            glm::value_ptr(modelViewProjectMatrix));

    GLfloat elapsedTime = glfwGetTime();

    //GLfloat green = (sin(elapsedTime) / 2) + 0.5; // 0 - 1.0
    GLfloat green = 1.0f;
    glUniform4f(glslProgram.GetUniformLocation("newColor"), 0.0f, green, 0.0f, 1.0f);

    GLfloat rippleDisplacement = (sin(elapsedTime)) * RIPPLE_DISPLACEMENT_SPEED;
    glUniform1f(glslProgram.GetUniformLocation("waveTime"), rippleDisplacement);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw the mesh triangles.
    // - first argument specifies what kind of primitive to render
    // - second argument specifies the number of elements to render
    // - third argument specifies the type of values in the indices
    // - forth argument specifies a pointer to the location where the indices are stored
    glDrawElements(GL_TRIANGLES, TOTAL_INDICES, GL_UNSIGNED_SHORT, static_cast<GLvoid*>(0));

    glfwSwapBuffers(window);

    glBindVertexArray(0);
}

void InitGlShaders()
{
    // Load shaders and create the GLSL program.
    glslProgram.AddShaderFromFile(GL_VERTEX_SHADER, VERTEX_SHADER_PATH);
    glslProgram.AddShaderFromFile(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_PATH);
    glslProgram.CreateAndLinkProgram();

    // Add shader attribute and uniforms.
    glslProgram.AddAttribute("vertex");
    glslProgram.AddUniform("waveTime");
    glslProgram.AddUniform("modelViewProjectMatrix");
    glslProgram.AddUniform("newColor");

    // Create buffers.
    glGenVertexArrays(1, &vaoId);
    glGenBuffers(1, &vboVerticesId);
    glGenBuffers(1, &vboIndicesId);

    // Bind the Vertex Array Object.
    glBindVertexArray(vaoId);

    // Bind the Vertex Buffer Object used for the mesh's position.
    glBindBuffer(GL_ARRAY_BUFFER, vboVerticesId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

    // Specify how the vertex buffer data should be interpreted whenever a drawing call is made.
    GLuint vVertexLocation = glslProgram.GetAttributeLocation("vertex");
    glVertexAttribPointer(
            vVertexLocation, // vertex attribute to configure
            3,               // size of the vertex attribute; the vertex attribute is a vec3 so it is composed of 3 values
            GL_FLOAT,        // data is GL_FLOAT (a vec* in GLSL consists of floating point values)
            GL_FALSE,        // normalize data
            0,               // no (zero) space between consecutive vertex attribute sets
            (GLvoid*)0);     // offset of where position data begins in the buffer
    glEnableVertexAttribArray(vVertexLocation);

    // Bind the Vertex Buffer Object used for plane indices.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);
}

/**
 * Creates and initializes a GLFW window and sets callback functions.
 */
GLFWwindow* InitGlfw()
{
    // Init GLFW.
    int glfwInitStatus = glfwInit();
    if (glfwInitStatus != GLFW_TRUE) {
        std::cerr << "GLFW initialization error" << std::endl;
    }

    // Set all the required options for GLFW.
#if 0
    // For Windows...
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
#endif
    // ... For macOS.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a GLFWwindow object that we can use for GLFW's functions.
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ripple Mesh Deformer", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Select the minimum number of monitor refreshes the driver wait should from the time glfwSwapBuffers()
    // was called before swapping the buffers.
    glfwSwapInterval(1);

    // Set the required callback functions.
    glfwSetFramebufferSizeCallback(window, GlfwFramebufferResizeCallback);
    glfwSetWindowRefreshCallback(window, GlfwWindowRefreshCallback);
    glfwSetKeyCallback(window, GlfwKeyCallback);
    glfwSetErrorCallback(GlfwErrorCallback);

    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions.
    glewExperimental = GL_TRUE;

    // Initialize GLEW to setup the OpenGL function pointers.
    GLenum err = glewInit();
    if (err != GLEW_OK)	{
        std::cerr << "GLEW initialization error: " << glewGetErrorString(err) << std::endl;
    }

    return window;
}

void GlfwFramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    //std::cout << "Resize" << std::endl;

    glViewport(static_cast<GLint>(0),
               static_cast<GLint>(0),
               static_cast<GLsizei>(width),
               static_cast<GLsizei>(height));
    projectionMatrix = glm::perspective(45.0f, static_cast<GLfloat>(width / height), 1.0f, 1000.0f);

    Render(window);
}

void GlfwWindowRefreshCallback(GLFWwindow* window)
{
    //std::cout << "Refresh" << std::endl;
}

/**
 * Called whenever a key is pressed/released via GLFW.
 */
void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

void GlfwErrorCallback(int error, const char* description)
{
    std::cerr << "GLFW error: " << description << " error code: " << std::endl;
}
