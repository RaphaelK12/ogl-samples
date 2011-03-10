//**********************************
// OpenGL Image texture
// 09/07/2010 - 10/03/2011
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
	std::string const SAMPLE_NAME = "OpenGL Image texture";
	std::string const VERTEX_SHADER_SOURCE(glf::DATA_DIRECTORY + "410/image-texture.vert");
	std::string const FRAGMENT_SHADER_SOURCE(glf::DATA_DIRECTORY + "410/image-texture.frag");
	std::string const TEXTURE_DIFFUSE(glf::DATA_DIRECTORY + "kueken256-rgba8.dds");
	int const SAMPLE_SIZE_WIDTH = 640;
	int const SAMPLE_SIZE_HEIGHT = 480;
	int const SAMPLE_MAJOR_VERSION = 4;
	int const SAMPLE_MINOR_VERSION = 1;

	struct vertex
	{
		vertex
		(
			glm::vec2 const & Position,
			glm::vec2 const & Texcoord
		) :
			Position(Position),
			Texcoord(Texcoord)
		{}

		glm::vec2 Position;
		glm::vec2 Texcoord;
	};

	GLsizei const VertexCount = 4;
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f))
	};

	GLsizei const ElementCount = 6;
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
	GLushort const ElementData[ElementCount] =
	{
		0, 1, 2, 
		2, 3, 0
	};

	glf::window Window(glm::ivec2(SAMPLE_SIZE_WIDTH, SAMPLE_SIZE_HEIGHT));
	glm::uvec2 ImageSize(0);

	namespace buffer
	{
		enum type
		{
			VERTEX,
			ELEMENT,
			MAX
		};
	}//namespace buffer

	GLuint VertexArrayName(0);
	GLuint PipelineName(0);
	GLuint ProgramName(0);

	GLuint BufferName[buffer::MAX] = {0, 0};
	GLuint Image2DName(0);
	GLuint SamplerName(0);

	GLuint UniformMVP(0);
	GLuint UniformImageData(0);
	GLuint UniformImageSize(0);
}//namespace

bool initProgram()
{
	bool Validated = true;

	if(Validated)
	{
		GLuint VertexShaderName = glf::createShader(GL_VERTEX_SHADER, VERTEX_SHADER_SOURCE);
		GLuint FragmentShaderName = glf::createShader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_SOURCE);

		ProgramName = glCreateProgram();
		glAttachShader(ProgramName, VertexShaderName);
		glAttachShader(ProgramName, FragmentShaderName);
		glDeleteShader(VertexShaderName);
		glDeleteShader(FragmentShaderName);
		glLinkProgram(ProgramName);
		Validated = glf::checkProgram(ProgramName);
	}

	if(Validated)
	{
		UniformMVP = glGetUniformLocation(ProgramName, "MVP");
		UniformImageData = glGetUniformLocation(ProgramName, "ImageData");
		UniformImageSize = glGetUniformLocation(ProgramName, "ImageSize");
	}

	return Validated && glf::checkError("initProgram");
}

bool initArrayBuffer()
{
	glGenBuffers(buffer::MAX, BufferName);
	glNamedBufferDataEXT(BufferName[buffer::VERTEX], VertexSize, VertexData, GL_STATIC_DRAW);
	glNamedBufferDataEXT(BufferName[buffer::ELEMENT], ElementSize, ElementData, GL_STATIC_DRAW);

	return glf::checkError("initArrayBuffer");;
}

bool initTexture2D()
{
	glGenTextures(1, &Image2DName);

	glTextureParameteriEXT(Image2DName, GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTextureParameteriEXT(Image2DName, GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1000);
	glTextureParameteriEXT(Image2DName, GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
	glTextureParameteriEXT(Image2DName, GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
	glTextureParameteriEXT(Image2DName, GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
	glTextureParameteriEXT(Image2DName, GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
	glTextureParameteriEXT(Image2DName, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteriEXT(Image2DName, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	gli::texture2D Image = gli::load(TEXTURE_DIFFUSE);
	for(std::size_t Level = 0; Level < Image.levels(); ++Level)
	{
		glTexImage2D(
			GL_TEXTURE_2D, 
			GLint(Level), 
			GL_RGBA32F, 
			GLsizei(Image[Level].dimensions().x), 
			GLsizei(Image[Level].dimensions().y), 
			0,  
			GL_BGR, 
			GL_UNSIGNED_BYTE, 
			Image[Level].data());
	}
	ImageSize = glm::uvec2(Image[0].dimensions());

	return glf::checkError("initTexture2D");
}

bool initVertexArray()
{
	glGenVertexArrays(1, &VertexArrayName);

	glVertexArrayVertexAttribOffsetEXT(VertexArrayName, BufferName[buffer::VERTEX], glf::semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
	glVertexArrayVertexAttribOffsetEXT(VertexArrayName, BufferName[buffer::VERTEX], glf::semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), sizeof(glm::vec2));

	glEnableVertexArrayAttribEXT(VertexArrayName, glf::semantic::attr::POSITION);
	glEnableVertexArrayAttribEXT(VertexArrayName, glf::semantic::attr::TEXCOORD);

	return glf::checkError("initVertexArray");
}

bool begin()
{
	bool Validated = true;
	Validated = Validated && glf::checkGLVersion(SAMPLE_MAJOR_VERSION, SAMPLE_MINOR_VERSION);
	Validated = Validated && glf::checkExtension("GL_EXT_direct_state_access");
	Validated = Validated && glf::checkExtension("GL_EXT_shader_image_load_store");

	if(Validated)
		Validated = initProgram();
	if(Validated)
		Validated = initArrayBuffer();
	if(Validated)
		Validated = initTexture2D();
	if(Validated)
		Validated = initVertexArray();

	return Validated && glf::checkError("begin");
}

bool end()
{
	glDeleteBuffers(buffer::MAX, BufferName);
	glDeleteProgram(ProgramName);
	glDeleteTextures(1, &Image2DName);
	glDeleteSamplers(1, &SamplerName);
	glDeleteVertexArrays(1, &VertexArrayName);
	glDeleteProgramPipelines(1, &PipelineName);

	return glf::checkError("end");
}

void display()
{
	// Compute the MVP (Model View Projection matrix)
	glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Window.TranlationCurrent.y));
	glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Window.RotationCurrent.y, glm::vec3(1.f, 0.f, 0.f));
	glm::mat4 View = glm::rotate(ViewRotateX, Window.RotationCurrent.x, glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 MVP = Projection * View * Model;

	glProgramUniformMatrix4fv(ProgramName, UniformMVP, 1, GL_FALSE, &MVP[0][0]);
	glProgramUniform1i(ProgramName, UniformImageData, 0);
	glProgramUniform2uiv(ProgramName, UniformImageSize, 1, &ImageSize[0]);

	glViewport(0, 0, Window.Size.x, Window.Size.y);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

	glUseProgram(ProgramName);

	glViewport(0, 0, Window.Size.x, Window.Size.y);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindImageTextureEXT(0, Image2DName, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	glBindVertexArray(VertexArrayName);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);

	glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, NULL, 1, 0);

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