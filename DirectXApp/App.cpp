#include "pch.h"

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
    }

    void Load(hstring const&)
    {
    }

    void Uninitialize()
    {
    }

    void Run()
    {
        CoreWindow window = CoreWindow::GetForCurrentThread();
        window.Activate();

        CoreDispatcher dispatcher = window.Dispatcher();
        dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
    }

    void SetWindow(CoreWindow const & window)
    {

    }

    //Event Handlers
    void OnActivated(CoreApplicationView const& /* applicationView */, IActivatedEventArgs const&)
    {

    }
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoreApplication::Run(make<App>());
}
