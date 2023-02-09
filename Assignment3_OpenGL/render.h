#pragma once
#include "level.h"

// Used to print debug infomation from OpenGL, pulled straight from the official OpenGL wiki.
#ifndef NDEBUG
void MessageCallback(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length,
	const GLchar* message, const void* userParam) {
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}
#endif

class Render {
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GOpenGLSurface ogl;

	GW::INPUT::GInput input;

	GLuint vertexShader = 0;
	GLuint fragmentShader = 0;
	GLuint shaderExecutable = 0;

	Level level;
	GW::MATH::GMatrix matrixHelper;
	GW::MATH::GVector vectorHelper;

	std::chrono::steady_clock::time_point lastUpdate;
	float deltaTime;
	float aspRat = 0;
	const float Camera_Speed = 5.0f;

public:
	Render(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GOpenGLSurface _ogl, Level _level) {
		win = _win;
		ogl = _ogl;
		level = _level;

		matrixHelper.Create();
		vectorHelper.Create();
		input.Create(win);

		ogl.GetAspectRatio(aspRat);

		level.ubo.world_Matrix = GW::MATH::GIdentityMatrixF;

		//GW::MATH::GVECTORF eye = { 4.75f, 5.5f, 5.0f, 1.0f };
		GW::MATH::GVECTORF eye = {4.0f, 4.0f, -4.0f, 1.0f };
		//GW::MATH::GVECTORF at = { 1.5f, 1.75f, 0.0f, 1.0f };
		GW::MATH::GVECTORF at = { 0.0f, 0.0f, 0.0f, 1.0f };
		GW::MATH::GVECTORF up = { 0.0f, 1.0f, 0.0f, 0.0f };
		GW::MATH::GVECTORF ambient = { 0.25f, 0.25f, -0.35f, 0.0f };
		GW::MATH::GVECTORF light_direction = { -1.0f, -4.0f, -2.0f, 1.0f };
		GW::MATH::GVECTORF light_color = { 1.0f, 1.0f, 1.0f, 1.0f };

		level.ubo.viewPos = eye;
		level.ubo.lightDir = light_direction;
		level.ubo.lightColor = light_color;

		matrixHelper.LookAtLHF(eye, at, up, level.ubo.view_Matrix);
		matrixHelper.ProjectionOpenGLLHF(G_DEGREE_TO_RADIAN(65), aspRat, .1, 100, level.ubo.proj_Matrix);
		matrixHelper.MultiplyMatrixF(level.ubo.view_Matrix, level.ubo.proj_Matrix, level.ubo.viewproj_Matrix);


		LoadExtensions();
		// In debug mode we link openGL errors to the console
#ifndef NDEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(MessageCallback, 0);
#endif

		glGenVertexArrays(1, &level.vertexArray);
		glBindVertexArray(level.vertexArray);

		glGenBuffers(1, &level.vertexBufferObject);
		glBindBuffer(GL_ARRAY_BUFFER, level.vertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER, sizeof(VERTEX) * level.allVert.size(), level.allVert.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VERTEX), (void*)0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VERTEX), (void*)12);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VERTEX), (void*)24);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		glGenBuffers(1, &level.indexBufferObject);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, level.indexBufferObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * level.allIndicies.size(), level.allIndicies.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glGenBuffers(1, &level.uniformBufferObject);
		glBindBuffer(GL_UNIFORM_BUFFER, level.uniformBufferObject);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(level.ubo), &level.ubo, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);

#pragma region Shaders
		std::string vertexShaderSource = ShaderAsString("vs.glsl");
		const char* vssource = vertexShaderSource.c_str();
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vssource, NULL);
		glCompileShader(vertexShader);
		char errors[1024]; GLint result;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
		if (result == false) {
			glGetShaderInfoLog(vertexShader, 1024, NULL, errors);
			std::cout << errors << std::endl;
		}

		std::string fragmentShaderSource = ShaderAsString("fs.glsl");
		const char* ffsource = fragmentShaderSource.c_str();
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &ffsource, NULL);
		glCompileShader(fragmentShader);
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
		if (result == false) {
			glGetShaderInfoLog(fragmentShader, 1024, NULL, errors);
			std::cout << errors << std::endl;
		}
		shaderExecutable = glCreateProgram();
		glAttachShader(shaderExecutable, vertexShader);
		glAttachShader(shaderExecutable, fragmentShader);
		glLinkProgram(shaderExecutable);
		glGetProgramiv(shaderExecutable, GL_LINK_STATUS, &result);
		if (result == false)
		{
			glGetProgramInfoLog(shaderExecutable, 1024, NULL, errors);
			std::cout << errors << std::endl;
		}

