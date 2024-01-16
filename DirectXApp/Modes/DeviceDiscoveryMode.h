#pragma once

#include "Mode.h"

class DeviceDiscoveryMode : public Mode
{
public:
	DeviceDiscoveryMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState = 0) override;
	virtual void uninitializeMode() override;

	virtual void handleKeyPress(winrt::Windows::System::VirtualKey pressedKey) override;

	virtual void handlePersonalCaddieConnectionEvent(bool connectionStatus) override;
	virtual void getBLEConnectionStatus(bool status) override;
	virtual void getBLEDeviceWatcherStatus(bool status) override;

	std::wstring getCurrentlySelectedDevice() { return m_currentlySelectedDeviceAddress; }

	virtual void getString(std::wstring message) override;

private:
	void initializeTextOverlay();
	virtual void uiElementStateChangeHandler(std::shared_ptr<ManagedUIElement> element) override;

	std::wstring m_currentlySelectedDeviceAddress = L"";
	bool m_connected, m_deviceWatcherActive;
};