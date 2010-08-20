//**********************************
// OpenGL Layered rendering
// 19/08/2010 - 20/08/2010
//**********************************
// Christophe Riccio
// g.truc.creation@gmail.com
//**********************************
// G-Truc Creation
// www.g-truc.net
//**********************************

#include <glf/glf.hpp>

namespace
{
	std::string const SAMPLE_NAME = "OpenGL Layered rendering";
	std::string const VERT_SHADER_SOURCE1(glf::DATA_DIRECTORY + "410/layer.vert");
	std::string const GEOM_SHADER_SOURCE1(glf::DATA_DIRECTORY + "410/layer.geom");
	std::string const FRAG_SHADER_SOURCE1(glf::DATA_DIRECTORY + "410/layer.frag");
	std::string const VERT_SHADER_SOURCE2(glf::DATA_DIRECTORY + "410/viewport.vert");
	std::string const GEOM_SHADER_SOURCE2(glf::DATA_DIRECTORY + "410/viewport.geom");
	std::string const FRAG_SHADER_SOURCE2(glf::DATA_DIRECTORY + "410/viewport.frag");
	glm::ivec2 const FRAMEBUFFER_SIZE(320, 240);
	int const SAMPLE_SIZE_WIDTH = 640;
	int const SAMPLE_SIZE_HEIGHT = 480;
	int const SAMPLE_MAJOR_VERSION = 4;
	int const SAMPLE_MINOR_VERSION = 1;

	glf::window Window(glm::ivec2(SAMPLE_SIZE_WIDTH, SAMPLE_SIZE_HEIGHT));

	GLsizei const VertexCount = 4;
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 1.0f))
	};

	GLsizei const ElementCount = 6;
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
	GLushort const ElementData[ElementCount] =
	{
		0, 1, 2, 
		2, 3, 0
	};

	enum buffer_type
	{
		BUFFER_VERTEX,
		BUFFER_ELEMENT,
		BUFFER_MAX
	};

	enum program 
	{
		LAYERING,
		VIEWPORT,
		PROGRAM_MAX
	};

	GLuint FramebufferName = 0;
	GLuint VertexArrayName[PROGRAM_MAX];

	GLuint ProgramName[PROGRAM_MAX];
	GLuint UniformMVP[PROGRAM_MAX];
	GLuint UniformDiffuse = 0;

	GLuint BufferName[BUFFER_MAX];
	GLuint TextureColorbufferName = 0;

}//namespace

bool initProgram()
{
	bool Validated = true;

	if(Validated)
	{
		GLuint VertShaderName = glf::createShader(GL_VERTEX_SHADER, VERT_SHADER_SOURCE1);
		GLuint GeomShaderName = glf::createShader(GL_GEOMETRY_SHADER, GEOM_SHADER_SOURCE1);
		GLuint FragShaderName = glf::createShader(GL_FRAGMENT_SHADER, FRAG_SHADER_SOURCE1);

		ProgramName[LAYERING] = glCreateProgram();
		glAttachShader(ProgramName[LAYERING], VertShaderName);
		glAttachShader(ProgramName[LAYERING], GeomShaderName);
		glAttachShader(ProgramName[LAYERING], FragShaderName);
		glDeleteShader(VertShaderName);
		glDeleteShader(GeomShaderName);
		glDeleteShader(FragShaderName);
		glLinkProgram(ProgramName[LAYERING]);
		Validated = glf::checkProgram(ProgramName[LAYERING]);
	}

	if(Validated)
	{
		GLuint VertShaderName = glf::createShader(GL_VERTEX_SHADER, VERT_SHADER_SOURCE2);
		GLuint GeomShaderName = glf::createShader(GL_GEOMETRY_SHADER, GEOM_SHADER_SOURCE2);
		GLuint FragShaderName = glf::createShader(GL_FRAGMENT_SHADER, FRAG_SHADER_SOURCE2);

		ProgramName[VIEWPORT] = glCreateProgram();
		glAttachShader(ProgramName[VIEWPORT], VertShaderName);
		glAttachShader(ProgramName[VIEWPORT], GeomShaderName);
		glAttachShader(ProgramName[VIEWPORT], FragShaderName);
		glDeleteShader(VertShaderName);
		glDeleteShader(GeomShaderName);
		glDeleteShader(FragShaderName);
		glLinkProgram(ProgramName[VIEWPORT]);
		Validated = glf::checkProgram(ProgramName[VIEWPORT]);
	}

	if(Validated)
	{
		for(std::size_t i = 0; i < PROGRAM_MAX; ++i)
			UniformMVP[i] = glGetUniformLocation(ProgramName[i], "MVP");

		UniformDiffuse = glGetUniformLocation(ProgramName[VIEWPORT], "Diffuse");
	}

	return glf::checkError("initProgram");
}

