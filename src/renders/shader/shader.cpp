#include "renders/shader/shader.h"

#include <glm/gtc/type_ptr.hpp>
#include <glad/gl.h>

#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

namespace {
    std::string readShader(const std::string& path){
        std::ifstream file(path);

        if(!file){
            throw std::runtime_error("Couldn't find shader file: Is " + path + " the correct path?");
        }

        std::stringstream bufferOutput;
        bufferOutput << file.rdbuf();
        return bufferOutput.str();
    }

    uint32_t compileShader(GLenum type, const std::string& source, const std::string& path){
        uint32_t shader = glCreateShader(type);
        const char* inputSource = source.c_str();

        glShaderSource(shader, 1, &inputSource, nullptr);
        glCompileShader(shader);

        int status = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if(!status){
            int info = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info);
            std::vector<char> log(info);
            glGetShaderInfoLog(shader, info, nullptr, log.data());
            throw std::runtime_error("Couldn't compile shader: Check " + path + " status for potential issues. \nLog: " + std::string(log.data(), log.size()));
        }

        return shader;
    }
}

/*
    Summary: This class handles the vert/frag shader processing.
    The class compiles the input shaders given in the constructor,
    then binds the compiled shader via use(). There are also useful helper
    functions uniSet() that allow uniform set of any specified data type
*/
Shader::Shader(const std::string& fragPath, const std::string& vertPath){
    uint32_t fragFile = compileShader(GL_FRAGMENT_SHADER, readShader(fragPath), fragPath);
    uint32_t vertFile = compileShader(GL_VERTEX_SHADER, readShader(vertPath), vertPath);

    program_ = glCreateProgram();
    glAttachShader(program_, fragFile);
    glAttachShader(program_, vertFile);
    glLinkProgram(program_);

    int status = 0;
    glGetProgramiv(program_, GL_LINK_STATUS, &status);
    if(!status){
        int info = 0;
        glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &info);
        std::vector<char> log(info);
        glGetProgramInfoLog(program_, info, nullptr, log.data());

        glDeleteShader(fragFile);
        glDeleteShader(vertFile);

        throw std::runtime_error("Couldn't link shader files. \nLog: " + std::string(log.data(), log.size()));
    }

    glDeleteShader(fragFile);
    glDeleteShader(vertFile);
}

Shader::~Shader(){
    if(program_){
        glDeleteProgram(program_);
    }
}

void Shader::use() const{
    glUseProgram(program_);
}

int Shader::uniLocation(const std::string& loc) const{
    return glGetUniformLocation(program_, loc.c_str());
}

void Shader::uniSet(const std::string& loc, const glm::vec2& val) const{
    glUniform2fv(uniLocation(loc), 1, glm::value_ptr(val));
}

void Shader::uniSet(const std::string& loc, const glm::vec3& val) const{
    glUniform3fv(uniLocation(loc), 1, glm::value_ptr(val));
}

void Shader::uniSet(const std::string& loc, const glm::mat4& val) const{
    glUniformMatrix4fv(uniLocation(loc), 1, GL_FALSE, glm::value_ptr(val));
}

void Shader::uniSet(const std::string& loc, const std::vector<glm::mat4>& vals) const{
    glUniformMatrix4fv(
        uniLocation(loc), 
        static_cast<GLsizei>(vals.size()), 
        GL_FALSE,
        glm::value_ptr(vals[0])
    );
}

void Shader::uniSet(const std::string& loc, int val) const{
    glUniform1i(uniLocation(loc), val);
}