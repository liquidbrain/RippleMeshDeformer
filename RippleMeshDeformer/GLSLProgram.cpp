#include "GLSLProgram.h"

#include <fstream>
#include <iostream>
#include <sstream>

GLSLProgram::GLSLProgram() :
        _shaderProgramHandle(0),
        _didLink(GL_FALSE),
        _vertexShader(0),
        _fragmentShader(0),
        _geometryShader(0)
{ }

GLSLProgram::~GLSLProgram()
{
    DeleteProgram();
}

// Build and compile a shader
void GLSLProgram::AddShader(GLenum shaderType, const GLchar* const source)
{
    GLuint shader = glCreateShader(shaderType); // creates an empty shader object
    glShaderSource(shader,      // shader to be compiled
            1,                  // number of lines of shader code
            &source,            // reference to pointer(s) of strings (lines of source code)
            nullptr);           // array of string lengths

    glCompileShader(shader);

    GLint didCompile = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &didCompile);
    if (didCompile == GL_TRUE) {
        switch (shaderType) {
            case GL_VERTEX_SHADER :
                _vertexShader = shader;
                break;
            case GL_FRAGMENT_SHADER :
                _fragmentShader = shader;
                break;
            case GL_GEOMETRY_SHADER :
                _geometryShader = shader;
        }
    }
    else  {
        GLint logLength = 0;    // this will include the NULL character
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        GLchar* logBuffer = new GLchar[logLength];
        glGetShaderInfoLog(shader, logLength, nullptr, logBuffer);

        std::cerr << "GLSLProgram::AddShader compile error: " << logBuffer << std::endl;

        delete[] logBuffer;     // don't leak the log buffer
        glDeleteShader(shader); // don't leak the shader
    }
}

// Build and compile a shader
void GLSLProgram::AddShader(GLenum shaderType, const std::string& source)
{
    const char* sourceAsChar = source.c_str();
    AddShader(shaderType, sourceAsChar);
}

// Build and compile a shader
void GLSLProgram::AddShaderFromFile(GLenum shaderType, const std::string& filename)
{
    std::ifstream inputStream;
    inputStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        inputStream.open(filename.c_str(), std::ios_base::in);

        std::stringstream shaderSource;
        shaderSource << inputStream.rdbuf();

        inputStream.close();

        std::string shaderSourceAsString = shaderSource.str();
        AddShader(shaderType, shaderSourceAsString);
    }
    catch(std::ios_base::failure& e) {
        std::cerr << "GLSLProgram::AddShaderFromFile: " << filename << " " << e.what() << " error code: " << e.code() << std::endl;
    }
}

GLuint GLSLProgram::CreateAndLinkProgram() {
    _shaderProgramHandle = glCreateProgram(); // create a shader program and return a reference to it

    if (_vertexShader != 0) {
        glAttachShader(_shaderProgramHandle, _vertexShader);
    }
    if (_fragmentShader != 0) {
        glAttachShader(_shaderProgramHandle, _fragmentShader);
    }
    if (_geometryShader != 0) {
        glAttachShader(_shaderProgramHandle, _geometryShader);
    }

    glLinkProgram(_shaderProgramHandle);

    glGetProgramiv(_shaderProgramHandle, GL_LINK_STATUS, &_didLink);
    if (_didLink != GL_TRUE) {
        GLint logLength = 0;    // this will include the NULL character
        glGetProgramiv(_shaderProgramHandle, GL_INFO_LOG_LENGTH, &logLength);

        GLchar* logBuffer = new GLchar[logLength];
        glGetProgramInfoLog(_shaderProgramHandle, logLength, nullptr, logBuffer);

        std::cerr << "GLSLProgram::AddShader link error: " << logBuffer << std::endl;

        delete[] logBuffer;
    }

    glDeleteShader(_vertexShader);
    glDeleteShader(_fragmentShader);
    glDeleteShader(_geometryShader);

    return _shaderProgramHandle;
}

GLuint GLSLProgram::IsCreated() const
{
    return _didLink;
}

void GLSLProgram::UseProgram()
{
    glUseProgram(_shaderProgramHandle);
}

void GLSLProgram::DeleteProgram()
{
    glDeleteProgram(_shaderProgramHandle);
}

// Maps an attribute name to its location.
// If successful:
// Returns the location of the attribute.
// If the attribute argument is a matrix attribute variable, the index of the first column of the
// matrix is returned.
// If error:
// Returns -1 if the named attribute variable is not an active attribute in the specified program
// or if the supplied attribute name starts with the reserved prefix "gl_".
GLint GLSLProgram::AddAttribute(const std::string& attribute)
{
    GLint returnCode = -1;
    if (_didLink) {
        returnCode = _attributeList[attribute] = glGetAttribLocation(_shaderProgramHandle, attribute.c_str());
    }
    return returnCode;
}

// Maps a uniform name to its location.
// If successful:
// Returns the location of the uniform.
// If error:
// Returns -1 if name does not correspond to an active uniform variable in program or if name
// starts with the reserved prefix "gl_".
GLint GLSLProgram::AddUniform(const std::string& uniform)
{
    GLint returnCode = -1;
    if (_didLink) {
        returnCode = _uniformList[uniform] = glGetUniformLocation(_shaderProgramHandle, uniform.c_str());
    }
    return returnCode;
}

GLuint GLSLProgram::GetAttributeLocation(const std::string& attribute) const
{
    GLuint location = -1;
    std::map<std::string, GLuint>::const_iterator iterator = _attributeList.find(attribute);
    if (iterator != _attributeList.end()) {
        location = iterator->second;
    }
    return location;
}

GLuint GLSLProgram::GetUniformLocation(const std::string& uniform) const
{
    GLuint location = -1;
    std::map<std::string, GLuint>::const_iterator iterator = _uniformList.find(uniform);
    if (iterator != _uniformList.end()) {
        location = iterator->second;
    }
    return location;
}

std::string GLSLProgram::ToString() const
{
    std::ostringstream programData;
    GLint maxBufferSize, nbrOfAttributes, nbrOfUniforms, charsInBuffer, size, location;
    GLchar* name;
    GLenum type;

    programData << "GLSL program handle: " << _shaderProgramHandle << "\n";

    glGetProgramiv(_shaderProgramHandle, GL_ACTIVE_ATTRIBUTES, &nbrOfAttributes);
    glGetProgramiv(_shaderProgramHandle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxBufferSize);
    name = new GLchar[maxBufferSize];

    for (int index = 0; index < nbrOfAttributes; ++index) {
        glGetActiveAttrib(_shaderProgramHandle, index, maxBufferSize, &charsInBuffer, &size, &type, name);
        location = glGetAttribLocation(_shaderProgramHandle, name);

        programData << "Attribute name: " << name << " location: " << location << "\n";
    }

    delete[] name;

    glGetProgramiv(_shaderProgramHandle, GL_ACTIVE_UNIFORMS, &nbrOfUniforms);
    glGetProgramiv(_shaderProgramHandle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxBufferSize);
    name = new GLchar[maxBufferSize];

    for (int index = 0; index < nbrOfUniforms; ++index) {
        glGetActiveUniform(_shaderProgramHandle, index, maxBufferSize, &charsInBuffer, &size, &type, name);
        location = glGetUniformLocation(_shaderProgramHandle, name);

        programData << "Uniform name:   " << name << " location: " << location << "\n";
    }

    delete[] name;

    return programData.str();
}
