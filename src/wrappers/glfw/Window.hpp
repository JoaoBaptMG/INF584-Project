#pragma once

#include "Monitor.hpp"
#include "WindowHint.hpp"
#include "Utils.hpp"
#include "Exception.hpp"
#include "Input.hpp"
#include "Cursor.hpp"
#include "util/array_view.hpp"
#include "wrappers/glException.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shared_mutex>
#include <concepts>

namespace glfw
{
	class Window final
	{
		// Handle
		GLFWwindow* window;
		static inline Window* currentContext = nullptr;

	public:
		// default constructor
		Window() noexcept : window(nullptr) {}

		// Constructors
		Window(int width, int height, std::string title, Monitor monitor, const Window& share,
			const WindowHint& windowHint = WindowHint())
		{
			windowHint.applyHints();
			window = glfwCreateWindow(width, height, title.c_str(), monitor.monitor, share.window);
			checkError();
		}

		Window(int width, int height, std::string title, const WindowHint& windowHint = WindowHint())
			: Window(width, height, title, Monitor(), Window(), windowHint) {}
		Window(int width, int height, std::string title, Monitor monitor, const WindowHint& windowHint = WindowHint())
			: Window(width, height, title, monitor, Window(), windowHint) {}
		Window(int width, int height, std::string title, Window& share, const WindowHint& windowHint = WindowHint())
			: Window(width, height, title, Monitor(), share, windowHint) {}

		// borderless full screen
		static Window borderless(std::string title, Monitor monitor, const Window& share,
			WindowHint windowHint = WindowHint())
		{
			const auto& videoMode = monitor.getVideoMode();
			windowHint.redBits(videoMode.redBits);
			windowHint.greenBits(videoMode.greenBits);
			windowHint.blueBits(videoMode.blueBits);
			windowHint.refreshRate(videoMode.refreshRate);
			return Window(videoMode.width, videoMode.height, title, monitor, share, windowHint);
		}
		static Window borderless(std::string title, WindowHint windowHint = WindowHint())
		{
			return borderless(title, Monitor::getPrimaryMonitor(), Window(), windowHint);
		}
		static Window borderless(std::string title, Monitor monitor, WindowHint windowHint = WindowHint())
		{
			return borderless(title, monitor, Window(), windowHint);
		}
		static Window borderless(std::string title, const Window& share, WindowHint windowHint = WindowHint())
		{
			return borderless(title, Monitor::getPrimaryMonitor(), share, windowHint);
		}

		// Disable copying, enable moving
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		Window(Window&& other) noexcept : window(other.window) { other.window = 0; }
		Window& operator=(Window&& other) noexcept
		{
			std::swap(window, other.window);
			return *this;
		}

		// API
		bool shouldClose() const { return checkError(glfwWindowShouldClose(window)); }
		void setShouldClose(bool close = true) { glfwSetWindowShouldClose(window, close); checkError(); }

		void setTitle(std::string title) { glfwSetWindowTitle(window, title.c_str()); checkError(); }

		auto getPos() const
		{
			IntCoord coord;
			glfwGetWindowPos(window, &coord.x, &coord.y);
			checkError();
			return coord;
		}
		void setPos(IntCoord coord) { setPos(coord.x, coord.y); }
		void setPos(int x, int y) { glfwSetWindowPos(window, x, y); checkError(); }

		auto getSize() const
		{
			Size ws;
			glfwGetWindowSize(window, &ws.width, &ws.height);
			checkError();
			return ws;
		}
		void setSize(Size ws) { setSize(ws.width, ws.height); }
		void setSize(int width, int height) { glfwSetWindowSize(window, width, height); checkError(); }

		void setSizeLimits(Size min, Size max)
		{
			setSizeLimits(min.width, min.height, max.width, max.height);
		}
		void setSizeLimits(int minwidth, int minheight, int maxwidth, int maxheight)
		{
			glfwSetWindowSizeLimits(window, minwidth, minheight, maxwidth, maxheight);
			checkError();
		}
		void setMaxSizeLimit(Size max) { setMaxSizeLimit(max.width, max.height); }
		void setMaxSizeLimit(int maxwidth, int maxheight)
		{
			glfwSetWindowSizeLimits(window, GLFW_DONT_CARE, GLFW_DONT_CARE, maxwidth, maxheight);
			checkError();
		}
		void setMinSizeLimit(Size min) { setMinSizeLimit(min.width, min.height); }
		void setMinSizeLimit(int minwidth, int minheight)
		{
			glfwSetWindowSizeLimits(window, minwidth, minheight, GLFW_DONT_CARE, GLFW_DONT_CARE);
			checkError();
		}
		void clearSizeLimits() { setSizeLimits(GLFW_DONT_CARE, GLFW_DONT_CARE, GLFW_DONT_CARE, GLFW_DONT_CARE); checkError(); }

