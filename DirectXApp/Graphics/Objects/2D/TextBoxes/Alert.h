#pragma once

#include "Graphics/Objects/2D/BasicElements/TextOverlay.h"
#include <chrono>

//The alert class is nothing more than a TextOverlay which keeps
//its own timer. When the timer expires the alert gets automatically
//deleted. Unlike other UI Elements, alerts persist between different
//modes. Because of this, the alert isn't responsible for deleting
//itslef, this instead falls on the UIElementManager class to deal with.

class Alert : public TextOverlay
{
public:
	Alert(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring message,
		float fontSize, std::vector<UIColor> colors, std::vector<unsigned long long> colorLocations, UITextJustification justification, long long time = 2500)
		: TextOverlay(windowSize, location, size, message, fontSize, colors, colorLocations, justification)
	{
		m_duration = time;
		m_startTime = std::chrono::steady_clock::now();
	}

	Alert() {} //empty default constructor

	long long m_duration; //the duration of the alert in milliseconds, default value is 2500
	std::chrono::steady_clock::time_point m_startTime; //A time stamp of when the alert first goes active
};