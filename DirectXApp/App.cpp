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
        //This is the third method to get called (right after SetWindow). This is a good place to load
        //resources specific to the application. 
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
        CoreWindow window = CoreWindow::GetForCurrentThread();
        window.Activate();

        CoreDispatcher dispatcher = window.Dispatcher();
        dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
    }

    void SetWindow(CoreWindow const & window)
    {
        //This gets called after the Initialize method and a CoreWindow object that represents the main window 
        //gets passed to this method. This is where we subscribe to window related events and can also configure some 
        //window and display behaviours. For example, we can construct a mouse pointer.
    }

    //Event Handlers
    void OnActivated(CoreApplicationView const& /* applicationView */, IActivatedEventArgs const&)
    {
        //After the App::Initialize, App::SetWindow and App::Load methods get called, the CoreApplicationView::Activated 
        //event gets raised. If an event handler was set for this event in the App::Initialize method (as is the case here)
        //then this is when it gets called. The only thing we do here is activate the main core window. This can also be 
        //done in the App::SetWindow method but since there are other resources we want to load it's deferred to this method.
        
        //CoreWindow window = CoreWindow::GetForCurrentThread();
        //window.Activate();
    }

private:
    /*std::shared_ptr<DX::*/
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoreApplication::Run(make<App>());
}
