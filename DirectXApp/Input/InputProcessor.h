#pragma once

/*
* This class handles all input from the keyboard and mouse. This processed input then gets
* transferred to other classes for use.
*/

//Define a random key to signify no keyboard key is actually being pressed
namespace KeyboardKeys
{
    const static winrt::Windows::System::VirtualKey DeadKey = winrt::Windows::System::VirtualKey::F12;
}


//This enum class tells us the current state of the InputProcessor. When in the none state 
//we are currently accepting any input. When in the WaitForInput state we are currently waiting 
//for a mouse click or keyboard press to process. In the Active state we are currently processing 
//a keyboard or mouse click so any other input will be ignored.
enum class KeyboardState
{
	None,
	WaitForInput,
	KeyPressed,
};

//An update package that is sent to the rest of the application on keypress or 
//mouse move/click
struct InputState
{
    winrt::Windows::System::VirtualKey currentPressedKey;
    //TODO: add mouse variables at some point in the future
};

class InputProcessor
{
public:
	InputProcessor(_In_ winrt::Windows::UI::Core::CoreWindow const& window);

	void InitWindow(_In_ winrt::Windows::UI::Core::CoreWindow const& window);

    void setKeyboardState(KeyboardState ks);

    InputState* update();

private:
    void ResetState();

    /*void OnPointerPressed(
        _In_ winrt::Windows::UI::Core::CoreWindow const& sender,
        _In_ winrt::Windows::UI::Core::PointerEventArgs const& args
    );
    void OnPointerMoved(
        _In_ winrt::Windows::UI::Core::CoreWindow const& sender,
        _In_ winrt::Windows::UI::Core::PointerEventArgs const& args
    );
    void OnPointerReleased(
        _In_ winrt::Windows::UI::Core::CoreWindow const& sender,
        _In_ winrt::Windows::UI::Core::PointerEventArgs const& args
    );
    void OnPointerExited(
        _In_ winrt::Windows::UI::Core::CoreWindow const& sender,
        _In_ winrt::Windows::UI::Core::PointerEventArgs const& args
    );*/
    void OnKeyDown(
        _In_ winrt::Windows::UI::Core::CoreWindow const& sender,
        _In_ winrt::Windows::UI::Core::KeyEventArgs const& args
    );
    void OnKeyUp(
        _In_ winrt::Windows::UI::Core::CoreWindow const& sender,
        _In_ winrt::Windows::UI::Core::KeyEventArgs const& args
    );
    /*void OnMouseMoved(
        _In_ winrt::Windows::Devices::Input::MouseDevice const& mouseDevice,
        _In_ winrt::Windows::Devices::Input::MouseEventArgs const& args
    );
    void OnBackRequested(
        _In_ winrt::Windows::Foundation::IInspectable const& sender,
        _In_ winrt::Windows::UI::Core::BackRequestedEventArgs const& args
    );*/

    InputState                              m_state;
    KeyboardState                           m_keyboardState; // Enum to keep track of whether or not keyboard presses are allowed

    bool newInput = false;

};