#pragma once

#include "Mode.h"

class TrainingMenuMode : public Mode
{
public:
	TrainingMenuMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState = 0) override;
	virtual void uninitializeMode() override;

	virtual void handleKeyPress(winrt::Windows::System::VirtualKey pressedKey) override;

	//Connection Handling Methods
	virtual void getBLEConnectionStatus(bool status) override;
	virtual void handlePersonalCaddieConnectionEvent(bool connectionStatus) override;

private:
	void initializeTextOverlay();
	void initializeToolTips();

	//Handler Methods
	virtual void uiElementStateChangeHandler(std::shared_ptr<ManagedUIElement> element) override;

	bool m_connected;
};