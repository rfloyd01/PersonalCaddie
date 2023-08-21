#include "pch.h"
#include "Main.h"

using namespace winrt;

using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::Graphics::Display;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Input;

struct App : implements<App, IFrameworkViewSource, IFrameworkView>
{
    IFrameworkView CreateView()
    {
        return *this;
    }

    void Initialize(CoreApplicationView const & applicationView)
    {
        //This is the very first method that gets called upon application launch. This is where we handle some of 
        //the most basic things that an app should be able to do, such as handling a suspension of the app (and 
        //a later resuming of it) by subscribing to events. We can also create device dependent graphics
        //resources here because we have access to the display adapter device.

        applicationView.Activated({ this, &App::OnActivated });

        CoreApplication::Suspending({ this, &App::OnSuspending });
        CoreApplication::Resuming({ this, &App::OnResuming });

        // At this point we have access to the device. 
        // We can create the device-dependent resources.
        m_deviceResources = std::make_shared<DX::DeviceResources>();
    }

    void Load(hstring const&)
    {
        //This is the third method to get called (right after SetWindow). This is a good place to load
        //resources specific to the application.
        if (!m_main)
        {
            m_main = winrt::make_self<Main>(m_deviceResources);
        }
    }

    void Uninitialize()
    {
        //When the application is closed this method gets called and we're given the chance to do any 
        //necessary cleanup.
    }

    void Run()
    {
        //Once all setup has been complete, the App::Run method is the last thing to be automatically envoked
        //(that is until we quit or suspent the app). This is the place to start the main application loop
        //where things like rendering and logic take place.
        m_main->Run();
    }

    void SetWindow(CoreWindow const & window)
    {
        //This gets called after the Initialize method and a CoreWindow object that represents the main window 
        //gets passed to this method. This is where we subscribe to window related events and can also configure some 
        //window and display behaviours. For example, we can construct a mouse pointer.
        window.PointerCursor(CoreCursor(CoreCursorType::Arrow, 0));

        PointerVisualizationSettings visualizationSettings{ PointerVisualizationSettings::GetForCurrentView() };
        visualizationSettings.IsContactFeedbackEnabled(false);
        visualizationSettings.IsBarrelButtonFeedbackEnabled(false);

        m_deviceResources->SetWindow(window);

        //Register Window Event Handlers
        window.Activated({ this, &App::OnWindowActivationChanged });
        window.SizeChanged({ this, &App::OnWindowSizeChanged });
        window.Closed({ this, &App::OnWindowClosed });
        window.VisibilityChanged({ this, &App::OnVisibilityChanged });

        DisplayInformation currentDisplayInformation{ DisplayInformation::GetForCurrentView() };
        currentDisplayInformation.DpiChanged({ this, &App::OnDpiChanged });
        currentDisplayInformation.OrientationChanged({ this, &App::OnOrientationChanged });

        DisplayInformation::DisplayContentsInvalidated({ this, &App::OnDisplayContentsInvalidated });
    }

    //Event Handlers
    void OnActivated(CoreApplicationView const& /* applicationView */, IActivatedEventArgs const&)
    {
        //After the App::Initialize, App::SetWindow and App::Load methods get called, the CoreApplicationView::Activated 
        //event gets raised. If an event handler was set for this event in the App::Initialize method (as is the case here)
        //then this is when it gets called. The only thing we do here is activate the main core window. This can also be 
        //done in the App::SetWindow method but since there are other resources we want to load it's deferred to this method.
        
        CoreWindow window = CoreWindow::GetForCurrentThread();
        window.Activate();
    }

    winrt::fire_and_forget OnSuspending(IInspectable const& /* sender */, SuspendingEventArgs const& args)
    {
        auto lifetime = get_strong();

        // Save app state asynchronously after requesting a deferral. Holding a deferral
        // indicates that the application is busy performing suspending operations. Be
        // aware that a deferral may not be held indefinitely. After about five seconds,
        // the app will be forced to exit.
        SuspendingDeferral deferral = args.SuspendingOperation().GetDeferral();

        co_await winrt::resume_background();

        m_deviceResources->Trim();

        m_main->Suspend();

        deferral.Complete();
    }

    void OnResuming(IInspectable const& /* sender */, IInspectable const& /* args */)
    {
        // Restore any data or state that was unloaded on suspend. By default, data
        // and state are persisted when resuming from suspend. Note that this event
        // does not occur if the app was previously terminated.
        m_main->Resume();
    }

    void OnVisibilityChanged(CoreWindow const& /* sender */, VisibilityChangedEventArgs const& args)
    {
        m_main->Visibility(args.Visible());
    }

    void App::OnWindowActivationChanged(CoreWindow const& /* sender */, WindowActivatedEventArgs const& args)
    {
        m_main->WindowActivationChanged(args.WindowActivationState());
    }

    void OnWindowSizeChanged(CoreWindow const& /* window */, WindowSizeChangedEventArgs const& args)
    {
        m_deviceResources->SetLogicalSize(args.Size());
        m_main->CreateWindowSizeDependentResources();
    }

    void OnWindowClosed(CoreWindow const& /* sender */, CoreWindowEventArgs const& /* args */)
    {
        m_main->Close();
    }

    void OnDpiChanged(DisplayInformation const& sender, IInspectable const& /* args */)
    {
        m_deviceResources->SetDpi(sender.LogicalDpi());
        m_main->CreateWindowSizeDependentResources();
    }

    void OnOrientationChanged(DisplayInformation const& sender, IInspectable const& /* args */)
    {
        m_deviceResources->SetCurrentOrientation(sender.CurrentOrientation());
        m_main->CreateWindowSizeDependentResources();
    }

    void OnDisplayContentsInvalidated(DisplayInformation const& /* sender */, IInspectable const& /* args */)
    {
        m_deviceResources->ValidateDevice();
    }

private:
    std::shared_ptr<DX::DeviceResources> m_deviceResources;
    winrt::com_ptr<Main> m_main;
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoreApplication::Run(make<App>());
}
