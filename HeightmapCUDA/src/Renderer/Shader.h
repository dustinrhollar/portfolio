//
// Created by Dustin Hollar on 10/29/18.
// source: learnopengl.com
//

#ifndef _SHADER_H
#define _SHADER_H


// TODO(Dustin): Remove C++ nonesense form this file
#include <string> 
#include <iostream>

// Extract into a platform file?
#include "Util/HandmadeMath.h"

class Shader
{
    public:
    
    unsigned int id;
    
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    static Shader Init(const char* vertexPath,
                       const char* fragmentPath,
                       const char* tesselationControlPath = nullptr,
                       const char* tesselationEvaluationPath = nullptr,
                       const char* geometryPath = nullptr)
    {
        Shader result = {};
        char *vertexCode = 0,*fragmentCode = 0,*tesselationEvaluationCode = 0,*tesselationControlCode = 0,*geometryCode = 0;
        u32 vertex_size,frag_size,tess_eval_size,tess_ctrl_size,geom_size;
        
        PlatformReadFileToBuffer(vertexPath, reinterpret_cast<u8**>(&vertexCode), &vertex_size);
        PlatformReadFileToBuffer(fragmentPath, reinterpret_cast<u8**>(&fragmentCode), &frag_size);
        if (geometryPath) PlatformReadFileToBuffer(geometryPath, reinterpret_cast<u8**>(&geometryCode), &geom_size);
        if (tesselationControlPath) PlatformReadFileToBuffer(tesselationControlPath, reinterpret_cast<u8**>(&tesselationControlCode), &tess_ctrl_size);
        if (tesselationEvaluationPath) PlatformReadFileToBuffer(tesselationEvaluationPath, reinterpret_cast<u8**>(&tesselationEvaluationCode), &tess_eval_size);
        
        
        
        const char* vShaderCode = vertexCode;
        const char * fShaderCode = fragmentCode;
        // 2. compile shaders
        unsigned int vertex, fragment;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        
        // if tesselation control shader is given, compile the shader
        unsigned int tesselationControl = 0;
        if(geometryPath != nullptr)
        {
            const char * tcShaderCode = tesselationControlCode ;
            tesselationControl = glCreateShader(GL_TESS_CONTROL_SHADER);
            glShaderSource(tesselationControl, 1, &tcShaderCode, NULL);
            glCompileShader(tesselationControl);
            checkCompileErrors(tesselationControl, "TESS_CONTROL");
        }
        
        // if tesselation evaluation shader is given, compile the shader
        unsigned int tesselationEvaluation = 0;
        if(tesselationEvaluationPath != nullptr)
        {
            const char * teShaderCode = tesselationEvaluationCode ;
            tesselationEvaluation = glCreateShader(GL_TESS_EVALUATION_SHADER);
            glShaderSource(tesselationEvaluation, 1, &teShaderCode, NULL);
            glCompileShader(tesselationEvaluation);
            checkCompileErrors(tesselationEvaluation, "TESS_EVALUATION");
        }
        
        // if geometry shader is given, compile geometry shader
        unsigned int geometry = 0;
        if(geometryPath != nullptr)
        {
            const char * gShaderCode = geometryCode ;
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);
            checkCompileErrors(geometry, "GEOMETRY");
        }
        
        // shader Program
        result.id = glCreateProgram();
        glAttachShader(result.id, vertex);
        glAttachShader(result.id, fragment);
        if(tesselationControlPath != nullptr)
            glAttachShader(result.id, tesselationControl);
        if(tesselationEvaluationPath != nullptr)
            glAttachShader(result.id, tesselationEvaluation);
        if(geometryPath != nullptr)
            glAttachShader(result.id, geometry);
        glLinkProgram(result.id);
        
        Shader::checkCompileErrors(result.id, "PROGRAM");
        
        // delete the shaders as they're linked into our program now and no longer necessery
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if(tesselationControlPath != nullptr)
            glDeleteShader(tesselationControl);
        if(tesselationEvaluationPath != nullptr)
            glDeleteShader(tesselationEvaluation);
        if(geometryPath != nullptr)
            glDeleteShader(geometry);
        
