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

//Forward Declarations
class MasterRenderer;

class ModeScreen
{
public:
	//PUBLIC METHODS AND VARIABLES

	//Initialization Methods
	ModeScreen();
	winrt::fire_and_forget Initialize(
		_In_ std::shared_ptr<InputProcessor> const& input,
		_In_ std::shared_ptr<MasterRenderer> const& renderer
	);
	void setPersonalCaddie(_In_ std::shared_ptr<PersonalCaddie> const& pc);

	//Updating Methods
	void update();

	//Handler Methods
	void PersonalCaddieHandler(PersonalCaddieEventType pcEvent, void* eventArgs);
	void ModeHandler(ModeAction action, void* eventArgs);

	//Rendering Methods
	const UIColor getBackgroundColor();
	std::map<UIElementType, std::vector<std::shared_ptr<ManagedUIElement> > > const& getCurrentModeUIElementMap();
	void resizeCurrentModeUIElements(winrt::Windows::Foundation::Size windowSize);
	void createAlert(std::wstring message, UIColor color);
	bool needs3DRendering();
	Camera& getCamera() { return m_camera; }
	std::vector<std::shared_ptr<VolumeElement> > const& getCurrentModeVolumeElements();

private:
	//PRIVATE METHODS AND VARIABLES

	//Updating Methods
	void processKeyboardInput(InputState* inputState);
	void processMouseInput(InputState* inputState);
	void processEvents();

	//Mode Methods
	void changeCurrentMode(ModeType mt);
	std::shared_ptr<Mode> getCurrentMode() { return m_modes[static_cast<int>(m_currentMode)]; }

	//Rendering Methods
	void getTextRenderPixels(std::vector<UIText*> const& text);

	uint32_t                                m_modeState; //holds info on the current mode state

	std::shared_ptr<PersonalCaddie>         m_personalCaddie;
	std::shared_ptr<InputProcessor>         m_inputProcessor;
	std::shared_ptr<MasterRenderer>         m_renderer;

	//Modes
	std::vector<std::shared_ptr<Mode> >     m_modes;
	ModeType                                m_currentMode;

	//Rendering variables
	Camera                                  m_camera; //a camera for rendering 3D scenes of certain modes
	std::vector<std::shared_ptr<Material> > m_materials; //materials used for rendering 3D objects

	//Input Variables
	DirectX::XMFLOAT2                       m_previousMousePosition; //let's us know if the mouse has moved since the last frame was rendered

	//Event variables
	volatile PersonalCaddieEventType personal_caddy_event; //let's the main thread know that a new alert has been created from a different thread and needs to be rendered

	//Timing variables
	bool alert_active, button_pressed;
	uint32_t alert_timer_duration, button_pressed_duration; //duration of the alert timer in milliseconds
	std::chrono::steady_clock::time_point alert_timer, button_pressed_timer;
};