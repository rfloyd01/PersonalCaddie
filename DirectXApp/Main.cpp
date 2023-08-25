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
    //First create a Personal Caddie instance
    m_personalCaddie = std::make_shared<PersonalCaddie>();

    //Then, pass class pointers to the ModeScreen class (this is really the main 
    //state of the application) and load up the main menu
    m_modeScreen->Initialize(m_personalCaddie, m_inputProcessor, m_renderer);

    //With the main menu mode loaded we can now activate the keyboard
    //for input processing. Also alert the user to the fact that we're
    //currently trying to connect to a personal caddie
    auto yeet = L"Currently searching for a Personal Caddie device.";
    m_inputProcessor->setKeyboardState(KeyboardState::WaitForInput);
    m_modeScreen->setCurrentModeAlerts({ L"Currently searching for a Personal Caddie device.", {{ {1.0, 0.788, 0.055, 0.75} }, {0, 50} }});

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