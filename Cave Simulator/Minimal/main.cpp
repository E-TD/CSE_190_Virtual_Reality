/************************************************************************************

Authors     :   Bradley Austin Davis <bdavis@saintandreas.org>
Copyright   :   Copyright Brad Davis. All Rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/


#include <iostream>
#include <memory>
#include <exception>
#include <algorithm>

#include <Windows.h>

#define __STDC_FORMAT_MACROS 1

#define FAIL(X) throw std::runtime_error(X)

// HERES MY INCLUDES
#include "Window.h"



///////////////////////////////////////////////////////////////////////////////
//
// GLM is a C++ math library meant to mirror the syntax of GLSL 
//


// Import the most commonly used types into the default namespace
using glm::ivec3;
using glm::ivec2;
using glm::uvec2;
using glm::mat3;
using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::quat;

///////////////////////////////////////////////////////////////////////////////
//
// GLEW gives cross platform access to OpenGL 3.x+ functionality.  
//

#include <GL/glew.h>

bool checkFramebufferStatus(GLenum target = GL_FRAMEBUFFER) {
	GLuint status = glCheckFramebufferStatus(target);
	switch (status) {
	case GL_FRAMEBUFFER_COMPLETE:
		return true;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		std::cerr << "framebuffer incomplete attachment" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		std::cerr << "framebuffer missing attachment" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		std::cerr << "framebuffer incomplete draw buffer" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		std::cerr << "framebuffer incomplete read buffer" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		std::cerr << "framebuffer incomplete multisample" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
		std::cerr << "framebuffer incomplete layer targets" << std::endl;
		break;

	case GL_FRAMEBUFFER_UNSUPPORTED:
		std::cerr << "framebuffer unsupported internal format or image" << std::endl;
		break;

	default:
		std::cerr << "other framebuffer error" << std::endl;
		break;
	}

	return false;
}

bool checkGlError() {
	GLenum error = glGetError();
	if (!error) {
		return false;
	}
	else {
		switch (error) {
		case GL_INVALID_ENUM:
			std::cerr << ": An unacceptable value is specified for an enumerated argument.The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_INVALID_VALUE:
			std::cerr << ": A numeric argument is out of range.The offending command is ignored and has no other side effect than to set the error flag";
			break;
		case GL_INVALID_OPERATION:
			std::cerr << ": The specified operation is not allowed in the current state.The offending command is ignored and has no other side effect than to set the error flag..";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			std::cerr << ": The framebuffer object is not complete.The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_OUT_OF_MEMORY:
			std::cerr << ": There is not enough memory left to execute the command.The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
			break;
		case GL_STACK_UNDERFLOW:
			std::cerr << ": An attempt has been made to perform an operation that would cause an internal stack to underflow.";
			break;
		case GL_STACK_OVERFLOW:
			std::cerr << ": An attempt has been made to perform an operation that would cause an internal stack to overflow.";
			break;
		}
		return true;
	}
}

void glDebugCallbackHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, GLvoid* data) {
	OutputDebugStringA(msg);
	std::cout << "debug call: " << msg << std::endl;
}

//////////////////////////////////////////////////////////////////////
//
// GLFW provides cross platform window creation
//


namespace glfw {
	inline GLFWwindow * createWindow(const uvec2 & size, const ivec2 & position = ivec2(INT_MIN)) {
		GLFWwindow * window = glfwCreateWindow(size.x, size.y, "glfw", nullptr, nullptr);
		if (!window) {
			FAIL("Unable to create rendering window");
		}
		if ((position.x > INT_MIN) && (position.y > INT_MIN)) {
			glfwSetWindowPos(window, position.x, position.y);
		}
		return window;
	}
}

// A class to encapsulate using GLFW to handle input and render a scene
class GlfwApp {

protected:
	uvec2 windowSize;
	ivec2 windowPosition;
	GLFWwindow * window{ nullptr };
	unsigned int frame{ 0 };

public:
	GlfwApp() {
		// Initialize the GLFW system for creating and positioning windows
		if (!glfwInit()) {
			FAIL("Failed to initialize GLFW");
		}
		glfwSetErrorCallback(ErrorCallback);
	}

	virtual ~GlfwApp() {
		if (nullptr != window) {
			glfwDestroyWindow(window);
		}
		glfwTerminate();
	}

	virtual int run() {
		preCreate();

		window = createRenderingTarget(windowSize, windowPosition);

		if (!window) {
			std::cout << "Unable to create OpenGL window" << std::endl;
			return -1;
		}

		postCreate();

		initGl();

		while (!glfwWindowShouldClose(window)) {
			++frame;
			glfwPollEvents();
			update();
			draw();
			finishFrame();
		}

		shutdownGl();

		return 0;
	}


protected:
	virtual GLFWwindow * createRenderingTarget(uvec2 & size, ivec2 & pos) = 0;

	virtual void draw() = 0;

	void preCreate() {
		glfwWindowHint(GLFW_DEPTH_BITS, 16);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	}


	void postCreate() {
		glfwSetWindowUserPointer(window, this);
		glfwSetKeyCallback(window, KeyCallback);
		glfwSetMouseButtonCallback(window, MouseButtonCallback);
		glfwMakeContextCurrent(window);

		// Initialize the OpenGL bindings
		// For some reason we have to set this experminetal flag to properly
		// init GLEW if we use a core context.
		glewExperimental = GL_TRUE;
		if (0 != glewInit()) {
			FAIL("Failed to initialize GLEW");
		}
		glGetError();

		if (GLEW_KHR_debug) {
			GLint v;
			glGetIntegerv(GL_CONTEXT_FLAGS, &v);
			if (v & GL_CONTEXT_FLAG_DEBUG_BIT) {
				//glDebugMessageCallback(glDebugCallbackHandler, this);
			}
		}
	}

	virtual void initGl() {
	}

	virtual void shutdownGl() {
	}

	virtual void finishFrame() {
		glfwSwapBuffers(window);
	}

	virtual void destroyWindow() {
		glfwSetKeyCallback(window, nullptr);
		glfwSetMouseButtonCallback(window, nullptr);
		glfwDestroyWindow(window);
	}

	virtual void onKey(int key, int scancode, int action, int mods) {
		if (GLFW_PRESS != action) {
			return;
		}

		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, 1);
			return;
		}
	}

	virtual void update() {}

	virtual void onMouseButton(int button, int action, int mods) {}

protected:
	virtual void viewport(const ivec2 & pos, const uvec2 & size) {
		glViewport(pos.x, pos.y, size.x, size.y);
	}

private:

	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		GlfwApp * instance = (GlfwApp *)glfwGetWindowUserPointer(window);
		instance->onKey(key, scancode, action, mods);
	}

	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		GlfwApp * instance = (GlfwApp *)glfwGetWindowUserPointer(window);
		instance->onMouseButton(button, action, mods);
	}

	static void ErrorCallback(int error, const char* description) {
		FAIL(description);
	}
};

//////////////////////////////////////////////////////////////////////
//
// The Oculus VR C API provides access to information about the HMD
//


class RiftManagerApp {
protected:
	ovrSession _session;
	ovrHmdDesc _hmdDesc;
	ovrGraphicsLuid _luid;

public:
	RiftManagerApp() {
		if (!OVR_SUCCESS(ovr_Create(&_session, &_luid))) {
			FAIL("Unable to create HMD session");
		}

		_hmdDesc = ovr_GetHmdDesc(_session);
	}

	~RiftManagerApp() {
		ovr_Destroy(_session);
		_session = nullptr;
	}
};

class RiftApp : public GlfwApp, public RiftManagerApp {
public:
	ovrLayerEyeFov _sceneLayer;
	ovrViewScaleDesc _viewScaleDesc;

	bool handView = false;
	bool debugMode = false;
	bool freezeMode = false;

	ovrVector3f handPositions[2];
	ovrQuatf handOrientations[2];


	int aMode = 0;
	int bMode = 0;

	bool aPress = false;
	bool bPress = false;

	// Holds Position and Orientation
	ovrVector3f headPosition;

	ovrVector3f eyePositions[2];
	ovrQuatf eyeOrientations[2];

	ovrVector3f eyeOffsets[2];
	ovrVector3f originalEyeOffsets[2];

	bool rightGoingRight = false;
	bool rightGoingLeft = false;

	bool leftGoingRight = false;
	bool leftGoingLeft = false;

	Skybox* skyboxleft;
	Skybox* skyboxright;

	Remote* remote;

private:
	GLuint _fbo{ 0 };
	GLuint _depthBuffer{ 0 };
	ovrTextureSwapChain _eyeTexture;

	GLuint _mirrorFbo{ 0 };
	ovrMirrorTexture _mirrorTexture;

	ovrEyeRenderDesc _eyeRenderDescs[2];

	mat4 _eyeProjections[2];


	uvec2 _renderTargetSize;
	uvec2 _mirrorSize;

	// PROJ 3 frame buffer objects

	GLuint left_fbos[3];
	GLuint right_fbos[3];
	GLuint left_texes[3];
	GLuint right_texes[3];


	//GLuint scene_fbos[3];
	//GLuint scene_texes[3];
	GLuint depth_buff;

public:

	RiftApp() {
		using namespace ovr;
		_viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;

		memset(&_sceneLayer, 0, sizeof(ovrLayerEyeFov));
		_sceneLayer.Header.Type = ovrLayerType_EyeFov;
		_sceneLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

		ovr::for_each_eye([&](ovrEyeType eye) {
			ovrEyeRenderDesc& erd = _eyeRenderDescs[eye] = ovr_GetRenderDesc(_session, eye, _hmdDesc.DefaultEyeFov[eye]);
			ovrMatrix4f ovrPerspectiveProjection =
				ovrMatrix4f_Projection(erd.Fov, 0.01f, 1000.0f, ovrProjection_ClipRangeOpenGL);
			_eyeProjections[eye] = ovr::toGlm(ovrPerspectiveProjection);
			_viewScaleDesc.HmdToEyeOffset[eye] = erd.HmdToEyeOffset;

			// IOD EYE OFFSETS
			eyeOffsets[eye] = erd.HmdToEyeOffset;
			originalEyeOffsets[eye] = erd.HmdToEyeOffset;

			ovrFovPort & fov = _sceneLayer.Fov[eye] = _eyeRenderDescs[eye].Fov;
			auto eyeSize = ovr_GetFovTextureSize(_session, eye, fov, 1.0f);
			_sceneLayer.Viewport[eye].Size = eyeSize;
			_sceneLayer.Viewport[eye].Pos = { (int)_renderTargetSize.x, 0 };

			_renderTargetSize.y = std::max(_renderTargetSize.y, (uint32_t)eyeSize.h);
			_renderTargetSize.x += eyeSize.w;
		});
		// Make the on screen window 1/4 the resolution of the render target
		_mirrorSize = _renderTargetSize;
		_mirrorSize /= 4;
	}

protected:
	GLFWwindow * createRenderingTarget(uvec2 & outSize, ivec2 & outPosition) override {
		return glfw::createWindow(_mirrorSize);
	}

	void initGl() override {
		GlfwApp::initGl();

		// Disable the v-sync for buffer swap
		glfwSwapInterval(0);

		ovrTextureSwapChainDesc desc = {};
		desc.Type = ovrTexture_2D;
		desc.ArraySize = 1;
		desc.Width = _renderTargetSize.x;
		desc.Height = _renderTargetSize.y;
		desc.MipLevels = 1;
		desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.SampleCount = 1;
		desc.StaticImage = ovrFalse;
		ovrResult result = ovr_CreateTextureSwapChainGL(_session, &desc, &_eyeTexture);
		_sceneLayer.ColorTexture[0] = _eyeTexture;
		if (!OVR_SUCCESS(result)) {
			FAIL("Failed to create swap textures");
		}

		int length = 0;
		result = ovr_GetTextureSwapChainLength(_session, _eyeTexture, &length);
		if (!OVR_SUCCESS(result) || !length) {
			FAIL("Unable to count swap chain textures");
		}
		for (int i = 0; i < length; ++i) {
			GLuint chainTexId;
			ovr_GetTextureSwapChainBufferGL(_session, _eyeTexture, i, &chainTexId);
			glBindTexture(GL_TEXTURE_2D, chainTexId);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			// Generate mipmaps, by the way.
			glGenerateMipmap(GL_TEXTURE_2D);

			
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		// Set up the framebuffer object
		glGenFramebuffers(1, &_fbo);
		glGenRenderbuffers(1, &_depthBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
		glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _renderTargetSize.x, _renderTargetSize.y);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		for (int eye = 0; eye < 2; eye++) {


			for (int i = 0; i < 3; i++) {
				// START PROJ 3 frame buffer generation
				GLuint scene_fbo;
				glGenFramebuffers(1, &scene_fbo);

				if (eye == 0) left_fbos[i] = scene_fbo;
				else right_fbos[i] = scene_fbo;

				glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo);

				//create texture object that frame buffer will be written to
				GLuint scene_tex;
				glGenTextures(1, &scene_tex);

				if (eye == 0) left_texes[i] = scene_tex;
				else right_texes[i] = scene_tex;

				glBindTexture(GL_TEXTURE_2D, scene_tex);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, desc.Width / 2, desc.Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
				// TODO insert width and height in the above function call
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glBindTexture(GL_TEXTURE_2D, 0);
				//bind texture to scene frame buffer
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scene_tex, 0);

				//create render buffer object for depth attachment
				glGenRenderbuffers(1, &depth_buff);
				glBindRenderbuffer(GL_RENDERBUFFER, depth_buff);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, desc.Width / 2, desc.Height);
				// TODO insert width and height in the above function call
				glBindRenderbuffer(GL_RENDERBUFFER, 0);
				// bind render buffer to scene frame buffer
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buff);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);

				// END PROJ 3 FRAME BUFFER GENERATION
			}
		}
		
		

		ovrMirrorTextureDesc mirrorDesc;
		memset(&mirrorDesc, 0, sizeof(mirrorDesc));
		mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		mirrorDesc.Width = _mirrorSize.x;
		mirrorDesc.Height = _mirrorSize.y;
		if (!OVR_SUCCESS(ovr_CreateMirrorTextureGL(_session, &mirrorDesc, &_mirrorTexture))) {
			FAIL("Could not create mirror texture");
		}
		glGenFramebuffers(1, &_mirrorFbo);

		skyboxleft = new Skybox(true);
		skyboxright = new Skybox(false);

		remote = new Remote();

	}

	void onKey(int key, int scancode, int action, int mods) override {
		if (GLFW_PRESS == action) switch (key) {
		case GLFW_KEY_R:
			ovr_RecenterTrackingOrigin(_session);
			return;
		}

		GlfwApp::onKey(key, scancode, action, mods);
	}

	// after generating frame buffer textures, render the cave to the final default frame buffer
	void renderCave(const glm::mat4 & projection, const glm::mat4 & headPose, GLuint * texes) {
		glUseProgram(Window::shaderProgram);
		GLuint uProjection = glGetUniformLocation(Window::shaderProgram, "projection");
		GLuint uModelView = glGetUniformLocation(Window::shaderProgram, "view");

		glUniformMatrix4fv(uProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uModelView, 1, GL_FALSE, glm::value_ptr(glm::inverse(headPose)));
		Window::cave->draw(Window::shaderProgram, texes);
	}

	void oneFrameBuffer(int mode, ovrPosef * eyePoses, ovrPosef * handPoses) {
		glm::vec3 pta, ptb, ptc;
		switch (mode) {
		case 0:
			pta = glm::vec3(-1.697f, -1.2f, 0.0f);
			ptb = glm::vec3(0.0f, -1.2f, -1.697f);
			ptc = glm::vec3(-1.697f, 1.2f, 0.0f);

			break;
		case 1:
			pta = glm::vec3(0.0f, -1.2f, -1.697f);
			ptb = glm::vec3(1.697f, -1.2f, 0.0f);
			ptc = glm::vec3(0.0f, 1.2f, -1.697f);
			break;
		case 2:
			pta = glm::vec3(0.0f, -1.2f, 1.697f);
			ptb = glm::vec3(1.697f, -1.2f, 0.0f);
			ptc = glm::vec3(-1.697f, -1.2f, 0.0f);
			break;
		}


		ovr::for_each_eye([&](ovrEyeType eye) {
			//draw to the frame buffer
			if(eye == ovrEye_Left) glBindFramebuffer(GL_FRAMEBUFFER, left_fbos[mode]);
			else  glBindFramebuffer(GL_FRAMEBUFFER, right_fbos[mode]);
			// Clear all attached buffers        
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // We're not using stencil buffer so why bother with clearing?

			glEnable(GL_DEPTH_TEST);

			// THESE LINESSSSSSS
			const auto& vp = _sceneLayer.Viewport[ovrEye_Left];
			glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
			_sceneLayer.RenderPose[eye] = eyePoses[eye];

			// STEREO
			if (eye == ovrEye_Left) Window::skybox = skyboxleft;
			else Window::skybox = skyboxright;

			ovrEyeRenderDesc& erd = _eyeRenderDescs[eye] = ovr_GetRenderDesc(_session, eye, _hmdDesc.DefaultEyeFov[eye]);
			glm::mat4 proj = projection(pta, ptb, ptc, ovr::toGlm(eyePoses[eye].Position), 0.01f, 1000.0f);

			eyePositions[eye] = eyePoses[eye].Position;
			eyeOrientations[eye] = eyePoses[eye].Orientation;

			/*if (handView) renderScene(proj, ovr::toGlm(handPoses[ovrHand_Right]));
			else renderScene(proj, ovr::toGlm(eyePoses[eye]));*/

			if (handView) {
				if (eye == ovrEye_Left) {
					handPoses[ovrHand_Right].Position.x = handPoses[ovrHand_Right].Position.x - 0.0325f;
				}
				else {
					handPoses[ovrHand_Right].Position.x = handPoses[ovrHand_Right].Position.x + 0.0325f;
				}
				renderScene(proj, ovr::toGlm(handPoses[ovrHand_Right]));
			}
			else {
				renderScene(proj, ovr::toGlm(eyePoses[eye]));
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		});

		

	}

	void draw() final override {
		ovrPosef eyePoses[2];

		ovr_GetEyePoses(_session, frame, true, _viewScaleDesc.HmdToEyeOffset, eyePoses, &_sceneLayer.SensorSampleTime);

		double displayMidpointSeconds = ovr_GetPredictedDisplayTime(_session, frame);
		ovrTrackingState trackState = ovr_GetTrackingState(_session, displayMidpointSeconds, ovrTrue);
		ovrPosef handPoses[2];
		ovrPosef headState;

		// Grab head and hand poses
		headState = trackState.HeadPose.ThePose;
		ovrVector3f headPos = headState.Position;

		handPoses[ovrHand_Right] = trackState.HandPoses[ovrHand_Right].ThePose;
		ovrQuatf rightOrient = handPoses[ovrHand_Right].Orientation;
		ovrVector3f rightPos = handPoses[ovrHand_Right].Position;

		/*eyePositions[ovrEye_Left] = eyePoses[ovrEye_Left].Position;
		eyeOrientations[ovrEye_Left] = eyePoses[ovrEye_Left].Orientation;

		eyePositions[ovrEye_Right] = eyePoses[ovrEye_Right].Position;
		eyeOrientations[ovrEye_Right] = eyePoses[ovrEye_Right].Orientation;*/

		// VIRTUAL CAVE SPACE MOTHERFUCKER
		if (!freezeMode) {
			oneFrameBuffer(0, eyePoses, handPoses);
			oneFrameBuffer(1, eyePoses, handPoses);
			oneFrameBuffer(2, eyePoses, handPoses);
		}

		// REAL RIFT SPACE MOTHERFUCKER
		int curIndex;
		ovr_GetTextureSwapChainCurrentIndex(_session, _eyeTexture, &curIndex);
		GLuint curTexId;
		ovr_GetTextureSwapChainBufferGL(_session, _eyeTexture, curIndex, &curTexId);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		std::vector<glm::vec3> linesToDraw;

		ovr::for_each_eye([&](ovrEyeType eye) {


			const auto& vp = _sceneLayer.Viewport[eye];
			glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
			_sceneLayer.RenderPose[eye] = eyePoses[eye];

			eyePositions[eye] = eyePoses[eye].Position;
			eyeOrientations[eye] = eyePoses[eye].Orientation;

			if(eye == ovrEye_Left) renderCave(_eyeProjections[eye], ovr::toGlm(eyePoses[eye]), left_texes);
			else  renderCave(_eyeProjections[eye], ovr::toGlm(eyePoses[eye]), right_texes);

			//Draw lines (pyramids)


			glUseProgram(Window::lineShader);
			GLuint uProjection = glGetUniformLocation(Window::lineShader, "projection");
			GLuint uModelView = glGetUniformLocation(Window::lineShader, "view");

			glUniformMatrix4fv(uProjection, 1, GL_FALSE, glm::value_ptr(_eyeProjections[eye]));
			glUniformMatrix4fv(uModelView, 1, GL_FALSE, glm::value_ptr(glm::inverse(ovr::toGlm(eyePoses[eye]))));


			if (debugMode) {

				glm::vec3 pta_1 = glm::vec3(-1.697f, -1.2f, 0.0f);
				glm::vec3 ptb_1 = glm::vec3(0.0f, -1.2f, -1.697f);
				glm::vec3 ptc_1 = glm::vec3(-1.697f, 1.2f, 0.0f);
				glm::vec3 ptd_1 = glm::vec3(0.0f, 1.2f, -1.697f);

				glm::vec3 pta_2 = glm::vec3(0.0f, 1.2f, -1.697f);
				glm::vec3 ptb_2 = glm::vec3(0.0f, -1.2f, -1.697f);
				glm::vec3 ptc_2 = glm::vec3(1.697f, -1.2f, 0.0f);
				glm::vec3 ptd_2 = glm::vec3(1.697f, 1.2f, 0.0f);

				glm::vec3 pta_3 = glm::vec3(-1.697f, -1.2f, 0.0f);
				glm::vec3 ptb_3 = glm::vec3(0.0f, -1.2f, -1.697f);
				glm::vec3 ptc_3 = glm::vec3(1.697f, -1.2f, 0.0f);
				glm::vec3 ptd_3 = glm::vec3(0.0f, -1.2f, 1.697f);

				ovrEyeType eyeArray[] = {ovrEye_Left, ovrEye_Right};
				ovrEyeType eyeLine;
				for (int i = 0; i < 2; i++) {
					eyeLine = eyeArray[i];

					linesToDraw.clear();

					float dude = 0.0f;
					if (eyeLine == ovrEye_Left) {
						handPoses[ovrHand_Right].Position.x = handPoses[ovrHand_Right].Position.x - 0.0325f;
					}
					else {
						handPoses[ovrHand_Right].Position.x = handPoses[ovrHand_Right].Position.x + 0.0325f;
					}

					if (i == 0) {
						remote->colorVal = glm::vec3(0.0f, 1.0f, 0.0f); 
					}
					else { 
						remote->colorVal = glm::vec3(1.0f, 0.0f, 0.0f); 
					}

					if (handView) {
						linesToDraw.push_back(ovr::toGlm(handPoses[ovrHand_Right].Position));
						linesToDraw.push_back(pta_1);

						linesToDraw.push_back(ovr::toGlm(handPoses[ovrHand_Right].Position));
						linesToDraw.push_back(ptb_1);

						linesToDraw.push_back(ovr::toGlm(handPoses[ovrHand_Right].Position));
						linesToDraw.push_back(ptc_1);

						linesToDraw.push_back(ovr::toGlm(handPoses[ovrHand_Right].Position));
						linesToDraw.push_back(ptd_1);

						linesToDraw.push_back(ovr::toGlm(handPoses[ovrHand_Right].Position));
						linesToDraw.push_back(pta_2);

						linesToDraw.push_back(ovr::toGlm(handPoses[ovrHand_Right].Position));
						linesToDraw.push_back(ptb_2);

						linesToDraw.push_back(ovr::toGlm(handPoses[ovrHand_Right].Position));
						linesToDraw.push_back(ptc_2);

						linesToDraw.push_back(ovr::toGlm(handPoses[ovrHand_Right].Position));
						linesToDraw.push_back(ptd_2);

						linesToDraw.push_back(ovr::toGlm(handPoses[ovrHand_Right].Position));
						linesToDraw.push_back(pta_3);

						linesToDraw.push_back(ovr::toGlm(handPoses[ovrHand_Right].Position));
						linesToDraw.push_back(ptb_3);

						linesToDraw.push_back(ovr::toGlm(handPoses[ovrHand_Right].Position));
						linesToDraw.push_back(ptc_3);

						linesToDraw.push_back(ovr::toGlm(handPoses[ovrHand_Right].Position));
						linesToDraw.push_back(ptd_3);
					}
					else {
						linesToDraw.push_back(ovr::toGlm(eyePositions[eyeLine]));
						linesToDraw.push_back(pta_1);

						linesToDraw.push_back(ovr::toGlm(eyePositions[eyeLine]));
						linesToDraw.push_back(ptb_1);

						linesToDraw.push_back(ovr::toGlm(eyePositions[eyeLine]));
						linesToDraw.push_back(ptc_1);

						linesToDraw.push_back(ovr::toGlm(eyePositions[eyeLine]));
						linesToDraw.push_back(ptd_1);

						linesToDraw.push_back(ovr::toGlm(eyePositions[eyeLine]));
						linesToDraw.push_back(pta_2);

						linesToDraw.push_back(ovr::toGlm(eyePositions[eyeLine]));
						linesToDraw.push_back(ptb_2);

						linesToDraw.push_back(ovr::toGlm(eyePositions[eyeLine]));
						linesToDraw.push_back(ptc_2);

						linesToDraw.push_back(ovr::toGlm(eyePositions[eyeLine]));
						linesToDraw.push_back(ptd_2);

						linesToDraw.push_back(ovr::toGlm(eyePositions[eyeLine]));
						linesToDraw.push_back(pta_3);

						linesToDraw.push_back(ovr::toGlm(eyePositions[eyeLine]));
						linesToDraw.push_back(ptb_3);

						linesToDraw.push_back(ovr::toGlm(eyePositions[eyeLine]));
						linesToDraw.push_back(ptc_3);

						linesToDraw.push_back(ovr::toGlm(eyePositions[eyeLine]));
						linesToDraw.push_back(ptd_3);
					}

					for (int j = 0; j < linesToDraw.size(); j += 2) {
						remote->Draw(Window::lineShader, linesToDraw[j], linesToDraw[j + 1]);
					}

				}
					
			}



		});

		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		ovr_CommitTextureSwapChain(_session, _eyeTexture);
		ovrLayerHeader* headerList = &_sceneLayer.Header;
		ovr_SubmitFrame(_session, frame, &_viewScaleDesc, &headerList, 1);

		GLuint mirrorTextureId;
		ovr_GetMirrorTextureBufferGL(_session, _mirrorTexture, &mirrorTextureId);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _mirrorFbo);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTextureId, 0);
		glBlitFramebuffer(0, 0, _mirrorSize.x, _mirrorSize.y, 0, _mirrorSize.y, _mirrorSize.x, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	}

	void update() final override
	{
		ovrInputState inputState;

		if (OVR_SUCCESS(ovr_GetInputState(_session, ovrControllerType_Touch, &inputState)))
		{

			// RIGHT HAND TRIGGER: Switch Viewpoint Between Head/Eyes and Right Controller
			if (inputState.HandTrigger[ovrHand_Right] > 0.5f) {

				// Switch viewpoint to right controller
				handView = true;
			}
			else {
				// Switch viewpoint back to User's head
				handView = false;
			}

			// B BUTTON: Toggle Viewpoint Freezing
			if (inputState.Buttons & ovrButton_B) {
				bPress = true;
			}
			else if (bPress) {
				// Toggle Freeze Mode
				freezeMode = !freezeMode;
				bPress = false;
			}

			// A BUTTON: Toggle Debug Mode
			if (inputState.Buttons & ovrButton_A)
			{
				aPress = true;
			}
			else if (aPress) {
				// Toggle Debug Mode
				debugMode = !debugMode;
				aPress = false;
			}

			// RIGHT THUMBSTICK: Move Cube Back/Forth and Scale Bigger/Smaller
			if (inputState.Thumbstick[ovrHand_Right].x > 0.0f && inputState.Thumbstick[ovrHand_Right].y < 0.2f && inputState.Thumbstick[ovrHand_Right].y > -0.2f) {
				// Grow the cube
				Window::cube->scale(0.05f);
			}
			else if (inputState.Thumbstick[ovrHand_Right].x < 0.0f && inputState.Thumbstick[ovrHand_Right].y < 0.2f && inputState.Thumbstick[ovrHand_Right].y > -0.2f) {
				// Shrink the cube
				Window::cube->scale(-0.05f);
			}
			if (inputState.Thumbstick[ovrHand_Right].y > 0.0f && inputState.Thumbstick[ovrHand_Right].x < 0.3f && inputState.Thumbstick[ovrHand_Right].x > -0.3f) {
				// Translate cube FORWARD
				Window::cube->translateBACKFORTH(-0.005f);
			}
			else if (inputState.Thumbstick[ovrHand_Right].y < 0.0f && inputState.Thumbstick[ovrHand_Right].x < 0.3f && inputState.Thumbstick[ovrHand_Right].x > -0.3f) {
				// Translate cube BACKWARD
				Window::cube->translateBACKFORTH(0.005f);
			}

			// LEFT THUMBSTICK: Move Cube Left/Right and Up/Down
			if (inputState.Thumbstick[ovrHand_Left].x > 0.0f && inputState.Thumbstick[ovrHand_Left].y < 0.2f && inputState.Thumbstick[ovrHand_Left].y > -0.2f) {
				// Translate cube RIGHT
				Window::cube->translateLEFTRIGHT(0.005f);
			}
			else if (inputState.Thumbstick[ovrHand_Left].x < 0.0f && inputState.Thumbstick[ovrHand_Left].y < 0.2f && inputState.Thumbstick[ovrHand_Left].y > -0.2f) {
				// Translate cube LEFT
				Window::cube->translateLEFTRIGHT(-0.005f);
			}
			if (inputState.Thumbstick[ovrHand_Left].y > 0.0f && inputState.Thumbstick[ovrHand_Left].x < 0.3f && inputState.Thumbstick[ovrHand_Left].x > -0.3f) {
				// Translate cube FORWARD
				Window::cube->translateUPDOWN(0.005f);
			}
			else if (inputState.Thumbstick[ovrHand_Left].y < 0.0f && inputState.Thumbstick[ovrHand_Left].x < 0.3f && inputState.Thumbstick[ovrHand_Left].x > -0.3f) {
				// Translate cube BACKWARD
				Window::cube->translateUPDOWN(-0.005f);
			}
		}
	}

	glm::mat4 projection(glm::vec3 pa, glm::vec3 pb, glm::vec3 pc, glm::vec3 pe, float n, float f) {

		glm::mat4 frust;

		float va[3], vb[3], vc[3];
		float vr[3], vu[3];

		float l, r, b, t, d;

		// Compute an orthonormal basis for the screen.

		for (int i = 0; i < 3; i++) {
			vr[i] = pb[i] - pa[i];
			vu[i] = pc[i] - pa[i];
		}

		glm::vec3 vrVec = glm::vec3(vr[0], vr[1], vr[2]);
		vrVec = glm::normalize(vrVec);

		glm::vec3 vuVec = glm::vec3(vu[0], vu[1], vu[2]);
		vuVec = glm::normalize(vuVec);

		glm::vec3 vnVec = glm::cross(vrVec, vuVec);
		vnVec = glm::normalize(vnVec);

		// Compute the screen corner vectors.

		for (int i = 0; i < 3; i++) {
			va[i] = pa[i] - pe[i];
			vb[i] = pb[i] - pe[i];
			vc[i] = pc[i] - pe[i];
		}

		glm::vec3 vaVec = glm::vec3(va[0], va[1], va[2]);
		glm::vec3 vbVec = glm::vec3(vb[0], vb[1], vb[2]);
		glm::vec3 vcVec = glm::vec3(vc[0], vc[1], vc[2]);

		// Find the distance from the eye to screen plane.

		d = -(glm::dot(vaVec, vnVec));

		// Find the extent of the perpendicular projection.

		l = glm::dot(vrVec, vaVec) * n / d;
		r = glm::dot(vrVec, vbVec) * n / d;
		b = glm::dot(vuVec, vaVec) * n / d;
		t = glm::dot(vuVec, vcVec) * n / d;

		// Load the perpendicular projection.

		//glMatrixMode(GL_PROJECTION);
		//glLoadIdentity();
		frust = glm::frustum(l, r, b, t, n, f);

		// Rotate the projection to be non-perpendicular.

		glm::mat4 MTrans;

		glm::vec4 M0 = glm::vec4(vrVec.x, vuVec.x, vnVec.x, 0.0f);
		glm::vec4 M1 = glm::vec4(vrVec.y, vuVec.y, vnVec.y, 0.0f);
		glm::vec4 M2 = glm::vec4(vrVec.z, vuVec.z, vnVec.z, 0.0f);
		glm::vec4 M3 = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

		MTrans[0] = M0;
		MTrans[1] = M1;
		MTrans[2] = M2;
		MTrans[3] = M3;

		glm::mat4 T;

		glm::vec4 T0 = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec4 T1 = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
		glm::vec4 T2 = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
		glm::vec4 T3 = glm::vec4(-pe[0], -pe[1], -pe[2], 1.0f);

		T[0] = T0;
		T[1] = T1;
		T[2] = T2;
		T[3] = T3;

		return frust * MTrans * T;
	}

	virtual void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose) = 0;
};

