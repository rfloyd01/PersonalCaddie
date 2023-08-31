#include "pch.h"
#include "UIElementColors.h"

UIElementColors::UIElementColors()
{
	colors.insert({ UIColor::Black, {0.0, 0.0, 0.0} });
	colors.insert({ UIColor::White, {1.0, 1.0, 1.0} });
	colors.insert({ UIColor::Red, {1.0, 0.0, 0.0} });
	colors.insert({ UIColor::Green, {0.0, 1.0, 0.0} });
	colors.insert({ UIColor::Blue, {0.0, 0.0, 1.0} });
	colors.insert({ UIColor::LightBlue, {0.002, 0.894, 0.655} });
	colors.insert({ UIColor::Yellow, {1.0, 0.788, 0.055} });
	colors.insert({ UIColor::PaleGray, {0.75, 0.75, 0.75} });
	colors.insert({ UIColor::Gray, {0.5, 0.5, 0.5} });
	colors.insert({ UIColor::DarkGray, {0.25, 0.25, 0.25} });

	colors.insert({ UIColor::ButtonPressed, {0.35, 0.35, 0.35} });
	colors.insert({ UIColor::ButtonNotPressed, {0.9, 0.9, 0.9} });

	colors.insert({ UIColor::FreeSwingMode, { 0.39, 0.592, 0.592} });
	colors.insert({ UIColor::SwingAnalysisMode, { 0.58, 0.93, 0.588, 1 } });
	colors.insert({ UIColor::TrainingMode, { 0.71, 0.541, 0.416, 1 } });
	colors.insert({ UIColor::CalibrationMode, { 0.498, 0.498, 0.498, 1 } });

}