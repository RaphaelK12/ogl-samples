#version 410 core

precision mediump int;

// Declare all the semantics
#define ATTR_POSITION	0
#define ATTR_COLOR		3
#define ATTR_TEXCOORD	4
#define VERT_POSITION	0
#define VERT_COLOR		3
#define VERT_TEXCOORD	4
#define VERT_INSTANCE	7
#define GEOM_TEXCOORD	4
#define GEOM_INSTANCE	7
#define FRAG_COLOR		0
#define FRAG_RED		0
#define FRAG_GREEN		1
#define FRAG_BLUE		2
#define FRAG_ALPHA		3

uniform sampler2DArray Diffuse;

layout(location = GEOM_TEXCOORD) in vec2 Texcoord;
layout(location = GEOM_INSTANCE) in int Instance;
layout(location = FRAG_COLOR, index = 0) out vec4 Color;

void main()
{
	Color = texture(Diffuse, vec3(Vert.Texcoord, Instance));
}