#pragma endregion

	}
	void render() {
		glBindVertexArray(level.vertexArray);
		glUseProgram(shaderExecutable);

		GLint uboIndex = glGetUniformBlockIndex(shaderExecutable, "ubo_data");
		glUniformBlockBinding(shaderExecutable, uboIndex, 0);

		/*int currOffset = 0;
		for (size_t i = 0; i < level.toyBox.size(); i++)
		{
			level.ubo.world_Matrix = level.toyBox[i].model;

			glBindBufferBase(GL_UNIFORM_BUFFER, uboIndex, level.uniformBufferObject);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UBO), &level.ubo);
		
		
			//matrixHelper.MultiplyMatrixF(level.toyBox[i].model, level.ubo.world_Matrix, level.ubo.world_Matrix);
			level.ubo.material = level.toyBox[i].source.materials.back().attrib; // Cannot stay hardcoded

			glDrawElements(GL_TRIANGLES, level.toyBox[i].source.indexCount, GL_UNSIGNED_INT,
				(GLvoid*)(currOffset * sizeof(unsigned int)));

			glBindBuffer(GL_UNIFORM_BUFFER, 0);
			currOffset += level.toyBox[i].source.indexCount;
		}*/

		int currOffset = 0;
		for (size_t i = 0; i < level.toyBox.size(); i++)
		{
			level.ubo.world_Matrix = level.toyBox[i].model;

			UBO currUB0 = level.ubo;
			H2B::Parser currSource = level.toyBox[i].source;

			int currMat = currSource.materialCount;

			for (size_t j = 0; j < currSource.meshCount; j++)
			{

				glBindBufferBase(GL_UNIFORM_BUFFER, uboIndex, level.uniformBufferObject);
				glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UBO), &currUB0);

				//matrixHelper.MultiplyMatrixF(level.toyBox[i].model, level.ubo.world_Matrix, level.ubo.world_Matrix);
				currUB0.material = currSource.materials[currSource.meshes[currMat-1].materialIndex].attrib;
				//currUB0.material = ; // Cannot stay hardcoded

				glDrawElements(GL_TRIANGLES, currSource.meshes[j].drawInfo.indexCount, GL_UNSIGNED_INT,
					(GLvoid*)(currSource.meshes[j].drawInfo.indexOffset * sizeof(unsigned int)));

				glBindBuffer(GL_UNIFORM_BUFFER, 0);
				currMat--;
			}
			
			currOffset += currSource.indexCount;
		}

		glBindVertexArray(0);
	}
	~Render()
	{
		glDeleteVertexArrays(1, &level.getVertexArray());
		glDeleteBuffers(1, &level.getVertexBufferObject());
		glDeleteBuffers(1, &level.getIndexBufferObject());
		glDeleteBuffers(1, &level.getUniformBufferObject());

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		glDeleteProgram(shaderExecutable);
	}
	void UpdateCamera() {
		GW::MATH::GMATRIXF camera_matrix = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF translation_matrix = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF pitchMatrix = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF yaw_Matrix = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF ry = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF rx = GW::MATH::GIdentityMatrixF;

		matrixHelper.InverseF(level.ubo.view_Matrix, camera_matrix);
#pragma region WASD / Up & Down

		float yChange = 0;
		float xChange = 0;
		float zChange = 0;
		float perFrame = 0;
		GW::MATH::GVECTORF pos = { 0,0,0,0 };

		// Keys
		float spacebarState, lShiftState, wKeyState, sKeyState,
			dKeyState, aKeyState;

		// Controller
		float lStickYState = 0;
		float lStickXState = 0;
		float rTriggerState = 0;
		float lTriggerState = 0;

		// Getting KeyStates
		input.GetState(23, spacebarState);
		input.GetState(14, lShiftState);
		input.GetState(60, wKeyState);
		input.GetState(56, sKeyState);
		input.GetState(41, dKeyState);
		input.GetState(38, aKeyState);

		// Change
		zChange = wKeyState - sKeyState + lStickYState;
		xChange = dKeyState - aKeyState + lStickXState;
		yChange = spacebarState - lShiftState + rTriggerState - lTriggerState;

		// Time
		auto now = std::chrono::steady_clock::now();
		deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(now - lastUpdate).count() / 1000000.0f;
		lastUpdate = now;

		// Get position
		GW::MATH::GVECTORF newPos = {0, (-yChange * Camera_Speed * deltaTime), 0, 0};
		vectorHelper.AddVectorF(pos, newPos, pos);

		perFrame = Camera_Speed * deltaTime;

		GW::MATH::GVECTORF translation = { xChange * perFrame, 0, zChange * perFrame, 0 };
		matrixHelper.TranslateGlobalF(translation_matrix, translation, translation_matrix);
		matrixHelper.MultiplyMatrixF(translation_matrix, camera_matrix, camera_matrix);

		GW::MATH::GVECTORF camPos = pos;
		//matrixHelper.TranslateGlobalF(camera_matrix, camPos, camera_matrix);

#pragma endregion

#pragma region Rotation

		float mouseXDelta;
		float mouseYDelta;
		float fov = G_DEGREE_TO_RADIAN_F(65);
		float thumbSpeed = (float)G_PI * deltaTime;

		// Controller
		float rStickYState = 0;
		float rStickXState = 0;
		float ar = aspRat;

		unsigned int height;
		unsigned int width;
		win.GetHeight(height);
		win.GetWidth(width);

		UNIVERSAL_WINDOW_HANDLE window;
		win.GetWindowHandle(window);

		if (window.window == GetFocus()) {
			GW::GReturn result = input.GetMouseDelta(mouseXDelta, mouseYDelta);
			if (G_PASS(result) && result != GW::GReturn::REDUNDANT) {
				//float totalPitch = fov * mouseYDelta / width + rStickYState * -thumbSpeed;
				//float totalYaw = fov * ar * mouseXDelta / height + rStickXState * thumbSpeed;

				matrixHelper.RotateYGlobalF(ry, G2D_DEGREE_TO_RADIAN_F(mouseXDelta * .2), ry);
				matrixHelper.RotateXLocalF(rx, G2D_DEGREE_TO_RADIAN_F(mouseYDelta * .2), rx);

				matrixHelper.MultiplyMatrixF(rx, camera_matrix, camera_matrix);

				pos = camPos;
				camPos = { 0, 0, 0, 0 };
				matrixHelper.MultiplyMatrixF(camera_matrix, ry, camera_matrix);
				camPos = pos;

			}
		}
#pragma endregion

		matrixHelper.InverseF(camera_matrix, level.ubo.view_Matrix);
		matrixHelper.MultiplyMatrixF(level.ubo.view_Matrix, level.ubo.proj_Matrix, level.ubo.viewproj_Matrix);
	}