//////////////////////////////////////////////////////////////////////
//
// The remainder of this code is specific to the scene we want to 
// render.  I use oglplus to render an array of cubes, but your 
// application would perform whatever rendering you want
//


//////////////////////////////////////////////////////////////////////
//
// OGLplus is a set of wrapper classes for giving OpenGL a more object
// oriented interface
//
#define OGLPLUS_USE_GLCOREARB_H 0
#define OGLPLUS_USE_GLEW 1
#define OGLPLUS_USE_BOOST_CONFIG 0
#define OGLPLUS_NO_SITE_CONFIG 1
#define OGLPLUS_LOW_PROFILE 1

#pragma warning( disable : 4068 4244 4267 4065)
#include <oglplus/config/basic.hpp>
#include <oglplus/config/gl.hpp>
#include <oglplus/all.hpp>
#include <oglplus/interop/glm.hpp>
#include <oglplus/bound/texture.hpp>
#include <oglplus/bound/framebuffer.hpp>
#include <oglplus/bound/renderbuffer.hpp>
#include <oglplus/bound/buffer.hpp>
#include <oglplus/shapes/cube.hpp>
#include <oglplus/shapes/wrapper.hpp>
#pragma warning( default : 4068 4244 4267 4065)



namespace Attribute {
	enum {
		Position = 0,
		TexCoord0 = 1,
		Normal = 2,
		Color = 3,
		TexCoord1 = 4,
		InstanceTransform = 5,
	};
}


