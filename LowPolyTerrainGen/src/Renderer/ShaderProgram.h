#ifndef _SHADER_PROGRAM
#define _SHADER_PROGRAM

typedef i32 ShaderProgram;
#define INVALID_SHADER_PROGRAM -1

typedef enum EShaderStage
{
    ShaderStage_Vertex,    // Vertex Shader Stage
    ShaderStage_Fragment,  // Frament Shader Stage
    ShaderStage_Geom,      // Geometry Shader Stage
    ShaderStage_TessEval,  // Tesselation Evaluation Shader Stage
    ShaderStage_TessCon,   // Tesselation Control Shader Stage
    ShaderStage_Compute,   // Compute Shader Stage

    ShaderStage_Count,
    ShaderStage_Unknown = ShaderStage_Count,
} EShaderStage;

// To initialize the shader program, a user should provide a file + shader type stage.
// A user must provide a vertex AND fragment shader at minimum
//
// Example:
// shader_program_init(shader, ShaderType_Vertex, "shader.vert", ShaderType_Fragment, "shader.frag")
void shader_program_init(ShaderProgram *shader, i32 stages, ...);
void shader_program_free(ShaderProgram *shader);

void shader_program_bind(ShaderProgram *shader);
void shader_program_unbind(ShaderProgram *shader);
void shader_program_set_m4(ShaderProgram *shader, const char *name, m4 mat);
void shader_program_set_int(ShaderProgram *shader, const char *name, int val);
void shader_program_set_float(ShaderProgram *shader, const char *name, r32 val);
void shader_program_set_v4(ShaderProgram *shader, const char *name, v4 vec);
void shader_program_set_uniform_block(ShaderProgram *shader, const char *name, u32 block);

#endif //_SHADER_PROGRAM
