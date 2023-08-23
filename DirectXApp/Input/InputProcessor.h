#pragma once

/*
* This class handles all input from the keyboard and mouse. This processed input then gets
* transferred to other classes for use.
*/


//This enum class tells us the current state of the InputProcessor. When in the none state 
//we are currently accepting any input. When in the WaitForInput state we are currently waiting 
//for a mouse click or keyboard press to process. In the Active state we are currently processing 
//a keyboard or mouse click so any other input will be ignored.
enum class InputState
{
	None,
	WaitForInput,
	Active,
};

class InputProcessor
{
public:
	InputProcessor(_In_ winrt::Windows::UI::Core::CoreWindow const& window);

	void InitWindow(_In_ winrt::Windows::UI::Core::CoreWindow const& window);

private:
    void ResetState();

    void OnPointerPressed(
        _In_ winrt::Windows::UI::Core::CoreWindow const& sender,
        _In_ winrt::Windows::UI::Core::PointerEventArgs const& args
    );
    /*void OnPointerMoved(
        _In_ winrt::Windows::UI::Core::CoreWindow const& sender,
        _In_ winrt::Windows::UI::Core::PointerEventArgs const& args
    );*/
    void OnPointerReleased(
        _In_ winrt::Windows::UI::Core::CoreWindow const& sender,
        _In_ winrt::Windows::UI::Core::PointerEventArgs const& args
    );
    void OnPointerExited(
        _In_ winrt::Windows::UI::Core::CoreWindow const& sender,
        _In_ winrt::Windows::UI::Core::PointerEventArgs const& args
    );
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
    );*/
    void OnBackRequested(
        _In_ winrt::Windows::Foundation::IInspectable const& sender,
        _In_ winrt::Windows::UI::Core::BackRequestedEventArgs const& args
    );

    // Properties of the controller object.
    InputState                  m_state;
    DirectX::XMFLOAT3           m_velocity;             // How far we move in this frame.
    float                       m_pitch;
    float                       m_yaw;                  // Orientation euler angles in radians.

    // Properties of the Move control.
    DirectX::XMFLOAT2           m_moveUpperLeft;        // Bounding box where this control will activate.
    DirectX::XMFLOAT2           m_moveLowerRight;
    bool                        m_moveInUse;            // The move control is in use.
    uint32_t                    m_movePointerID;        // Id of the pointer in this control.
    DirectX::XMFLOAT2           m_moveFirstDown;        // Point where initial contact occurred.
    DirectX::XMFLOAT2           m_movePointerPosition;  // Point where the move pointer is currently located.
    DirectX::XMFLOAT3           m_moveCommand;          // The net command from the move control.

    // Properties of the Look control.
    bool                        m_lookInUse;            // The look control is in use.
    uint32_t                    m_lookPointerID;        // Id of the pointer in this control.
    DirectX::XMFLOAT2           m_lookLastPoint;        // Last point (from last frame).
    DirectX::XMFLOAT2           m_lookLastDelta;        // The delta used for smoothing between frames.

    // Properties of the Fire control.
    bool                        m_autoFire;
    bool                        m_firePressed;
    DirectX::XMFLOAT2           m_fireUpperLeft;        // Bounding box where this control will activate.
    DirectX::XMFLOAT2           m_fireLowerRight;
    bool                        m_fireInUse;            // The fire control in in use.
    UINT32                      m_firePointerID;        // Id of the pointer in this control.
    DirectX::XMFLOAT2           m_fireLastPoint;        // Last fire position.

    // Properties of the Mouse control. This is a combination of Look and Fire.
    bool                        m_mouseInUse;
    uint32_t                    m_mousePointerID;
    DirectX::XMFLOAT2           m_mouseLastPoint;
    bool                        m_mouseLeftInUse;
    bool                        m_mouseRightInUse;

    bool                        m_buttonInUse;
    uint32_t                    m_buttonPointerID;
    DirectX::XMFLOAT2           m_buttonUpperLeft;
    DirectX::XMFLOAT2           m_buttonLowerRight;
    bool                        m_buttonPressed;
    bool                        m_pausePressed;

    // Input states for Keyboard.
    bool                        m_forward;
    bool                        m_back;
    bool                        m_left;
    bool                        m_right;
    bool                        m_up;
    bool                        m_down;
    bool                        m_pause;

    // Game controller related members.
    winrt::Windows::Gaming::Input::Gamepad m_activeGamepad{ nullptr };
    std::atomic<bool>                      m_gamepadsChanged;
    bool                                   m_gamepadStartButtonInUse;
    bool                                   m_gamepadTriggerInUse;
};