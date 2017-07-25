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
#include "Model.h"
#include "shader.h"
#include "Lights.h"
#include "Molecule.h"
#include "Remote.h"
#include <ctime>



///////////////////////////////////////////////////////////////////////////////
//
// GLM is a C++ math library meant to mirror the syntax of GLSL 
//

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

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

#include <GLFW/glfw3.h>

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

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

namespace ovr {

	// Convenience method for looping over each eye with a lambda
	template <typename Function>
	inline void for_each_eye(Function function) {
		for (ovrEyeType eye = ovrEyeType::ovrEye_Left;
			eye < ovrEyeType::ovrEye_Count;
			eye = static_cast<ovrEyeType>(eye + 1)) {
			function(eye);
		}
	}

	inline mat4 toGlm(const ovrMatrix4f & om) {
		return glm::transpose(glm::make_mat4(&om.M[0][0]));
	}

	inline mat4 toGlm(const ovrFovPort & fovport, float nearPlane = 0.01f, float farPlane = 10000.0f) {
		return toGlm(ovrMatrix4f_Projection(fovport, nearPlane, farPlane, true));
	}

	inline vec3 toGlm(const ovrVector3f & ov) {
		return glm::make_vec3(&ov.x);
	}

	inline vec2 toGlm(const ovrVector2f & ov) {
		return glm::make_vec2(&ov.x);
	}

	inline uvec2 toGlm(const ovrSizei & ov) {
		return uvec2(ov.w, ov.h);
	}

	inline quat toGlm(const ovrQuatf & oq) {
		return glm::make_quat(&oq.x);
	}

	inline mat4 toGlm(const ovrPosef & op) {
		mat4 orientation = glm::mat4_cast(toGlm(op.Orientation));
		mat4 translation = glm::translate(mat4(), ovr::toGlm(op.Position));
		return translation * orientation;
	}

	inline ovrMatrix4f fromGlm(const mat4 & m) {
		ovrMatrix4f result;
		mat4 transposed(glm::transpose(m));
		memcpy(result.M, &(transposed[0][0]), sizeof(float) * 16);
		return result;
	}

	inline ovrVector3f fromGlm(const vec3 & v) {
		ovrVector3f result;
		result.x = v.x;
		result.y = v.y;
		result.z = v.z;
		return result;
	}

	inline ovrVector2f fromGlm(const vec2 & v) {
		ovrVector2f result;
		result.x = v.x;
		result.y = v.y;
		return result;
	}

	inline ovrSizei fromGlm(const uvec2 & v) {
		ovrSizei result;
		result.w = v.x;
		result.h = v.y;
		return result;
	}

