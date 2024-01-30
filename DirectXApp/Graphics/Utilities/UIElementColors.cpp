#include "pch.h"
#include "UIElementColors.h"

UIElementColors::UIElementColors()
{
	colors.insert({ UIColor::Black, {0.0f, 0.0f, 0.0f, 1.0f} });
	colors.insert({ UIColor::White, {1.0f, 1.0f, 1.0f, 1.0f} });
	colors.insert({ UIColor::Red, {1.0f, 0.0f, 0.0f, 1.0f} });
	colors.insert({ UIColor::Green, {0.0f, 1.0f, 0.0f, 1.0f} });
	colors.insert({ UIColor::Blue, {0.0f, 0.0f, 1.0f, 1.0f} });
	colors.insert({ UIColor::LightBlue, {0.002f, 0.894f, 0.655f, 1.0f} });
	colors.insert({ UIColor::Yellow, {1.0f, 0.788f, 0.055f, 1.0f} });
	colors.insert({ UIColor::Magenta, {0.749f, 0.0f, 0.114f, 1.0f} });
	colors.insert({ UIColor::Cyan, {0.0f, 0.835f, 0.835f, 1.0f} });
	colors.insert({ UIColor::Orange, {1.0f, 0.502f, 0.0f, 1.0f} });
	colors.insert({ UIColor::Purple, {0.0f, 0.502f, 1.0f, 1.0f} });
	colors.insert({ UIColor::Mint, {0.0f, 1.0f, 0.502f, 1.0f} });
	colors.insert({ UIColor::Pink, {1.0f, 0.0f, 0.502f, 1.0f} });
	colors.insert({ UIColor::PaleGray, {0.75f, 0.75f, 0.75f, 1.0f} });
	colors.insert({ UIColor::Gray, {0.5f, 0.5f, 0.5f, 1.0f} });
	colors.insert({ UIColor::DarkGray, {0.25f, 0.25f, 0.25f, 1.0f} });

	colors.insert({ UIColor::ButtonPressed, {0.35f, 0.35f, 0.35f, 1.0f} });
	colors.insert({ UIColor::ButtonNotPressed, {0.9f, 0.9f, 0.9f, 1.0f} });

	colors.insert({ UIColor::FreeSwingMode, { 0.39f, 0.592f, 0.592f, 1.0f} });
	colors.insert({ UIColor::SwingAnalysisMode, { 0.58f, 0.93f, 0.588f, 1.0f } });
	colors.insert({ UIColor::TrainingMode, { 0.71f, 0.541f, 0.416f, 1.0f } });
	colors.insert({ UIColor::CalibrationMode, { 0.498f, 0.498f, 0.498f, 1.0f } });

	colors.insert({ UIColor::OpaqueBlue, { 0.6f, 0.851f, 0.918f, 0.5f } });

	//Assert that the number of colors in the color map is identical to the amount of 
	//colors in the UIColor::Enum. If there isn't it will cause issues in the creation
	//of some direct2D resources and cause the program to crash anyway so may as well
	//crash it here with an error message as to why
	assert(("The number of UIColors in the color map doesn't match the number of UIColors in the enum", colors.size() == static_cast<int>(UIColor::END)));
}