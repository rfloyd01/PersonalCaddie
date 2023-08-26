#pragma once

#include <string>

enum class TextType
{
	//this enum class is used to group different messages displayed on screen
	TITLE = 0,
	SUB_TITLE = 1,
	BODY = 2,
	SENSOR_INFO = 3,
	FOOT_NOTE = 4,
	ALERT = 5,
	END = 6  //Used to mark the end of the enum class for potentially looping through
};

struct TextColor
{
	float r;
	float g;
	float b;
	float a;
};

//This struct holds information about text to be rendered on the screen
struct Text
{
	Text(std::wstring const& message, std::vector<TextColor> const& colors, std::vector<unsigned long long> const & locations, TextType textType) :
		message(message),
		colors(colors),
		locations(locations),
		textType(textType)
	{

	}

	std::wstring message;
	std::vector<TextColor> colors; //holds the different colors of the text (in order)
	std::vector<unsigned long long> locations; //holds the index of all characters where the text color switches. This vector must be 1 element longer than the colors vector
	TextType textType;
};