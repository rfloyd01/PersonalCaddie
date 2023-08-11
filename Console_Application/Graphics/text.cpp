#include "pch.h"

#include <Graphics/text.h>

Text::Text(std::string tex, float ex, float why, float sc, glm::vec3 col, float ep)
{
	text = tex;
	x = ex;
	x_end = ep;
	y = why;
	scale = sc;
	color = col;
}

MessageType mtFromInt(int m)
{
	return static_cast<MessageType>(m);
}