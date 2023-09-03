#pragma once

#include <string>

enum class UIColor;

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

//An enum describing the location inside of a UI element that text should be rendered
enum class UITextJustification
{
	UpperLeft = 0,
	UpperCenter = 1,
	UpperRight = 2,
	CenterLeft = 3,
	CenterCenter = 4,
	CenterRight = 5,
	LowerLeft = 6,
	LowerCenter = 7,
	LowerRight = 8,
	END = 9
};

//This struct holds information about text to be rendered on the screen
struct UIText
{
	UIText(std::wstring const& message, float fontSize, DirectX::XMFLOAT2 startLocation, DirectX::XMFLOAT2 renderArea,
		std::vector<UIColor> const& colors, std::vector<unsigned long long> const & colorLocations, UITextType textType,
		UITextJustification justification = UITextJustification::UpperLeft) :
		message(message),
		fontSize(fontSize),
		startLocation(startLocation),
		renderArea(renderArea),
		colors(colors),
		colorLocations(colorLocations),
		textType(textType),
		justification(justification)
	{

	}

	UIText() { textType = UITextType::END; }; //this tells other parts of the program that this UIText hasn't been initialized yet

	std::wstring message;
	DirectX::XMFLOAT2 startLocation; //the top left corner of the rendering box for the text (the size of the render box is calculated elsewhere)
	DirectX::XMFLOAT2 renderArea; //the size of the rendering area for the text
	float fontSize;
	std::vector<UIColor> colors; //holds the different colors of the text (in order)
	std::vector<unsigned long long> colorLocations; //holds the index of all characters where the text color switches. This vector must be 1 element longer than the colors vector
	UITextType  textType;
	UITextJustification justification;

	//for some text (like in scroll boxes) we need to know the height of the textlayout in pixels to know when
	//we've reached the bottom of the text. By daefulat the needDPIHeight bool is set to false. Setting it to
	//true will cause the modeScreen class to get the text layout height from the master renderer and store it
	//int the renderingHeightDPI float. 
	bool needDPI = false;
	DirectX::XMFLOAT2A renderDPI = { 0, 0 };
};