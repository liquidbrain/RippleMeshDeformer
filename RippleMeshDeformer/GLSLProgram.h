#pragma once

#include <map>
#include <string>

// GLEW: OpenGL Extension Wrangler
#include <GL/glew.h>

// GLM: OpenGL Math
#include <glm/glm.hpp>

class GLSLProgram
{
public:
    GLSLProgram();

    GLSLProgram(const GLSLProgram& rhs) = delete;
    GLSLProgram(GLSLProgram&& rhs) = delete;

    GLSLProgram& operator=(const GLSLProgram& rhs) = delete;
    GLSLProgram& operator=(GLSLProgram&& rhs) = delete;

    ~GLSLProgram();

    void AddShader(GLenum shaderType, const GLchar* const source);
    void AddShader(GLenum shaderType, const std::string &source);
    void AddShaderFromFile(GLenum shaderType, const std::string &filename);

    GLuint CreateAndLinkProgram();

    GLuint IsCreated() const;

    void UseProgram();

    void DeleteProgram();

    GLint AddAttribute(const std::string& attribute);
    GLint AddUniform(const std::string& uniform);

    GLuint GetAttributeLocation(const std::string& attribute) const;
    GLuint GetUniformLocation(const std::string& uniform) const;

    std::string ToString() const;

    // Uniform convenience functions =============================================================

    void setBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(_shaderProgramHandle, name.c_str()), (int)value);
    }

    void setInt(const std::string &name, int value) const
    {
        glUniform1i(glGetUniformLocation(_shaderProgramHandle, name.c_str()), value);
    }

    void setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(_shaderProgramHandle, name.c_str()), value);
    }

    void setVec2(const std::string& name, const glm::vec2& value) const
    {
        glUniform2fv(glGetUniformLocation(_shaderProgramHandle, name.c_str()), 1, &value[0]);
    }

    void setVec2(const std::string& name, float x, float y) const
    {
        glUniform2f(glGetUniformLocation(_shaderProgramHandle, name.c_str()), x, y);
    }

    void setVec3(const std::string& name, const glm::vec3& value) const
    {
        glUniform3fv(glGetUniformLocation(_shaderProgramHandle, name.c_str()), 1, &value[0]);
    }

    void setVec3(const std::string& name, float x, float y, float z) const
    {
        glUniform3f(glGetUniformLocation(_shaderProgramHandle, name.c_str()), x, y, z);
    }

    void setVec4(const std::string& name, const glm::vec4& value) const
    {
        glUniform4fv(glGetUniformLocation(_shaderProgramHandle, name.c_str()), 1, &value[0]);
    }
    void setVec4(const std::string& name, float x, float y, float z, float w)
    {
        glUniform4f(glGetUniformLocation(_shaderProgramHandle, name.c_str()), x, y, z, w);
    }

    void setMat2(const std::string& name, const glm::mat2& mat) const
    {
        glUniformMatrix2fv(glGetUniformLocation(_shaderProgramHandle, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void setMat3(const std::string& name, const glm::mat3& mat) const
    {
        glUniformMatrix3fv(glGetUniformLocation(_shaderProgramHandle, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void setMat4(const std::string& name, const glm::mat4& mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(_shaderProgramHandle, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

private:
    GLuint _shaderProgramHandle;

    GLint _didLink; // GL_TRUE if the GLSL program was successfully created and linked

    GLuint _vertexShader;
    GLuint _fragmentShader;
    GLuint _geometryShader;

    std::map<std::string, GLuint> _attributeList;   // maps attribute names to locations
    std::map<std::string, GLuint> _uniformList;     // maps uniform names to locations
};