private:
	std::string ShaderAsString(const char* shaderFilePath) {
		std::string output;
		unsigned int stringLength = 0;
		GW::SYSTEM::GFile file; file.Create();
		file.GetFileSize(shaderFilePath, stringLength);
		if (stringLength && +file.OpenBinaryRead(shaderFilePath)) {
			output.resize(stringLength);
			file.Read(&output[0], stringLength);
		}
		else
			std::cout << "ERROR: Shader Source File \"" << shaderFilePath << "\" Not Found!" << std::endl;
		return output;
	}
#pragma region Queried API Functions

	// Modern OpenGL API Functions must be queried before use
	PFNGLCREATESHADERPROC				glCreateShader = nullptr;
	PFNGLSHADERSOURCEPROC				glShaderSource = nullptr;
	PFNGLCOMPILESHADERPROC				glCompileShader = nullptr;
	PFNGLGETSHADERIVPROC				glGetShaderiv = nullptr;
	PFNGLGETSHADERINFOLOGPROC			glGetShaderInfoLog = nullptr;
	PFNGLATTACHSHADERPROC				glAttachShader = nullptr;
	PFNGLDETACHSHADERPROC				glDetachShader = nullptr;
	PFNGLDELETESHADERPROC				glDeleteShader = nullptr;
	PFNGLCREATEPROGRAMPROC				glCreateProgram = nullptr;
	PFNGLLINKPROGRAMPROC				glLinkProgram = nullptr;
	PFNGLUSEPROGRAMPROC					glUseProgram = nullptr;
	PFNGLGETPROGRAMIVPROC				glGetProgramiv = nullptr;
	PFNGLGETPROGRAMINFOLOGPROC			glGetProgramInfoLog = nullptr;
	PFNGLGENVERTEXARRAYSPROC			glGenVertexArrays = nullptr;
	PFNGLBINDVERTEXARRAYPROC			glBindVertexArray = nullptr;
	PFNGLGENBUFFERSPROC					glGenBuffers = nullptr;
	PFNGLBINDBUFFERPROC					glBindBuffer = nullptr;
	PFNGLBUFFERDATAPROC					glBufferData = nullptr;
	PFNGLENABLEVERTEXATTRIBARRAYPROC	glEnableVertexAttribArray = nullptr;
	PFNGLDISABLEVERTEXATTRIBARRAYPROC	glDisableVertexAttribArray = nullptr;
	PFNGLVERTEXATTRIBPOINTERPROC		glVertexAttribPointer = nullptr;
	PFNGLGETUNIFORMLOCATIONPROC			glGetUniformLocation = nullptr;
	PFNGLUNIFORMMATRIX4FVPROC			glUniformMatrix4fv = nullptr;
	PFNGLDELETEBUFFERSPROC				glDeleteBuffers = nullptr;
	PFNGLDELETEPROGRAMPROC				glDeleteProgram = nullptr;
	PFNGLDELETEVERTEXARRAYSPROC			glDeleteVertexArrays = nullptr;
	PFNGLDEBUGMESSAGECALLBACKPROC		glDebugMessageCallback = nullptr;
	PFNGLGETUNIFORMBLOCKINDEXPROC		glGetUniformBlockIndex = nullptr;
	PFNGLBINDBUFFERBASEPROC				glBindBufferBase = nullptr;
	PFNGLUNIFORMBLOCKBINDINGPROC		glUniformBlockBinding = nullptr;
	PFNGLBUFFERSUBDATAPROC				glBufferSubData = nullptr;

	void LoadExtensions()
	{
		ogl.QueryExtensionFunction(nullptr, "glCreateShader", (void**)&glCreateShader);
		ogl.QueryExtensionFunction(nullptr, "glShaderSource", (void**)&glShaderSource);
		ogl.QueryExtensionFunction(nullptr, "glCompileShader", (void**)&glCompileShader);
		ogl.QueryExtensionFunction(nullptr, "glGetShaderiv", (void**)&glGetShaderiv);
		ogl.QueryExtensionFunction(nullptr, "glGetShaderInfoLog", (void**)&glGetShaderInfoLog);
		ogl.QueryExtensionFunction(nullptr, "glAttachShader", (void**)&glAttachShader);
		ogl.QueryExtensionFunction(nullptr, "glDetachShader", (void**)&glDetachShader);
		ogl.QueryExtensionFunction(nullptr, "glDeleteShader", (void**)&glDeleteShader);
		ogl.QueryExtensionFunction(nullptr, "glCreateProgram", (void**)&glCreateProgram);
		ogl.QueryExtensionFunction(nullptr, "glLinkProgram", (void**)&glLinkProgram);
		ogl.QueryExtensionFunction(nullptr, "glUseProgram", (void**)&glUseProgram);
		ogl.QueryExtensionFunction(nullptr, "glGetProgramiv", (void**)&glGetProgramiv);
		ogl.QueryExtensionFunction(nullptr, "glGetProgramInfoLog", (void**)&glGetProgramInfoLog);
		ogl.QueryExtensionFunction(nullptr, "glGenVertexArrays", (void**)&glGenVertexArrays);
		ogl.QueryExtensionFunction(nullptr, "glBindVertexArray", (void**)&glBindVertexArray);
		ogl.QueryExtensionFunction(nullptr, "glGenBuffers", (void**)&glGenBuffers);
		ogl.QueryExtensionFunction(nullptr, "glBindBuffer", (void**)&glBindBuffer);
		ogl.QueryExtensionFunction(nullptr, "glBufferData", (void**)&glBufferData);
		ogl.QueryExtensionFunction(nullptr, "glEnableVertexAttribArray", (void**)&glEnableVertexAttribArray);
		ogl.QueryExtensionFunction(nullptr, "glDisableVertexAttribArray", (void**)&glDisableVertexAttribArray);
		ogl.QueryExtensionFunction(nullptr, "glVertexAttribPointer", (void**)&glVertexAttribPointer);
		ogl.QueryExtensionFunction(nullptr, "glGetUniformLocation", (void**)&glGetUniformLocation);
		ogl.QueryExtensionFunction(nullptr, "glUniformMatrix4fv", (void**)&glUniformMatrix4fv);
		ogl.QueryExtensionFunction(nullptr, "glDeleteBuffers", (void**)&glDeleteBuffers);
		ogl.QueryExtensionFunction(nullptr, "glDeleteProgram", (void**)&glDeleteProgram);
		ogl.QueryExtensionFunction(nullptr, "glDeleteVertexArrays", (void**)&glDeleteVertexArrays);
		ogl.QueryExtensionFunction(nullptr, "glDebugMessageCallback", (void**)&glDebugMessageCallback);
		ogl.QueryExtensionFunction(nullptr, "glGetUniformBlockIndex", (void**)&glGetUniformBlockIndex);
		ogl.QueryExtensionFunction(nullptr, "glBindBufferBase", (void**)&glBindBufferBase);
		ogl.QueryExtensionFunction(nullptr, "glUniformBlockBinding", (void**)&glUniformBlockBinding);
		ogl.QueryExtensionFunction(nullptr, "glBufferSubData", (void**)&glBufferSubData);
	}
#pragma endregion

};
