#include "pch.h"
#include "Main.h"

#include <iostream>

using namespace DirectX;
using namespace winrt::Windows::UI::Core;

Main::Main(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_windowClosed(false),
    m_haveFocus(false),
    m_visible(true)
{
    m_deviceResources->RegisterDeviceNotify(this);

    m_modeScreen = std::make_shared<ModeScreen>();
    m_renderer = std::make_shared<MasterRenderer>(m_deviceResources, m_modeScreen);
    m_inputProcessor = std::make_shared<InputProcessor>(CoreWindow::GetForCurrentThread());
}

Main::~Main()
{
    // Deregister device notification
    m_deviceResources->RegisterDeviceNotify(nullptr);
}

void Main::Run()
{
    //First, create the main app instance (the mode screen class) and pass in pointers
    //to the input processor and the master renderer.
    m_modeScreen->Initialize(m_inputProcessor, m_renderer);

    //Then create a Personal Caddie instance and pass a handler function from the Mode
    //screen class for receiving BLE events
    m_personalCaddie = std::make_shared<PersonalCaddie>(std::bind(&ModeScreen::PersonalCaddieHandler, m_modeScreen.get(), std::placeholders::_1, std::placeholders::_2));

    //After successfully creating the Personal Caddie instance, pass a pointer
    //of it to the Mode Screen app as well so that the Personal Caddie can send
    //the necessary data to be rendered.
    m_modeScreen->setPersonalCaddie(m_personalCaddie);

    //With the main menu mode loaded we can now activate the keyboard
    //for input processing. Also alert the user to the fact that we're
    //currently trying to connect to a personal caddie
    m_inputProcessor->setKeyboardState(KeyboardState::WaitForInput);
    m_inputProcessor->setMouseState(MouseClickState::WaitForInput);

    while (!m_windowClosed)
    {
        if (m_visible)
        {
            CoreWindow::GetForCurrentThread().Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
            m_modeScreen->update();
            m_renderer->Render();
            m_deviceResources->Present();
        }
        else
        {
            CoreWindow::GetForCurrentThread().Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
        }

    }
}

void Main::CreateWindowSizeDependentResources()
{
    m_renderer->CreateWindowSizeDependentResources();
    m_modeScreen->resizeCurrentModeUIElements(m_renderer->getCurrentScreenSize());
}

void Main::WindowActivationChanged(CoreWindowActivationState activationState)
{
    if (activationState == CoreWindowActivationState::Deactivated)
    {
        m_haveFocus = false;

        //TODO: Add stuff here later
    }
    else if (activationState == CoreWindowActivationState::CodeActivated || activationState == CoreWindowActivationState::PointerActivated)
    {
        m_haveFocus = true;

        //TODO: Add stuff here later
    }
}

void Main::OnDeviceLost()
{
    //TODO: Add stuff here later
}

void Main::OnDeviceRestored()
{
    //HandleDeviceRestored();
}

void Main::Suspend()
{
    //TODO: Add stuff here later
}

void Main::Resume()
{
    //TODO: Add stuff here later
}

void Main::Visibility(bool visible)
{
    if (visible && !m_visible)
    {
        m_renderNeeded = true;
    }
    m_visible = visible;
}