	inline ovrQuatf fromGlm(const quat & q) {
		ovrQuatf result;
		result.x = q.x;
		result.y = q.y;
		result.z = q.z;
		result.w = q.w;
		return result;
	}
}

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
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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

		ovrMirrorTextureDesc mirrorDesc;
		memset(&mirrorDesc, 0, sizeof(mirrorDesc));
		mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		mirrorDesc.Width = _mirrorSize.x;
		mirrorDesc.Height = _mirrorSize.y;
		if (!OVR_SUCCESS(ovr_CreateMirrorTextureGL(_session, &mirrorDesc, &_mirrorTexture))) {
			FAIL("Could not create mirror texture");
		}
		glGenFramebuffers(1, &_mirrorFbo);
	}

	void onKey(int key, int scancode, int action, int mods) override {
		if (GLFW_PRESS == action) switch (key) {
		case GLFW_KEY_R:
			ovr_RecenterTrackingOrigin(_session);
			return;
		}

		GlfwApp::onKey(key, scancode, action, mods);
	}

	void draw() final override {
		ovrPosef eyePoses[2];
		ovr_GetEyePoses(_session, frame, true, _viewScaleDesc.HmdToEyeOffset, eyePoses, &_sceneLayer.SensorSampleTime);

		int curIndex;
		ovr_GetTextureSwapChainCurrentIndex(_session, _eyeTexture, &curIndex);
		GLuint curTexId;
		ovr_GetTextureSwapChainBufferGL(_session, _eyeTexture, curIndex, &curTexId);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ovr::for_each_eye([&](ovrEyeType eye) {
			const auto& vp = _sceneLayer.Viewport[eye];
			glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
			_sceneLayer.RenderPose[eye] = eyePoses[eye];
			renderScene(_eyeProjections[eye], ovr::toGlm(eyePoses[eye]));
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
	//std::shared_ptr<ColorCubeScene> cubeScene;

public:
	ExampleApp() { }
	GLint shaderProgram;
	GLint trackShader;

	Model* factory;
	Model* co2;
	Model* o2;
	vector<Molecule*> co2_mols; // vector of all co2 molecules in the scene
	vector<Molecule*> o2_mols; // vector of all o2 molecules in the scene
	Lights* light;
	vector<Remote*> remotes;

	clock_t begin_time;
	bool endState = false;

protected:
	void initGl() override {
		RiftApp::initGl();
		glClearColor(0.0f, 0.0f, 0.5f, 0.0f);
		glEnable(GL_DEPTH_TEST);
		ovr_RecenterTrackingOrigin(_session);


		remotes.push_back(new Remote());
		remotes.push_back(new Remote());
		factory = new Model("../models/factory2/factory2.obj");
		factory->toWorld = glm::scale(factory->toWorld, glm::vec3(0.25f, 0.25f, 0.25f));
		factory->toWorld = glm::translate(factory->toWorld, glm::vec3(0.0f, -10.0f, -60.0f));
		light = new Lights(1);
		co2 = new Model("../models/co2/co2.obj");
		o2 = new Model("../models/o2/o2.obj");
		Molecule* temp;

		// adds 5 molecules with random displacement to the scene
		for (int i = 0; i < 5; i++) {
			temp = new Molecule(co2);
			co2_mols.push_back(temp);
		}

		shaderProgram = LoadShaders("../shader.vert", "../shader.frag");
		trackShader = LoadShaders("../trackshader.vert", "../trackshader.frag");
		begin_time = clock();

	}

	void shutdownGl() override {
		//cubeScene.reset();
	}

	void resetState() {
		co2_mols.clear();
		o2_mols.clear();

		begin_time = clock();
		endState = false;

		// adds 5 molecules with random displacement to the scene
		Molecule* temp;
		for (int i = 0; i < 5; i++) {
			temp = new Molecule(co2);
			co2_mols.push_back(temp);
		}

		glClearColor(0.0f, 0.0f, 0.5f, 0.0f);


		ovr_RecenterTrackingOrigin(_session);
	}

	void keyCallback() {
		ovrInputState inputState;

		if (OVR_SUCCESS(ovr_GetInputState(_session, ovrControllerType_Touch, &inputState))) {

			// Left Trigger
			if (inputState.IndexTrigger[ovrHand_Left] > 0.5f) {
				// Change left line to red
				remotes[0]->colorVal = vec3(1.0f, 0.0f, 0.0f);
			}
			else {
				// Change left line to green
				remotes[0]->colorVal = vec3(0.0f, 1.0f, 0.0f);

			}

			// Right Trigger
			if (inputState.IndexTrigger[ovrHand_Right] > 0.5f) {
				// Change right line to red
				remotes[1]->colorVal = vec3(1.0f, 0.0f, 0.0f);

				if (inputState.IndexTrigger[ovrHand_Left] > 0.5f) {
					// Check for intersection
					detectCollision();
				}
			}
			else {
				// Change right line to green
				remotes[1]->colorVal = vec3(0.0f, 1.0f, 0.0f);

			}

			// Every other button should restart game if game is over
			// If game is still going then do nothing
			if (inputState.Buttons & ovrButton_A & endState)
			{
				resetState();
			}
			if (inputState.Buttons & ovrButton_B & endState)
			{
				resetState();
			}
			if (inputState.Buttons & ovrButton_RThumb & endState)
			{
				resetState();
			}
			if (inputState.Buttons & ovrButton_X & endState)
			{
				resetState();
			}
			if (inputState.Buttons & ovrButton_Y & endState)
			{
				resetState();
			}
			if (inputState.Buttons & ovrButton_LThumb & endState)
			{
				resetState();
			}
			if (inputState.Buttons & ovrButton_Enter & endState)
			{
				resetState();
			}
			if (inputState.HandTrigger[ovrHand_Left] > 0.5f && endState) {
				resetState();
			}
			if (inputState.HandTrigger[ovrHand_Right] > 0.5f && endState) {
				resetState();
			}

		}
	}

	void detectCollision() {
		Molecule* temp;
		Molecule* o2_temp;
		char buff[100];

		for (int i = 0; i < co2_mols.size();i++) {
			temp = co2_mols[i];
			int numBooms = 0;
			for (int j = 0; j < 2; j++) {
				vector<glm::vec3> coords = remotes[j]->calcCoords();
				float numerator = glm::length(glm::cross((temp->center - coords[0]), (temp->center - coords[1])));
				float denominator = glm::length(coords[1] - coords[0]);

				float dist = numerator / denominator;

				if (dist < 5.0f) {
					numBooms++;

				}

			}
			if (numBooms == 2) {
				// delete the co2 molecule
				sprintf_s(buff, "BOOM\n");
				OutputDebugStringA(buff);
				o2_temp = new Molecule(o2);
				o2_temp->center = co2_mols[i]->center;
				o2_mols.push_back(o2_temp);

				co2_mols.erase(co2_mols.begin() + i);
				if (co2_mols.size() == 0) {
					endState = true;
					glClearColor(0.25f, 0.5f, 1.0f, 0.0f);
				}
			}
			else {
				sprintf_s(buff, "NOT BOOM\n");
				OutputDebugStringA(buff);
			}

		}

	}

	void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose) override {

		if (!endState) {
			clock_t end_time = clock();
			if (double(end_time - begin_time) / CLOCKS_PER_SEC > 1.0) {
				co2_mols.push_back(new Molecule(co2));

				if (co2_mols.size() > 10) {
					endState = true;
					for (int i = 0; i < 100; i++) {
						co2_mols.push_back(new Molecule(co2));
					}
				}

				begin_time = clock();
			}
		}
		// update all CO2 molecules
		for (int i = 0; i < co2_mols.size(); i++) {
			co2_mols[i]->update(endState);
		}

		for (int i = 0; i < o2_mols.size(); i++) {
			o2_mols[i]->update(endState);
		}

		glUseProgram(shaderProgram);

		GLuint uProjection = glGetUniformLocation(shaderProgram, "projection");
		GLuint uModelView = glGetUniformLocation(shaderProgram, "view");

		glUniformMatrix4fv(uProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uModelView, 1, GL_FALSE, glm::value_ptr(glm::inverse(headPose)));

		// lighting block
		GLuint MatrixID = glGetUniformLocation(shaderProgram, "light.intensity");
		glUniform3f(MatrixID, (light->intensity.x), (light->intensity.y), (light->intensity.z));

		MatrixID = glGetUniformLocation(shaderProgram, "light.direction");
		glUniform3f(MatrixID, (light->direction.x), (light->direction.y), (light->direction.z));

		MatrixID = glGetUniformLocation(shaderProgram, "light.ambient");
		glUniform1f(MatrixID, light->ambient);

		MatrixID = glGetUniformLocation(shaderProgram, "light.pos");
		glUniform3f(MatrixID, light->position.x, light->position.y, light->position.z);

		MatrixID = glGetUniformLocation(shaderProgram, "light.spec");
		glUniform1f(MatrixID, light->specular);

		MatrixID = glGetUniformLocation(shaderProgram, "light.theta");
		glUniform1f(MatrixID, light->theta);

		MatrixID = glGetUniformLocation(shaderProgram, "light.cosExp");
		glUniform1f(MatrixID, light->cosExp);

		MatrixID = glGetUniformLocation(shaderProgram, "cameraPos");
		glUniform3f(MatrixID, (0.0f), (0.0f), (0.0f));


		factory->Draw(shaderProgram); // Draw the factory

		// draw all active molecules
		for (int i = 0; i < co2_mols.size(); i++) {
			co2_mols[i]->Draw(shaderProgram);
		}

		for (int i = 0; i < o2_mols.size(); i++) {
			o2_mols[i]->Draw(shaderProgram);
		}

		// Touch controller schtuff
		double displayMidpointSeconds = ovr_GetPredictedDisplayTime(_session, frame);
		ovrTrackingState trackState = ovr_GetTrackingState(_session, displayMidpointSeconds, ovrTrue);

		ovrPosef leftHandPose = trackState.HandPoses[ovrHand_Left].ThePose;
		ovrPosef rightHandPose = trackState.HandPoses[ovrHand_Right].ThePose;

		glm::quat myQuat(leftHandPose.Orientation.w, leftHandPose.Orientation.x, leftHandPose.Orientation.y, leftHandPose.Orientation.z);

		remotes[0]->position = ovr::toGlm(leftHandPose.Position);
		remotes[0]->quat = glm::toMat4(myQuat);

		myQuat = glm::quat(rightHandPose.Orientation.w, rightHandPose.Orientation.x, rightHandPose.Orientation.y, rightHandPose.Orientation.z);

		remotes[1]->position = ovr::toGlm(rightHandPose.Position);
		remotes[1]->quat = glm::toMat4(myQuat);

		// key callback
		keyCallback();

		glUseProgram(trackShader);

		uProjection = glGetUniformLocation(trackShader, "projection");
		uModelView = glGetUniformLocation(trackShader, "view");

		glUniformMatrix4fv(uProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uModelView, 1, GL_FALSE, glm::value_ptr(glm::inverse(headPose)));
		remotes[0]->Draw(trackShader);
		remotes[1]->Draw(trackShader);


	}
	void onKey(int key, int scancode, int action, int mods) override {
		if (GLFW_PRESS == action) switch (key) {
		case GLFW_KEY_R:
			co2_mols.clear();
			o2_mols.clear();

			begin_time = clock();
			endState = false;

			// adds 5 molecules with random displacement to the scene
			Molecule* temp;
			for (int i = 0; i < 5; i++) {
				temp = new Molecule(co2);
				co2_mols.push_back(temp);
			}

			ovr_RecenterTrackingOrigin(_session);
			return;
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