        MemFree<char>(vertexCode);
        MemFree<char>(fragmentCode);
        if (geometryCode) MemFree<char>(geometryCode);
        if (tesselationEvaluationCode) MemFree<char>(tesselationEvaluationCode);
        if (tesselationControlCode) MemFree<char>(tesselationControlCode);
        
        return result;
    }
    
    void Shutdown()
    {
        glDeleteProgram(id);
    }
    
    // activate the shader
    // ------------------------------------------------------------------------
    void Use()
    {
        glUseProgram(id);
    }
    
    // utility uniform functions
    // ------------------------------------------------------------------------
    void SetBool(const char *name, bool value) const
    {
        glUniform1i(glGetUniformLocation(id, name ), (int)value);
    }
    
    // ------------------------------------------------------------------------
    void SetInt(const char *name, int value) const
    {
        glUniform1i(glGetUniformLocation(id, name ), value);
    }
    
    // ------------------------------------------------------------------------
    void SetFloat(const char *name, float value) const
    {
        glUniform1f(glGetUniformLocation(id, name), value);
    }
    
    // ------------------------------------------------------------------------
    void SetVec2(const char *name, const vec2 value) const
    {
        glUniform2fv(glGetUniformLocation(id, name), 1, &value.Elements[0]);
    }
    
    // ------------------------------------------------------------------------
    void SetIVec2(const char *name, const i32 value[2]) const
    {
        glUniform2iv(glGetUniformLocation(id, name), 1, &value[0]);
    }
    
    void SetVec2(const char *name, float x, float y) const
    {
        glUniform2f(glGetUniformLocation(id, name), x, y);
    }
    
    // ------------------------------------------------------------------------
    void SetVec3(const char *name, const vec3 value) const
    {
        glUniform3fv(glGetUniformLocation(id, name), 1, &value.Elements[0]);
    }
    
    void SetVec3(const char *name, float x, float y, float z) const
    {
        glUniform3f(glGetUniformLocation(id, name ), x, y, z);
    }
    
    void SetIVec3(const char *name, const i32 value[3]) const
    {
        glUniform3iv(glGetUniformLocation(id, name), 1, &value[0]);
    }
    
    // ------------------------------------------------------------------------
    void SetVec4(const char *name, const vec4 value) const
    {
        glUniform4fv(glGetUniformLocation(id, name ), 1, &value.Elements[0]);
    }
    
    void SetIVec4(const char *name, const i32 value[4]) const
    {
        glUniform4iv(glGetUniformLocation(id, name), 1, &value[0]);
    }
    
    void SetVec4(const char *name, float x, float y, float z, float w)
    {
        glUniform4f(glGetUniformLocation(id, name ), x, y, z, w);
    }
    
#if 0
    // ------------------------------------------------------------------------
    void SetMat2(const char *name, const mat2 mat) const
    {
        glUniformMatrix2fv(glGetUniformLocation(id, name ), 1, GL_FALSE, &mat.Elements[0]);
    }
    
    // ------------------------------------------------------------------------
    void SetMat3(const char *name, mat3 &mat) const
    {
        glUniformMatrix3fv(glGetUniformLocation(id, name ), 1, GL_FALSE, &mat.Elements[0]);
    }
    
#endif
    
    // ------------------------------------------------------------------------
    void SetMat4(const char *name, mat4 &mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(id, name ), 1, GL_FALSE, &mat.Elements[0][0]);
    }
    
    private:
    
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    static void checkCompileErrors(GLuint shader, std::string type)
    {
        GLint success;
        GLchar infoLog[1024];
        if(type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if(!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                
                
                PlatformFatalError("ERROR::SHADER_COMPILATION_ERROR of type: %s\n%s\n\n -- --------------------------------------------------- -- \n", type.c_str(), infoLog);
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if(!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                PlatformFatalError("ERROR::PROGRAM_LINKING_ERROR of type: %s\n%s\n\n -- --------------------------------------------------- -- \n", type.c_str(), infoLog);
            }
        }
    }
};

#endif //_SHADER_H
