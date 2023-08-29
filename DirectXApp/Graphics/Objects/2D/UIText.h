#pragma once

#include <string>

enum class UITextType
{
	//this enum class is used to group different messages displayed on screen
	TITLE = 0,
	SUB_TITLE = 1,
	BODY = 2,
	SENSOR_INFO = 3,
	FOOT_NOTE = 4,
	ALERT = 5,
	ELEMENT_TEXT = 6,
	END = 7  //Used to mark the end of the enum class for potentially looping through
};

//This struct holds information about text to be rendered on the screen
struct UIText
{
	UIText(std::wstring const& message, std::vector<D2D1_COLOR_F> const& colors, std::vector<unsigned long long> const & colorLocations, UITextType textType) :
		message(message),
		colors(colors),
		colorLocations(colorLocations),
		textType(textType)
	{

	}

	std::wstring message;
	std::vector<D2D1_COLOR_F> colors; //holds the different colors of the text (in order)
	std::vector<unsigned long long> colorLocations; //holds the index of all characters where the text color switches. This vector must be 1 element longer than the colors vector
	UITextType  textType;
};