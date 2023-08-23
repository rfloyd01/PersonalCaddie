#pragma once

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
	Text(wchar_t const* message, float x, float y, const TextColor& color) :
		message(message),
		x(x),
		y(y),
		color(color)
	{

	}

	int getMessageLength()
	{
		int length = 0;
		auto it = message;

		while (*it++ != L'\0') length++;

		return length;
	}

	wchar_t const* message;
	float x;
	float y;
	TextColor color;
};