#pragma once

#include "Mode.h"

class SettingsMenuMode : public Mode
{
public:
	SettingsMenuMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState = 0) override;
	virtual void uninitializeMode() override;

	virtual void handleKeyPress(winrt::Windows::System::VirtualKey pressedKey) override;

	//Connection Handling Methods
	virtual void getBLEConnectionStatus(bool status) override;
	virtual void handlePersonalCaddieConnectionEvent(bool connectionStatus) override;

private:
	void initializeTextOverlay(winrt::Windows::Foundation::Size windowSize);

	bool m_connected;
};