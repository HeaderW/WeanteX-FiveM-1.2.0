


#ifndef _glfw3_native_h_
#define _glfw3_native_h_

#ifdef __cplusplus
extern "C" {
#endif


#if defined(GLFW_EXPOSE_NATIVE_WIN32)


 #undef APIENTRY
 #include <windows.h>
#elif defined(GLFW_EXPOSE_NATIVE_COCOA)
 #include <ApplicationServices/ApplicationServices.h>
 #if defined(__OBJC__)
  #import <Cocoa/Cocoa.h>
 #else
  typedef void* id;
 #endif
#elif defined(GLFW_EXPOSE_NATIVE_X11)
 #include <X11/Xlib.h>
 #include <X11/extensions/Xrandr.h>
#elif defined(GLFW_EXPOSE_NATIVE_WAYLAND)
 #include <wayland-client.h>
#elif defined(GLFW_EXPOSE_NATIVE_MIR)
 #include <mir_toolkit/mir_client_library.h>
#endif

#if defined(GLFW_EXPOSE_NATIVE_WGL)

#endif
#if defined(GLFW_EXPOSE_NATIVE_NSGL)

#endif
#if defined(GLFW_EXPOSE_NATIVE_GLX)
 #include <GL/glx.h>
#endif
#if defined(GLFW_EXPOSE_NATIVE_EGL)
 #include <EGL/egl.h>
#endif


#if defined(GLFW_EXPOSE_NATIVE_WIN32)


GLFWAPI const char* glfwGetWin32Adapter(GLFWmonitor* monitor);


GLFWAPI const char* glfwGetWin32Monitor(GLFWmonitor* monitor);


GLFWAPI HWND glfwGetWin32Window(GLFWwindow* window);
#endif

#if defined(GLFW_EXPOSE_NATIVE_WGL)


GLFWAPI HGLRC glfwGetWGLContext(GLFWwindow* window);
#endif

#if defined(GLFW_EXPOSE_NATIVE_COCOA)


GLFWAPI CGDirectDisplayID glfwGetCocoaMonitor(GLFWmonitor* monitor);


GLFWAPI id glfwGetCocoaWindow(GLFWwindow* window);
#endif

#if defined(GLFW_EXPOSE_NATIVE_NSGL)


GLFWAPI id glfwGetNSGLContext(GLFWwindow* window);
#endif

#if defined(GLFW_EXPOSE_NATIVE_X11)


GLFWAPI Display* glfwGetX11Display(void);


GLFWAPI RRCrtc glfwGetX11Adapter(GLFWmonitor* monitor);


GLFWAPI RROutput glfwGetX11Monitor(GLFWmonitor* monitor);


GLFWAPI Window glfwGetX11Window(GLFWwindow* window);
#endif

#if defined(GLFW_EXPOSE_NATIVE_GLX)


GLFWAPI GLXContext glfwGetGLXContext(GLFWwindow* window);


GLFWAPI GLXWindow glfwGetGLXWindow(GLFWwindow* window);
#endif

#if defined(GLFW_EXPOSE_NATIVE_WAYLAND)


GLFWAPI struct wl_display* glfwGetWaylandDisplay(void);


GLFWAPI struct wl_output* glfwGetWaylandMonitor(GLFWmonitor* monitor);


GLFWAPI struct wl_surface* glfwGetWaylandWindow(GLFWwindow* window);
#endif

#if defined(GLFW_EXPOSE_NATIVE_MIR)


GLFWAPI MirConnection* glfwGetMirDisplay(void);


GLFWAPI int glfwGetMirMonitor(GLFWmonitor* monitor);


GLFWAPI MirSurface* glfwGetMirWindow(GLFWwindow* window);
#endif

#if defined(GLFW_EXPOSE_NATIVE_EGL)


GLFWAPI EGLDisplay glfwGetEGLDisplay(void);


GLFWAPI EGLContext glfwGetEGLContext(GLFWwindow* window);


GLFWAPI EGLSurface glfwGetEGLSurface(GLFWwindow* window);
#endif

#ifdef __cplusplus
}
#endif

#endif

