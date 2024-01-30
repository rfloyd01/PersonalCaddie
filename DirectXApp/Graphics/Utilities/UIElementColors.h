#pragma once

#include <map>

//This file is meant to help control all colors that can be used for rendering
//UI Elements. All of the colors are created in the UIElementColors class and 
//are tied to the UIElementColor enum class.

//Note: When a new color is needed it needs to be added to both the UIColor enum class
//and the map of the UIElementColors class
enum class UIColor
{
	//SOLID COLORS
	//-------------
	//Standard Colors
	Black,
	White,
	Red,
	LightBlue,
	Blue,
	Green,
	Yellow,
	Magenta,
	Cyan,
	Orange,
	Purple,
	Mint,
	Pink,
	PaleGray,
	Gray,
	DarkGray,

	//UI Element Specific Colors
	ButtonPressed,
	ButtonNotPressed,

	//Mode Specific Colors
	FreeSwingMode,
	SwingAnalysisMode,
	TrainingMode,
	CalibrationMode,

	//OPAQUE COLORS
	//-------------
	OpaqueBlue,

	END
};

struct UIElementColors
{
	UIElementColors();
	
	std::map<UIColor, D2D1::ColorF> colors;
};