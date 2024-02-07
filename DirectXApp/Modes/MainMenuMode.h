#pragma once

#include "Mode.h"

class MainMenuMode : public Mode
{
public:
	MainMenuMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState = 0) override;
	virtual void uninitializeMode() override;

	virtual void handleKeyPress(winrt::Windows::System::VirtualKey pressedKey) override;

private:
	void initializeTextOverlay();
	void initializeToolTips();

	//Handler Methods
	virtual void uiElementStateChangeHandler(std::shared_ptr<ManagedUIElement> element) override;
};