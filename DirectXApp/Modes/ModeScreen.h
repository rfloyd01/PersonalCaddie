#pragma once

#include "Input/InputProcessor.h"
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

//The ModeState enum class keeps track of what the current state of the app is. When idle,
//nothing is happening except for displaying text or a static 3D model. In active mode,
//we are taking real time data from the Personal Caddie to render a moving golf club on
//the screen. In recording mode we're saving data from the Personal Caddie for either 
//calibration or creating graphs.
enum class ModeState
{
	Idle,
	Active,
	Recording,
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

	std::shared_ptr<std::map<TextType, std::wstring> > getRenderText();
	std::shared_ptr<std::map<TextType, TextTypeColorSplit> > getRenderTextColors();

private:
	std::shared_ptr<InputProcessor>     m_inputProcessor;
	std::shared_ptr<MasterRenderer>     m_renderer;

	//Modes
	std::vector<std::shared_ptr<Mode> > m_modes;
	ModeType                            m_currentMode;
};