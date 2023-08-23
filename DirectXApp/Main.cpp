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

    m_renderer = std::make_shared<MasterRenderer>(m_deviceResources);
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

    while (!m_windowClosed)
    {
        //TODO: Update as the app gets more complex
        if (m_visible)
        {
            CoreWindow::GetForCurrentThread().Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
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
    //TODO: Update this as the app gets more complicated
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