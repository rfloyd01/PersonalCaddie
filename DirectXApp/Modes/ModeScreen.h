#pragma once

#include "Input/InputProcessor.h"
#include "Devices/PersonalCaddie.h"
#include "Mode.h"

/*
* The ModeScreen represents the main visual context of the application. The app has a number of
different modes that it can be operating in (such as Freeswing Mode, Settings Mode, Main Menu mode, etc.)
and this class acts as the display for whatever mode is currently being run. Apart from just displaying
visuals, this class controls the current state of the app and handles logic, timers, etc. Any input from the 
mouse or keyboard will also be seen here. There are also some helper enums/structs included here.
*/

//Keeps track of whether a Personal Caddie device is connected to the app via bluetooth.
//If there isn't one we are very limited in what we can actually do.
enum class ConnectionState
{
	Connected,
	NotConnected,
};

class MasterRenderer;

class ModeScreen
{
public:
	ModeScreen();

	void Initialize(
		_In_ std::shared_ptr<InputProcessor> const& input,
		_In_ std::shared_ptr<MasterRenderer> const& renderer
	);

	void setPersonalCaddie(_In_ std::shared_ptr<PersonalCaddie> const& pc);

	void update();

	const float* getBackgroundColor();

	//Handler Methods
	void PersonalCaddieAlertHandler(std::pair<std::wstring, TextTypeColorSplit> alert);

	std::shared_ptr<std::map<TextType, std::wstring> > getRenderText();
	std::shared_ptr<std::map<TextType, TextTypeColorSplit> > getRenderTextColors();

private:
	void processKeyboardInput(winrt::Windows::System::VirtualKey pressedKey);
	void processTimers();

	void changeCurrentMode(ModeType mt);

	

	std::pair<std::wstring, TextTypeColorSplit> getCurrentModeAlerts();
	void setCurrentModeAlerts(std::pair<std::wstring, TextTypeColorSplit> alerts);

	uint32_t                            m_modeState; //holds info on the current mode state

	std::shared_ptr<PersonalCaddie>     m_personalCaddie;
	std::shared_ptr<InputProcessor>     m_inputProcessor;
	std::shared_ptr<MasterRenderer>     m_renderer;

	//Modes
	std::vector<std::shared_ptr<Mode> > m_modes;
	ModeType                            m_currentMode;

	//Timing variables
	uint32_t alert_timer_duration; //duration of the alert timer in milliseconds
	bool alert_active;
	std::chrono::steady_clock::time_point alert_timer;
};