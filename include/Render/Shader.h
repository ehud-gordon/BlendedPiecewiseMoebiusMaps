#pragma once

#include "Utils/Constants.h"
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <vector>

std::string getShaderTypeName(GLenum type);
std::string readShaderFile(const std::string& shaderPath);
void checkCompileErrors(GLuint shader, GLenum type, const std::string& shader_name = "");
std::string extractShaderName(const std::string& shaderPath);
unsigned int compileShaderProgram(const std::vector<std::string>& shaderPaths, const std::vector<GLenum>& shaderTypes, const std::string& shader_name="");

class Shader {
public:
    unsigned int ID;
    std::string shader_name;
    Shader() {}
    Shader(const std::vector<std::string>& shaderPaths, const std::vector<GLenum>& shaderTypes) {
        if (shaderPaths.size() != shaderTypes.size()) { std::cerr << "ERROR::SHADER::PATHS_AND_TYPES_SIZE_MISMATCH" << std::endl; return; }
        if (shaderPaths.size() < 1) { std::cerr << "ERROR::SHADER::NO_SHADER_PATHS_PROVIDED" << std::endl; return;  }

        shader_name = extractShaderName(shaderPaths[0]);
        ID = compileShaderProgram(shaderPaths, shaderTypes, shader_name);
    }

    
    // activate the shader
    // ------------------------------------------------------------------------
    void use() 
    { 
        glUseProgram(ID); 
    }
    void disable() 
    { 
        glUseProgram(0); 
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string &name, bool value) const
    {         
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string &name, int value) const
    { 
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
    }
    // ------------------------------------------------------------------------
    void setUInt(const std::string &name, unsigned int value) const
    { 
        glUniform1ui(glGetUniformLocation(ID, name.c_str()), value); 
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string &name, float value) const
    { 
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
    }
    // ------------------------------------------------------------------------
    void setVec2(const std::string &name, const glm::vec2 &value) const
    { 
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
    }
    void setVec2(const std::string &name, float x, float y) const
    { 
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); 
    }
    // ------------------------------------------------------------------------
    void setVec3(const std::string &name, const glm::vec3 &value) const
    { 
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
    }
    void setVec3(const std::string &name, float x, float y, float z) const
    { 
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); 
    }
    // ------------------------------------------------------------------------
    void setVec4(const std::string &name, const glm::vec4 &value) const
    { 
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
    }
    void setVec4(const std::string &name, float x, float y, float z, float w)  { 
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); 
    }
    // ------------------------------------------------------------------------
    void setMat2(const std::string &name, const glm::mat2 &mat) const {
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat3(const std::string &name, const glm::mat3 &mat) const {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
};
