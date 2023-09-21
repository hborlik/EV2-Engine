#include "renderer/opengl_renderer/gl_render_api.hpp"
#include "renderer/opengl_renderer/ev_gl.hpp"

#include "core/log.hpp"
#include "window.hpp"


#define EV_GL_MIN_SEVERITY GL_DEBUG_SEVERITY_LOW
#define EV_GL_ALLOW_NOTIFICATIONS false

namespace {

// gl error callback
__attribute__ ((stdcall))
void gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
        GLsizei length, const GLchar *message, const void *userParam) {
    std::string output_severity{};
    std::string output_source{};
    std::string output_type{};
    std::string output_message{message};
    spdlog::level::level_enum level = spdlog::level::info;

    if (severity >= EV_GL_MIN_SEVERITY || (severity == GL_DEBUG_SEVERITY_NOTIFICATION && !EV_GL_ALLOW_NOTIFICATIONS))
        return;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        output_severity = "NOTIFICATION";
        level = spdlog::level::info;
        break;
    case GL_DEBUG_SEVERITY_LOW:
        output_severity = "LOW";
        level = spdlog::level::warn;
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        output_severity = "MEDIUM";
        level = spdlog::level::warn;
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        output_severity = "HIGH";
        level = spdlog::level::err;
        break;
    default:
        break;
    }

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        output_source = "API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        output_source = "WINDOW_SYSTEM";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        output_source = "THIRD_PARTY";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        output_source = "APPLICATION";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        output_source = "OTHER";
        break;
    default:
        break;
    }

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        output_type = "ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        output_type = "DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        output_type = "UNDEFINED_BEHAVIOR";
        break;
    //case GL_DEBUG_TYPE_PORTABILITIY:
    //    break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        output_type = "PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_MARKER:
        output_type = "MARKER";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        // output_type = "PUSH_GROUP";
        return;
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        // output_type = "POP_GROUP";
        return;
        break;
    case GL_DEBUG_TYPE_OTHER:
        output_type = "OTHER";
        break;
    default:
        break;
    }

    ev2::Log::log_core(level, "OpenGL[{}:{}:{}] {}", output_source, output_severity, output_type, output_message);
}

}

namespace ev2::renderer {

void GLRenderAPI::init() {
    // load gl functions
    ev2::Log::info_core("Initializing OpenGL GLAD...");
    if(!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        ev2::Log::critical_core("Failed to initialize GLAD");
        throw engine_exception{"Failed to initialize GLAD"};
    }
    ev2::Log::info_core("GLAD Init complete.");

    // only for core version 4.3 and later
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(gl_debug_callback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);

    const GLubyte* renderer = glGetString( GL_RENDERER );
    const GLubyte* vendor = glGetString( GL_VENDOR );
    const GLubyte* version = glGetString( GL_VERSION );
    const GLubyte* glslVersion = glGetString( GL_SHADING_LANGUAGE_VERSION );

    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    
    ev2::Log::info_core("GL: {} {}\n", std::string_view{(char*)vendor}, std::string_view{(char*)renderer});
    ev2::Log::info_core("GL Version: {} ({}.{})\n", std::string_view{(char*)version}, major, minor);
    ev2::Log::info_core("GLSL Version : {}\n", std::string_view{(char*)glslVersion});

    // push a test message
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, 25, "GL TEST DEBUG MESSAGE");
}

}