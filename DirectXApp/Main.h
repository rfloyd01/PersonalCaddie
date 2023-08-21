#pragma once

#include "Graphics/Rendering/MasterRenderer.h"

class Main : public winrt::implements<Main, winrt::Windows::Foundation::IInspectable>, DX::IDeviceNotify
{
public:
    Main(std::shared_ptr<DX::DeviceResources> const& deviceResources);
    ~Main();

    void CreateWindowSizeDependentResources();
    void Run();
    void Suspend();
    void Resume();
	void Visibility(bool visible);
    void Close() { m_windowClosed = true; }

    // IDeviceNotify
    virtual void OnDeviceLost();
    virtual void OnDeviceRestored();

    void WindowActivationChanged(winrt::Windows::UI::Core::CoreWindowActivationState activationState);

private:
    //winrt::fire_and_forget HandleDeviceRestored();

    bool                                                m_windowClosed;
    bool                                                m_pauseRequested;
    bool                                                m_pressComplete;
    bool                                                m_renderNeeded;
    bool                                                m_haveFocus;
    bool                                                m_visible;

    std::shared_ptr<DX::DeviceResources>                m_deviceResources;

    std::shared_ptr<MasterRenderer>                     m_renderer;

    uint32_t                                            m_loadingCount;
};