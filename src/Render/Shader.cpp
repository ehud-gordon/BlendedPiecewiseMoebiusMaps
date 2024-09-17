#include "Render/Shader.h"


unsigned int compileShaderProgram(const std::vector<std::string>& shaderPaths, const std::vector<GLenum>& shaderTypes, const std::string& shader_name) {
    unsigned int programID = glCreateProgram();
    std::vector<unsigned int> shaderIDs;
    shaderIDs.reserve(shaderPaths.size());
    // Compile each shader
    for (size_t i = 0; i < shaderPaths.size(); ++i) {
        std::string shaderCode = readShaderFile(shaderPaths[i]);
        const char* shaderSource = shaderCode.c_str();
        shaderIDs[i] = glCreateShader(shaderTypes[i]);
        glShaderSource(shaderIDs[i], 1, &shaderSource, nullptr);
        glCompileShader(shaderIDs[i]);
        checkCompileErrors(shaderIDs[i], shaderTypes[i], shader_name);

        // Attach compiled shader to program
        glAttachShader(programID, shaderIDs[i]);
    }

    // Link the program
    glLinkProgram(programID);
    checkCompileErrors(programID, GL_LINK_STATUS, shader_name);

    // Delete shaders as they're linked into our program now and no longer necessary
    for (size_t i = 0; i < shaderPaths.size(); ++i){
        glDeleteShader(shaderIDs[i]);
    }
    return programID;
}

std::string getShaderTypeName(GLenum type) {
    switch (type)
    {
    case GL_VERTEX_SHADER: return "VERTEX";
    case GL_FRAGMENT_SHADER: return "FRAGMENT";
    case GL_GEOMETRY_SHADER: return "GEOMETRY";
    case GL_COMPUTE_SHADER: return "COMPUTE";
    default: return "UNKNOWN";
    }
}

std::string readShaderFile(const std::string& shaderPath) {
    std::string shaderCode;
    std::ifstream shaderFile;
    // ensure ifstream objects can throw exceptions:
    shaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    try 
    {
        // open files
        shaderFile.open(shaderPath);
        std::stringstream shaderStream;
        // read file's buffer contents into streams
        shaderStream << shaderFile.rdbuf();
        // close file handlers
        shaderFile.close();
        // convert stream into string
        shaderCode = shaderStream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }
    return shaderCode;
}

void checkCompileErrors(GLuint shader, GLenum type, const std::string& shader_name) {
    GLint success;
    GLchar infoLog[1024];
    if(type != GL_LINK_STATUS) {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << getShaderTypeName(type) << " at " << shader_name << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if(!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << " at " << shader_name << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

std::string extractShaderName(const std::string& shaderPath) {
    std::filesystem::path path(shaderPath);
    std::string baseName = path.stem().string(); // Get the base filename
    std::string parentFolder;

    if (path.has_parent_path()) {
        parentFolder = path.parent_path().filename().string(); // Get the parent folder name
    }

    if (!parentFolder.empty()) {
        return parentFolder + "/" + baseName;
    } else {
        return baseName;
    }
}