		void setAspectRatio(int numer, int denom) { glfwSetWindowAspectRatio(window, numer, denom); checkError(); }
		void clearAspectRatio() { setAspectRatio(GLFW_DONT_CARE, GLFW_DONT_CARE); }

		auto getFramebufferSize() const
		{
			Size fs;
			glfwGetFramebufferSize(window, &fs.width, &fs.height);
			checkError();
			return fs;
		}

		auto getFrameSize() const
		{
			WindowFrameSize wfs;
			glfwGetWindowFrameSize(window, &wfs.left, &wfs.top, &wfs.right, &wfs.bottom);
			checkError();
			return wfs;
		}

		auto getContentScale() const
		{
			FloatCoord coord;
			glfwGetWindowContentScale(window, &coord.x, &coord.y);
			checkError();
			return coord;
		}

		auto getOpacity() const { return checkError(glfwGetWindowOpacity(window)); }
		void setOpacity(float opacity) { glfwSetWindowOpacity(window, opacity); checkError(); }

		void iconify() { glfwIconifyWindow(window); checkError(); }
		void restore() { glfwRestoreWindow(window); checkError(); }
		void maximize() { glfwMaximizeWindow(window); checkError(); }
		void show() { glfwShowWindow(window); checkError(); }
		void hide() { glfwHideWindow(window); checkError(); }
		void focus() { glfwFocusWindow(window); checkError(); }
		void requestAttention() { glfwRequestWindowAttention(window); checkError(); }

		Monitor getMonitor() const { return Monitor(checkError(glfwGetWindowMonitor(window))); }

		void setMonitor(Monitor monitor, IntCoord position, Size size, int refreshRate)
		{
			setMonitor(monitor, position.x, position.y, size.width, size.height, refreshRate);
		}
		void setMonitor(Monitor monitor, IntRectangle rect, int refreshRate)
		{
			setMonitor(monitor, rect.x, rect.y, rect.width, rect.height, refreshRate);
		}
		void setMonitor(Monitor monitor, int x, int y, int width, int height, int refreshRate)
		{ 
			glfwSetWindowMonitor(window, monitor.monitor, x, y, width, height, refreshRate);
			checkError();
		}

		void setBorderlessFullscreen(Monitor monitor = Monitor::getPrimaryMonitor())
		{
			const auto& videoMode = monitor.getVideoMode();
			glfwSetWindowMonitor(window, monitor.monitor, 0, 0, videoMode.width, videoMode.height, videoMode.refreshRate);
		}

		// Window attributes
		bool isFocused() const { return checkError(glfwGetWindowAttrib(window, GLFW_FOCUSED)); }
		bool isIconified() const { return checkError(glfwGetWindowAttrib(window, GLFW_ICONIFIED)); }
		bool isMaximized() const { return checkError(glfwGetWindowAttrib(window, GLFW_MAXIMIZED)); }
		bool isBeingHovered() const { return checkError(glfwGetWindowAttrib(window, GLFW_HOVERED)); }
		bool isVisible() const { return checkError(glfwGetWindowAttrib(window, GLFW_VISIBLE)); }
		bool isResizable() const { return checkError(glfwGetWindowAttrib(window, GLFW_RESIZABLE)); }
		bool isDecorated() const { return checkError(glfwGetWindowAttrib(window, GLFW_DECORATED)); }
		bool isAutoIconify() const { return checkError(glfwGetWindowAttrib(window, GLFW_AUTO_ICONIFY)); }
		bool isFloating() const { return checkError(glfwGetWindowAttrib(window, GLFW_FLOATING)); }
		bool isTransparentFramebuffer() const { return checkError(glfwGetWindowAttrib(window, GLFW_TRANSPARENT_FRAMEBUFFER)); }
		bool isFocusOnShow() const { return checkError(glfwGetWindowAttrib(window, GLFW_FOCUS_ON_SHOW)); }

