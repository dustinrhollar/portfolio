//
// Created by Dustin Hollar on 10/29/18.
// source: learnopengl.com
//

#ifndef TERRAINGENERATORTEST_SHADER_H
#define TERRAINGENERATORTEST_SHADER_H


#include <glad/glad.h>
#include <glm/glm.hpp>

inline int GetFileSize(FILE *fp)
{
	fseek(fp, 0, SEEK_END);
	int result =  ftell(fp);
	fseek(fp, 0, SEEK_SET);
	return result;
}

class Shader
{
    public:
    unsigned int ID;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath,
           const char* fragmentPath,
           const char* tesselationControlPath = nullptr,
           const char* tesselationEvaluationPath = nullptr,
           const char* geometryPath = nullptr)
    {
		printf("Vertex   Shader: %s\n", vertexPath);
		printf("Fragment Shader: %s\n", fragmentPath);
		printf("Control Shader: %s\n", tesselationControlPath);
		printf("Eval Shader: %s\n", tesselationEvaluationPath);
		
        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode, fragmentCode;
        std::string tesselationEvaluationCode;
        std::string geometryCode;
        std::ifstream vShaderFile, fShaderFile;
        std::ifstream tcShaderFile, teShaderFile;
        std::ifstream gShaderFile;
        char *tess_ctrl_code = 0;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        tcShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        teShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        gShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            // open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
            
            // if tesselation control shader path is present, also load a tesselation control shader
            if(tesselationControlPath != nullptr)
            {
				FILE * fp = fopen (tesselationControlPath, "r");
				assert(fp);
				int size = GetFileSize(fp);
				tess_ctrl_code = (char*)malloc(size);
				fread(tess_ctrl_code, size, 1, fp);
                fclose(fp);
            }
            
            // if geometry shader path is present, also load a geometry shader
            if(tesselationEvaluationPath != nullptr)
            {
                teShaderFile.open(tesselationEvaluationPath);
                std::stringstream teShaderStream;
                teShaderStream << teShaderFile.rdbuf();
                teShaderFile.close();
                tesselationEvaluationCode = teShaderStream.str();
            }
            
            // if geometry shader path is present, also load a geometry shader
            if(geometryPath != nullptr)
            {
                gShaderFile.open(geometryPath);
                std::stringstream gShaderStream;
                gShaderStream << gShaderFile.rdbuf();
                gShaderFile.close();
                geometryCode = gShaderStream.str();
            }
        }
        catch (std::ifstream::failure e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        }
        const char* vShaderCode = vertexCode.c_str();
        const char * fShaderCode = fragmentCode.c_str();
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
        unsigned int tesselationControl;
        if(tesselationControlPath != nullptr)
        {
            //const char * tcShaderCode = tesselationControlCode.c_str(); printf("Control: %s\n", tcShaderCode);
            tesselationControl = glCreateShader(GL_TESS_CONTROL_SHADER);
            glShaderSource(tesselationControl, 1, &tess_ctrl_code, NULL);
            glCompileShader(tesselationControl);
            checkCompileErrors(tesselationControl, "TESS_CONTROL");
            free(tess_ctrl_code);
        }
        
        // if tesselation evaluation shader is given, compile the shader
        unsigned int tesselationEvaluation;
        if(tesselationEvaluationPath != nullptr)
        {
            const char * teShaderCode = tesselationEvaluationCode.c_str();
            tesselationEvaluation = glCreateShader(GL_TESS_EVALUATION_SHADER);
            glShaderSource(tesselationEvaluation, 1, &teShaderCode, NULL);
            glCompileShader(tesselationEvaluation);
            checkCompileErrors(tesselationEvaluation, "TESS_EVALUATION");
        }
        
        // if geometry shader is given, compile geometry shader
        unsigned int geometry;
        if(geometryPath != nullptr)
        {
            const char * gShaderCode = geometryCode.c_str();
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);
            checkCompileErrors(geometry, "GEOMETRY");
        }
        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        if(tesselationControlPath != nullptr)
           glAttachShader(ID, tesselationControl);
        if(tesselationEvaluationPath != nullptr)
            glAttachShader(ID, tesselationEvaluation);
        if(geometryPath != nullptr)
            glAttachShader(ID, geometry);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessery
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if(tesselationControlPath != nullptr)
            glDeleteShader(tesselationControl);
        if(tesselationEvaluationPath != nullptr)
            glDeleteShader(tesselationEvaluation);
        if(geometryPath != nullptr)
            glDeleteShader(geometry);
        
    }
    // activate the shader
    // ------------------------------------------------------------------------
    void use()
    {
        glUseProgram(ID);
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
    void setFloat(const char *name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name), value);
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
    void setVec3(const char *name, const glm::vec3 &value) const
    {
        glUniform3fv(glGetUniformLocation(ID, name), 1, &value[0]);
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
    void setVec4(const std::string &name, float x, float y, float z, float w)
    {
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    }
    // ------------------------------------------------------------------------
    void setMat2(const std::string &name, const glm::mat2 &mat) const
    {
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat3(const std::string &name, const glm::mat3 &mat) const
    {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string &name, const glm::mat4 &mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    
    private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(GLuint shader, std::string type)
    {
        GLint success;
        GLchar infoLog[1024];
        if(type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if(!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if(!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};
#endif //TERRAINGENERATORTEST_SHADER_H