bool initVertexBuffer()
{
	glGenBuffers(BUFFER_MAX, BufferName);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[BUFFER_ELEMENT]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, BufferName[BUFFER_VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return glf::checkError("initArrayBuffer");
}

bool initTexture()
{
	glGenTextures(1, &TextureColorbufferName);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, TextureColorbufferName);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 1000);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_R, GL_RED);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);

	glTexImage3D(
		GL_TEXTURE_2D_ARRAY, 
		0, 
		GL_RGB, 
		GLsizei(FRAMEBUFFER_SIZE.x), 
		GLsizei(FRAMEBUFFER_SIZE.y), 
		GLsizei(4), //depth
		0,  
		GL_RGB, 
		GL_UNSIGNED_BYTE, 
		NULL);

	return glf::checkError("initTexture");
}

bool initFramebuffer()
{
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, TextureColorbufferName, 0, 0);
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, TextureColorbufferName, 0, 1);
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, TextureColorbufferName, 0, 2);
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, TextureColorbufferName, 0, 3);

	GLenum DrawBuffers[4]= {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
	glDrawBuffers(4, DrawBuffers);

	if(glf::checkFramebuffer(FramebufferName))
		return false;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}

bool initVertexArray()
{
	glGenVertexArrays(PROGRAM_MAX, VertexArrayName);

    glBindVertexArray(VertexArrayName[VIEWPORT]);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[BUFFER_VERTEX]);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), GLF_BUFFER_OFFSET(0));
		glVertexAttribPointer(glf::semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), GLF_BUFFER_OFFSET(sizeof(glm::vec2)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);
		glEnableVertexAttribArray(glf::semantic::attr::TEXCOORD);
	glBindVertexArray(0);

    glBindVertexArray(VertexArrayName[LAYERING]);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[BUFFER_VERTEX]);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), GLF_BUFFER_OFFSET(0));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);
	glBindVertexArray(0);

	return glf::checkError("initVertexArray");
}

bool begin()
{
	GLint MajorVersion = 0;
	GLint MinorVersion = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &MajorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &MinorVersion);
	bool Validated = glf::version(MajorVersion, MinorVersion) >= glf::version(SAMPLE_MAJOR_VERSION, SAMPLE_MINOR_VERSION);

	if(Validated)
		Validated = initProgram();
	if(Validated)
		Validated = initVertexBuffer();
	if(Validated)
		Validated = initVertexArray();
	if(Validated)
		Validated = initTexture();
	if(Validated)
		Validated = initFramebuffer();

	return Validated && glf::checkError("begin");
}

bool end()
{
	glDeleteVertexArrays(PROGRAM_MAX, VertexArrayName);
	glDeleteBuffers(BUFFER_MAX, BufferName);
	glDeleteTextures(1, &TextureColorbufferName);
	glDeleteFramebuffers(1, &FramebufferName);
	glDeleteProgram(ProgramName[VIEWPORT]);
	glDeleteProgram(ProgramName[LAYERING]);

	return glf::checkError("end");
}

void display()
{
	glm::mat4 Projection = glm::ortho(-1.0f, 1.0f, 1.0f,-1.0f, 1.0f, -1.0f);
	glm::mat4 View = glm::mat4(1.0f);
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 MVP = Projection * View * Model;

	// Pass 1
	{
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
		glViewportIndexedfv(0, &glm::vec4(0, 0, FRAMEBUFFER_SIZE)[0]);

		glUseProgram(ProgramName[LAYERING]);
		glUniformMatrix4fv(UniformMVP[LAYERING], 1, GL_FALSE, &MVP[0][0]);

		glBindVertexArray(VertexArrayName[LAYERING]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[BUFFER_ELEMENT]);

		glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, NULL, 1, 0);
	}

	// Pass 2
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewportIndexedfv(0, &glm::vec4(1, 1, FRAMEBUFFER_SIZE - 2)[0]);
		glViewportIndexedfv(1, &glm::vec4((Window.Size.x >> 1) + 1, 0, FRAMEBUFFER_SIZE - 2)[0]);
		glViewportIndexedfv(2, &glm::vec4((Window.Size.x >> 1) + 1, (Window.Size.y >> 1) + 1, FRAMEBUFFER_SIZE - 2)[0]);
		glViewportIndexedfv(3, &glm::vec4(1, (Window.Size.y >> 1) + 1, FRAMEBUFFER_SIZE - 2)[0]);

		glUseProgram(ProgramName[VIEWPORT]);
		glUniformMatrix4fv(UniformMVP[VIEWPORT], 1, GL_FALSE, &MVP[0][0]);
		glUniform1i(UniformDiffuse, 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, TextureColorbufferName);

		glBindVertexArray(VertexArrayName[VIEWPORT]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[BUFFER_ELEMENT]);

		glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, NULL, 1, 0);
	}

	glf::checkError("display");
	glf::swapBuffers();
}

int main(int argc, char* argv[])
{
	if(glf::run(
		argc, argv,
		glm::ivec2(SAMPLE_SIZE_WIDTH, SAMPLE_SIZE_HEIGHT), 
		SAMPLE_MAJOR_VERSION, 
		SAMPLE_MINOR_VERSION))
		return 0;
	return 1;
}