// An example application that renders a simple cube
class ExampleApp : public RiftApp {

public:
	ExampleApp() { }


protected:
	void initGl() override {
		RiftApp::initGl();
		Window::initialize(_session);

	}

	void shutdownGl() override {
		//cubeScene.reset();
	}

	void resetState() {
		Window::reset(_session);
	}


	void keyCallback() {
		ovrInputState inputState;

		ovrPosef eyePoses[2];
		ovr_GetEyePoses(_session, frame, true, _viewScaleDesc.HmdToEyeOffset, eyePoses, &_sceneLayer.SensorSampleTime);
		
		if (OVR_SUCCESS(ovr_GetInputState(_session, ovrControllerType_Touch, &inputState))) {
			

		}
	}

	

	void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose) override {
		

		Window::displayCallback(projection, headPose);
		
		//Window::displayCallback(projection, headPose, _session, frame);
		//keyCallback();

	}
	void onKey(int key, int scancode, int action, int mods) override {
		if (GLFW_PRESS == action) switch (key) {
		case GLFW_KEY_R:
			
		case GLFW_KEY_T: // debug key that prints current head position and orientation

			ovrPosef eyePoses[2];
			ovr_GetEyePoses(_session, frame, true, _viewScaleDesc.HmdToEyeOffset, eyePoses, &_sceneLayer.SensorSampleTime);

			char buff[100];
			sprintf_s(buff, "(%f, %f, %f)\n", eyePoses[0].Position.x, eyePoses[0].Position.y, eyePoses[0].Position.z);
			OutputDebugStringA(buff);

			sprintf_s(buff, "(%f, %f, %f)\n", eyePoses[1].Position.x, eyePoses[1].Position.y, eyePoses[1].Position.z);
			OutputDebugStringA(buff);

			sprintf_s(buff, "(%f, %f, %f, %f)\n", eyePoses[0].Orientation.x, eyePoses[0].Orientation.y, eyePoses[0].Orientation.z, eyePoses[0].Orientation.w);
			OutputDebugStringA(buff);

			sprintf_s(buff, "(%f, %f, %f, %f)\n", eyePoses[1].Orientation.x, eyePoses[1].Orientation.y, eyePoses[1].Orientation.z, eyePoses[1].Orientation.w);

			OutputDebugStringA(buff);
			return;
		}

		GlfwApp::onKey(key, scancode, action, mods);
	}
};

// Execute our example class
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	int result = -1;
	try {
		if (!OVR_SUCCESS(ovr_Initialize(nullptr))) {
			FAIL("Failed to initialize the Oculus SDK");
		}
		result = ExampleApp().run();
	}
	catch (std::exception & error) {
		OutputDebugStringA(error.what());
		std::cerr << error.what() << std::endl;
	}
	ovr_Shutdown();
	return result;
}