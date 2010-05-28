#version 330 core

// Declare all the semantics
#define ATTR_POSITION	0
#define ATTR_COLOR		3
#define ATTR_TEXCOORD	4
#define VERT_POSITION	0
#define VERT_COLOR		3
#define VERT_TEXCOORD	4
#define FRAG_COLOR		0
#define FRAG_RED		0
#define FRAG_GREEN		1
#define FRAG_BLUE		2
#define FRAG_ALPHA		3

/*layout(location = VERT_COLOR)*/ in vec4 VertColor;
layout(location = FRAG_COLOR, index = 0) out vec4 Color;

void main()
{
	Color = VertColor;
}