		// Setting attributes
		void setResizable(bool resizable) { glfwSetWindowAttrib(window, GLFW_RESIZABLE, resizable); checkError(); }
		void setDecorated(bool decorated) { glfwSetWindowAttrib(window, GLFW_DECORATED, decorated); checkError(); }
		void setAutoIconify(bool autoIconify) { glfwSetWindowAttrib(window, GLFW_AUTO_ICONIFY, autoIconify); checkError(); }
		void setFloating(bool floating) { glfwSetWindowAttrib(window, GLFW_FLOATING, floating); checkError(); }
		void setTransparentFramebuffer(bool transparentFramebuffer) 
		{ 
			glfwSetWindowAttrib(window, GLFW_TRANSPARENT_FRAMEBUFFER, transparentFramebuffer);
			checkError();
		}
		void setFocusOnShow(bool focusOnShow) { glfwSetWindowAttrib(window, GLFW_FOCUS_ON_SHOW, focusOnShow); checkError(); }

		// Context
		ClientApi getClientApi() const { return static_cast<ClientApi>(checkError(glfwGetWindowAttrib(window, GLFW_CLIENT_API))); }
		ContextApi getContextCreationApi() const
		{
			return static_cast<ContextApi>(checkError(glfwGetWindowAttrib(window, GLFW_CONTEXT_CREATION_API)));
		}

