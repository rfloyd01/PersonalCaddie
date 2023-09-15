#pragma once

#include "Input/InputProcessor.h"
#include "Devices/PersonalCaddie.h"
#include "Mode.h"
#include "Graphics/Objects/3D/Camera.h"

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

	const UIColor getBackgroundColor();

	//Handler Methods
	void PersonalCaddieHandler(PersonalCaddieEventType pcEvent, void* eventArgs);

	//2D Rendering Methods
	std::vector<std::shared_ptr<UIElement> > const& getCurrentModeUIElements();
	void resizeCurrentModeUIElements(winrt::Windows::Foundation::Size windowSize);

	void createAlert(std::wstring message, UIColor color);

	//3D Rendering Methods
	bool needs3DRendering();
	DirectX::XMMATRIX getCameraView() { return m_camera.View(); };

	std::vector<std::shared_ptr<VolumeElement> > const& getCurrentModeVolumeElements();

private:
	void processKeyboardInput(winrt::Windows::System::VirtualKey pressedKey);
	void processMouseInput(InputState* inputState);
	void processEvents();
	void processTimers();

	void changeCurrentMode(ModeType mt);

	void getTextRenderPixels(std::vector<UIText*> const& text);

	void enterActiveState(); //depending on the current mode, something different will happen here
	void stateUpdate();
	void leaveActiveState();

	uint32_t                            m_modeState; //holds info on the current mode state

	std::shared_ptr<PersonalCaddie>     m_personalCaddie;
	std::shared_ptr<InputProcessor>     m_inputProcessor;
	std::shared_ptr<MasterRenderer>     m_renderer;

	//Modes
	std::vector<std::shared_ptr<Mode> > m_modes;
	ModeType                            m_currentMode;

	Camera                              m_camera; //a camera for rendering 3D scenes of certain modes

	//Event variables
	volatile PersonalCaddieEventType personal_caddy_event; //let's the main thread know that a new alert has been created from a different thread and needs to be rendered

	//Timing variables
	bool alert_active, button_pressed;
	uint32_t alert_timer_duration, button_pressed_duration; //duration of the alert timer in milliseconds
	std::chrono::steady_clock::time_point alert_timer, button_pressed_timer;
};