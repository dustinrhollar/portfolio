
static void shader_program_check_shader_compile(u32 shader);
static void shader_program_check_program_compile(u32 program);

GLenum shader_program_to_gl_shader(EShaderStage stage)
{
    u32 result = 0;
    
    if (stage == ShaderStage_Vertex)        result = GL_VERTEX_SHADER;
    else if (stage == ShaderStage_Fragment) result = GL_FRAGMENT_SHADER;
    else if (stage == ShaderStage_Geom)     result = GL_GEOMETRY_SHADER;
    else if (stage == ShaderStage_TessEval) result = GL_TESS_EVALUATION_SHADER;
    else if (stage == ShaderStage_TessCon)  result = GL_TESS_CONTROL_SHADER;
    else if (stage == ShaderStage_Compute)  result = GL_COMPUTE_SHADER;

    return result;
}

void shader_program_init(ShaderProgram *shader, i32 stages, ...)
{
    u32 shader_comp[ShaderStage_Count];
    *shader = glCreateProgram();

    va_list list;
    va_start(list, stages);

    for (i32 i = 0; i < stages; ++i)
    {
        EShaderStage stage = va_arg(list, EShaderStage);
        const char *file = va_arg(list, const char *);

        char *buffer = 0;
        u32 buffer_size;
        PlatformErrorType err = PlatformReadFileToBuffer(file, (u8**)&buffer, &buffer_size);
        if (err != PlatformError_Success)
        {
            LogError("Failed to load shader file: %s", file);
            *shader = INVALID_SHADER_PROGRAM;
            return;
        }

        shader_comp[i] = glCreateShader(shader_program_to_gl_shader(stage));
        glShaderSource(shader_comp[i], 1, (const char* const*)&buffer, NULL);
        glCompileShader(shader_comp[i]);
        shader_program_check_shader_compile(shader_comp[i]);
        glAttachShader(*shader, shader_comp[i]);

        MemFree(buffer);
    }

    va_end(list);

    glLinkProgram(*shader);
    shader_program_check_program_compile(*shader);

    for (i32 i = 0; i < stages; ++i)
    {
        glDeleteShader(shader_comp[i]);
    }
}


void shader_program_free(ShaderProgram *shader)
{
    glDeleteProgram(*shader);
    *shader = INVALID_SHADER_PROGRAM;
}

void shader_program_bind(ShaderProgram *shader)
{
    glUseProgram(*shader);
}

void shader_program_unbind(ShaderProgram *shader)
{
    glUseProgram(0);
}

void shader_program_set_m4(ShaderProgram *shader, const char *name, m4 mat)
{
    glUniformMatrix4fv(glGetUniformLocation(*shader, name), 1, GL_FALSE, &mat.p[0][0]);
}

void shader_program_set_float(ShaderProgram *shader, const char *name, r32 val)
{
    glUniform1f(glGetUniformLocation(*shader, name), val);
}

void shader_program_set_int(ShaderProgram *shader, const char *name, int val)
{
    glUniform1i(glGetUniformLocation(*shader, name), val);
}

void shader_program_set_v4(ShaderProgram *shader, const char *name, v4 vec)
{
    glUniform4fv(glGetUniformLocation(*shader, name), 1, &vec.p[0]);
}

void shader_program_set_uniform_block(ShaderProgram *shader, const char *name, u32 block)
{
    u32 uniform = glGetUniformBlockIndex(*shader, name);
    glUniformBlockBinding(*shader, uniform, block);
}

static void shader_program_check_shader_compile(u32 shader)
{
    GLint success;
    GLchar info_log[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 1024, NULL, info_log);
        LogError("ERROR::SHADER_COMPILATION_ERROR::%s", info_log);
    }
}

static void shader_program_check_program_compile(u32 program)
{
    GLint success;
    GLchar info_log[1024];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 1024, NULL, info_log);
        LogError("ERROR::PROGRAM_COMPILATION_ERROR::%s", info_log);
    }
}
