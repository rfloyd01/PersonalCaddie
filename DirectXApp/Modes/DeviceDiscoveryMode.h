#pragma once

#include "Mode.h"

//enum DeviceDiscoveryState
//{
//	DISCOVERY = 1, //the device watcher is on and newly found device will be displayed in the scroll box
//	ATTEMPT_CONNECT = 2, //attempt to connect to the BLE Device selected in the scroll box
//	CONNECTED = 4, //we're currently connected to a BLE Device
//	DISCONNECT = 8 //disconnect from the currently connected BLE Device
//};

class DeviceDiscoveryMode : public Mode
{
public:
	DeviceDiscoveryMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState = 0) override;
	virtual void uninitializeMode() override;

	virtual void handlePersonalCaddieConnectionEvent(bool connectionStatus) override;
	virtual void getBLEConnectionStatus(bool status) override;
	virtual void getBLEDeviceWatcherStatus(bool status) override;

	std::wstring getCurrentlySelectedDevice() { return m_currentlySelectedDeviceAddress; }

	virtual void getString(std::wstring message) override;

private:
	void initializeTextOverlay(winrt::Windows::Foundation::Size windowSize);
	virtual void uiElementStateChangeHandler(std::shared_ptr<ManagedUIElement> element) override;

	std::wstring m_currentlySelectedDeviceAddress = L"";
	bool m_connected, m_deviceWatcherActive;
};