		OpenGLVersion getContextVersion()
		{
			return
			{
				glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR),
				glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR),
				checkError(glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION))
			};
		}

		bool isContextForwardCompatible() const { return checkError(glfwGetWindowAttrib(window, GLFW_OPENGL_FORWARD_COMPAT)); }
		bool isDebugContext() const { return checkError(glfwGetWindowAttrib(window, GLFW_OPENGL_DEBUG_CONTEXT)); }

		Profile getOpenGLProfile() const { return static_cast<Profile>(checkError(glfwGetWindowAttrib(window, GLFW_OPENGL_PROFILE))); }
		Robustness getContextRobustness() const
		{ 
			return static_cast<Robustness>(checkError(glfwGetWindowAttrib(window, GLFW_CONTEXT_ROBUSTNESS)));
		}

		// Swap buffers
		void swapBuffers() { glfwSwapBuffers(window); checkError(); }
		static void setSwapInterval(int interval) { glfwSwapInterval(interval); checkError(); }

		// Context
		void makeCurrent()
		{
			glfwMakeContextCurrent(window);
			checkError();
			currentContext = this;
		}
		static void clearCurrent()
		{ 
			glfwMakeContextCurrent(nullptr); 
			checkError();
			currentContext = nullptr;
		}
		static Window& getCurrentContext() { return *currentContext; }

		// Input
		CursorMode getCursorMode() const { return static_cast<CursorMode>(checkError(glfwGetInputMode(window, GLFW_CURSOR))); }
		bool getStickyKeysEnabled() const { return checkError(glfwGetInputMode(window, GLFW_STICKY_KEYS)); }
		bool getStickyMouseButtonsEnabled() const { return checkError(glfwGetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS)); }
		bool getLockKeyModsEnabled() const { return checkError(glfwGetInputMode(window, GLFW_LOCK_KEY_MODS)); }
		bool getRawMouseMotionEnabled() const { return checkError(glfwGetInputMode(window, GLFW_RAW_MOUSE_MOTION)); }

		void setCursorMode(CursorMode mode) { glfwSetInputMode(window, GLFW_CURSOR, static_cast<int>(mode)); checkError(); }
		void setStickyKeysEnabled(bool enable = true) { glfwSetInputMode(window, GLFW_STICKY_KEYS, enable); checkError(); }
		void setStickyMouseButtonsEnabled(bool enable = true) { glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, enable); checkError(); }
		void setLockKeyModsEnabled(bool enable = true) { glfwSetInputMode(window, GLFW_LOCK_KEY_MODS, enable); checkError(); }
		void setRawMouseMotionEnabled(bool enable = true) { glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, enable); checkError(); }

		bool getKey(int key) const { return checkError(glfwGetKey(window, key)); }
		bool getMouseButton(int button) const { return checkError(glfwGetMouseButton(window, button)); }

		auto getCursorPos() const
		{
			DoubleCoord position;
			glfwGetCursorPos(window, &position.x, &position.y);
			checkError();
			return position;
		}

		void setCursorPos(double xpos, double ypos) { glfwSetCursorPos(window, xpos, ypos); checkError(); }
		void setCursorPos(DoubleCoord position) { setCursorPos(position.x, position.y); }

		void setCursor(const Cursor& cursor) { glfwSetCursor(window, cursor.cursor); checkError(); }

	private:
		struct WindowCallbacks final
		{
			Window& window;

			std::function<void(Window&, int, int)> position;
			std::function<void(Window&, int, int)> size;
			std::function<void(Window&)> close;
			std::function<void(Window&)> refresh;
			std::function<void(Window&, bool)> focus;
			std::function<void(Window&, bool)> iconify;
			std::function<void(Window&, bool)> maximize;
			std::function<void(Window&, int, int)> bufferSize;
			std::function<void(Window&, float, float)> contentScale;

			std::function<void(Window&, glfw::MouseButton, glfw::Action, glfw::ModKeyFlags)> mouseButton;
			std::function<void(Window&, double, double)> cursorPos;
			std::function<void(Window&, bool)> cursorEnter;
			std::function<void(Window&, double, double)> scroll;
			std::function<void(Window&, int, int, glfw::Action, glfw::ModKeyFlags)> key;
			std::function<void(Window&, unsigned int)> charAction;
			std::function<void(Window&, util::array_view<const char*>)> drop;

			WindowCallbacks(Window& window) : window(window) {}
		};

		static WindowCallbacks& getCallbacksForWindow(GLFWwindow* window)
		{
			return *static_cast<WindowCallbacks*>(checkError(glfwGetWindowUserPointer(window)));
		}

		WindowCallbacks& getWindowCallbacks() const
		{
			return getCallbacksForWindow(window);
		}

		void createWindowCallbacksStructure()
		{
			if (!checkError(glfwGetWindowUserPointer(window)))
			{
				glfwSetWindowUserPointer(window, new WindowCallbacks(*this));
				checkError();
			}
		}

		void destroyWindowCallbacksStructure()
		{
			delete &getWindowCallbacks();
		}

	public:
		// Window event processing
		template <std::invocable<Window&, int, int> C>
		void setPosCallback(C callback)
		{
			createWindowCallbacksStructure();
			getWindowCallbacks().position = callback;
			glfwSetWindowPosCallback(window, [](GLFWwindow* window, int xpos, int ypos)
				{ auto& cbs = getCallbacksForWindow(window); cbs.position(cbs.window, xpos, ypos); });
			checkError();
		}

		template <std::invocable<Window&, int, int> C>
		void setSizeCallback(C callback)
		{
			createWindowCallbacksStructure();
			getWindowCallbacks().size = callback;
			glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height)
				{ auto& cbs = getCallbacksForWindow(window); cbs.size(cbs.window, width, height); });
			checkError();
		}

		template <std::invocable<Window&> C>
		void setCloseCallback(C callback)
		{
			createWindowCallbacksStructure();
			getWindowCallbacks().close = callback;
			glfwSetWindowCloseCallback(window, [](GLFWwindow* window)
				{ auto& cbs = getCallbacksForWindow(window); cbs.close(cbs.window); });
			checkError();
		}

		template <std::invocable<Window&> C>
		void setRefreshCallback(C callback)
		{
			createWindowCallbacksStructure();
			getWindowCallbacks().refresh = callback;
			glfwSetWindowRefreshCallback(window, [](GLFWwindow* window)
				{ auto& cbs = getCallbacksForWindow(window); cbs.refresh(cbs.window); });
			checkError();
		}

		template <std::invocable<Window&, bool> C>
		void setFocusCallback(C callback)
		{
			createWindowCallbacksStructure();
			getWindowCallbacks().focus = callback;
			glfwSetWindowFocusCallback(window, [](GLFWwindow* window, int focused)
				{ auto& cbs = getCallbacksForWindow(window); cbs.focus(cbs.window, focused); });
			checkError();
		}

		template <std::invocable<Window&, bool> C>
		void setIconifyCallback(C callback)
		{
			createWindowCallbacksStructure();
			getWindowCallbacks().iconify = callback;
			glfwSetWindowIconifyCallback(window, [](GLFWwindow* window, int iconified)
				{ auto& cbs = getCallbacksForWindow(window); cbs.iconify(cbs.window, iconified); });
			checkError();
		}

		template <std::invocable<Window&, bool> C>
		void setMaximizeCallback(C callback)
		{
			createWindowCallbacksStructure();
			getWindowCallbacks().maximize = callback;
			glfwSetWindowMaximizeCallback(window, [](GLFWwindow* window, int maximized)
				{ auto& cbs = getCallbacksForWindow(window); cbs.maximize(cbs.window, maximized); });
			checkError();
		}

		template <std::invocable<Window&, int, int> C>
		void setFramebufferSizeCallback(C callback)
		{
			createWindowCallbacksStructure();
			getWindowCallbacks().bufferSize = callback;
			glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
				{ auto& cbs = getCallbacksForWindow(window); cbs.bufferSize(cbs.window, width, height); });
			checkError();
		}

		template <std::invocable<Window&, float, float> C>
		void setContentScaleCallback(C callback)
		{
			createWindowCallbacksStructure();
			getWindowCallbacks().contentScale = callback;
			glfwSetWindowContentScaleCallback(window, [](GLFWwindow* window, float xscale, float yscale)
				{ auto& cbs = getCallbacksForWindow(window); cbs.contentScale(cbs.window, xscale, yscale); });
			checkError();
		}

		// Input event processing
		template <std::invocable<Window&, glfw::MouseButton, glfw::Action, glfw::ModKeyFlags> C>
		void setMouseButtonCallback(C callback)
		{
			createWindowCallbacksStructure();
			getWindowCallbacks().mouseButton = callback;
			glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods)
				{ auto& cbs = getCallbacksForWindow(window); cbs.mouseButton(cbs.window, 
					static_cast<glfw::MouseButton>(button), static_cast<glfw::Action>(action), glfw::ModKeyFlags(mods)); });
			checkError();
		}

		template <std::invocable<Window&, double, double> C>
		void setCursorPosCallback(C callback)
		{
			createWindowCallbacksStructure();
			getWindowCallbacks().cursorPos = callback;
			glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos)
				{ auto& cbs = getCallbacksForWindow(window); cbs.cursorPos(cbs.window, xpos, ypos); });
			checkError();
		}

		template <std::invocable<Window&, bool> C>
		void setCursorEnterCallback(C callback)
		{
			createWindowCallbacksStructure();
			getWindowCallbacks().cursorEnter = callback;
			glfwSetCursorEnterCallback(window, [](GLFWwindow* window, int entered)
				{ auto& cbs = getCallbacksForWindow(window); cbs.cursorEnter(cbs.window, entered); });
			checkError();
		}

		template <std::invocable<Window&, double, double> C>
		void setScrollCallback(C callback)
		{
			createWindowCallbacksStructure();
			getWindowCallbacks().scroll = callback;
			glfwSetScrollCallback(window, [](GLFWwindow* window, double xscroll, double yscroll)
				{ auto& cbs = getCallbacksForWindow(window); cbs.scroll(cbs.window, xscroll, yscroll); });
			checkError();
		}

		template <std::invocable<Window&, int, int, glfw::Action, glfw::ModKeyFlags> C>
		void setKeyCallback(C callback)
		{
			createWindowCallbacksStructure();
			getWindowCallbacks().key = callback;
			glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
				{ auto& cbs = getCallbacksForWindow(window); cbs.key(cbs.window, key, scancode, 
					static_cast<glfw::Action>(action), glfw::ModKeyFlags(mods)); });
			checkError();
		}

		template <std::invocable<Window&, unsigned int> C>
		void setCharCallback(C callback)
		{
			createWindowCallbacksStructure();
			getWindowCallbacks().charAction = callback;
			glfwSetKeyCallback(window, [](GLFWwindow* window, unsigned int codepoint)
				{ auto& cbs = getCallbacksForWindow(window); cbs.charAction(cbs.window, codepoint); });
			checkError();
		}

		template <std::invocable<Window&, util::array_view<const char*>> C>
		void setDropCallback(C callback)
		{
			createWindowCallbacksStructure();
			getWindowCallbacks().drop = callback;
			glfwSetKeyCallback(window, [](GLFWwindow* window, int pathCount, const char* paths[])
				{ auto& cbs = getCallbacksForWindow(window); cbs.drop(cbs.window, util::array_view(paths, pathCount)); });
			checkError();

		}

		void setViewport() const
		{
			auto size = getFramebufferSize();
			glViewport(0, 0, size.width, size.height); gl::checkError();
		}

		// Destructor
		~Window()
		{ 
			if (window)
			{
				destroyWindowCallbacksStructure();
				glfwDestroyWindow(window);
			}
		}

		// Operator
		operator GLFWwindow* () const { return window; }
	};
}