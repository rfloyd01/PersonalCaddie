#include "pch.h"
#include "InputProcessor.h"

using namespace DirectX;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Input;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Devices::Input;
using namespace winrt::Windows::Gaming::Input;
using namespace winrt::Windows::System;

InputProcessor::InputProcessor(_In_ CoreWindow const& window) :
    m_keyboardState(KeyboardState::None)//,
    //m_mouseState(MouseClickState::None)
{
    m_state.currentPressedKey = KeyboardKeys::DeadKey;
    m_state.mousePosition = { 0, 0 };
    //m_state.mouseClick = false;
    m_state.mouseClickState = MouseClickState::None;
    m_state.scrollWheelDirection = 0;
    InitWindow(window);
}

void InputProcessor::InitWindow(_In_ CoreWindow const& window)
{
    ResetState();

    window.PointerPressed({ this, &InputProcessor::OnPointerPressed });

    window.PointerReleased({ this, &InputProcessor::OnPointerReleased });

    /*
    window.PointerExited({ this, &InputProcessor::OnPointerExited });*/

    window.PointerMoved({ this, &InputProcessor::OnPointerMoved });

    window.KeyDown({ this, &InputProcessor::OnKeyDown });

    window.KeyUp({ this, &InputProcessor::OnKeyUp });

    window.PointerWheelChanged({this, &InputProcessor::OnPointerWheelScroll});

    // There is a separate handler for mouse-only relative mouse movement events.
    //MouseDevice::GetForCurrentView().MouseMoved({ this, &InputProcessor::OnMouseMoved });

    //SystemNavigationManager::GetForCurrentView().BackRequested({ this, &InputProcessor::OnBackRequested });
}

void InputProcessor::setKeyboardState(KeyboardState ks)
{
    m_keyboardState = ks;
}

void InputProcessor::setMouseState(MouseClickState ms)
{
    //m_mouseState = ms;
    m_state.mouseClickState = ms;
}

void InputProcessor::OnPointerPressed(
    _In_ CoreWindow const& /* sender */,
    _In_ PointerEventArgs const& args
)
{
    //Either a left or right mouse click has occured. Only update
    //the MouseClickState if clicks are currently allowed.
    if (args.CurrentPoint().Properties().IsLeftButtonPressed())
    {
        if (m_state.mouseClickState == MouseClickState::WaitForInput)
        {
            m_state.mouseClickState = MouseClickState::MouseClicked;
        }
    }
    else if (args.CurrentPoint().Properties().IsRightButtonPressed())
    {
        if (m_state.mouseClickState == MouseClickState::WaitForInput)
        {
            m_state.mouseClickState = MouseClickState::MouseRightClick;
        }
    }
}

void InputProcessor::OnPointerReleased(
    _In_ CoreWindow const& /* sender */,
    _In_ PointerEventArgs const& args
)
{
    //if (m_mouseState == MouseClickState::MouseHeld)
    if (m_state.mouseClickState == MouseClickState::MouseHeld)
    {
        //m_mouseState = MouseClickState::WaitForInput;
        m_state.mouseClickState = MouseClickState::MouseReleased;
    }
}

void InputProcessor::OnPointerWheelScroll(
    _In_ winrt::Windows::UI::Core::CoreWindow const& sender,
    _In_ winrt::Windows::UI::Core::PointerEventArgs const& args
)
{
    //The only thing we do when scrolling the mouse wheel is update the scrollWheelDirection
    //property of the input state. Once the scroll has been processed elsewhere this
    //value will be rest.
    m_state.scrollWheelDirection = args.CurrentPoint().Properties().MouseWheelDelta();
}

void InputProcessor::OnPointerMoved(
    _In_ CoreWindow const& /* sender */,
    _In_ PointerEventArgs const& args
)
{
    //For now I only really care about the location of the pointer. This isn't a game
    //where moving the mouse will move the camera or anything like that so just update
    //the local pointer position variable

    PointerPoint point = args.CurrentPoint();
    uint32_t pointerID = point.PointerId();
    Point pointerPosition = point.Position();
    PointerPointProperties pointProperties = point.Properties();
    auto pointerDevice = point.PointerDevice();

    m_state.mousePosition = XMFLOAT2(pointerPosition.X, pointerPosition.Y);     // convert to allow math
}

//void InputProcessor::OnMouseMoved(
//    _In_ MouseDevice const& /* mouseDevice */,
//    _In_ MouseEventArgs const& args
//)
//{
//    // Handle Mouse Input via dedicated relative movement handler.
//
//    switch (m_state)
//    {
//    case InputState::Active:
//        XMFLOAT2 mouseDelta;
//        mouseDelta.x = static_cast<float>(args.MouseDelta().X);
//        mouseDelta.y = static_cast<float>(args.MouseDelta().Y);
//
//        XMFLOAT2 rotationDelta;
//        rotationDelta.x = mouseDelta.x * MoveLookConstants::RotationGain;   // Scale for control sensitivity.
//        rotationDelta.y = mouseDelta.y * MoveLookConstants::RotationGain;
//
//        // Update our orientation based on the command.
//        m_pitch -= rotationDelta.y;
//        m_yaw += rotationDelta.x;
//
//        // Limit pitch to straight up or straight down.
//        float limit = XM_PI / 2.0f - 0.01f;
//        m_pitch = __max(-limit, m_pitch);
//        m_pitch = __min(+limit, m_pitch);
//
//        // Keep longitude in sane range by wrapping.
//        if (m_yaw > XM_PI)
//        {
//            m_yaw -= XM_PI * 2.0f;
//        }
//        else if (m_yaw < -XM_PI)
//        {
//            m_yaw += XM_PI * 2.0f;
//        }
//        break;
//    }
//}

//void InputProcessor::OnPointerExited(
//    _In_ CoreWindow const& /* sender */,
//    _In_ PointerEventArgs const& args
//)
//{
//    PointerPoint point = args.CurrentPoint();
//    uint32_t pointerID = point.PointerId();
//    Point pointerPosition = point.Position();
//    PointerPointProperties pointProperties = point.Properties();
//
//    XMFLOAT2 position = XMFLOAT2(pointerPosition.X, pointerPosition.Y);
//
//#ifdef InputProcessor_TRACE
//    DebugTrace(L"%-7s (%d) at (%4.0f, %4.0f)\n", L"Exit", pointerID, position.x, position.y);
//#endif
//
//    switch (m_state)
//    {
//    case InputState::WaitForInput:
//        if (m_buttonInUse && (pointerID == m_buttonPointerID))
//        {
//            m_buttonInUse = false;
//            m_buttonPressed = false;
//#ifdef InputProcessor_TRACE
//            DebugTrace(L"\tWaitForPress: ButtonPressed = false -- Got Exit Event\n");
//#endif
//        }
//        break;
//
//    case InputState::Active:
//        if (pointerID == m_movePointerID)
//        {
//            m_velocity = XMFLOAT3(0, 0, 0);      // Stop on release.
//            m_moveInUse = false;
//            m_movePointerID = 0;
//        }
//        else if (pointerID == m_lookPointerID)
//        {
//            m_lookInUse = false;
//            m_lookPointerID = 0;
//        }
//        else if (pointerID == m_firePointerID)
//        {
//            m_fireInUse = false;
//            m_firePointerID = 0;
//        }
//        else if (pointerID == m_mousePointerID)
//        {
//            m_mouseInUse = false;
//            m_mousePointerID = 0;
//            m_mouseLeftInUse = false;
//            m_mouseRightInUse = false;
//        }
//        break;
//    }
//}

void InputProcessor::OnKeyDown(
    _In_ CoreWindow const& /* sender */,
    _In_ KeyEventArgs const& args
)
{
    if (m_keyboardState == KeyboardState::WaitForInput)
    {
        //When a key is pressed we change the keyboard state 
        //so that all other keys are disabled until this key
        //is released
        m_keyboardState = KeyboardState::KeyPressed;
        m_state.currentPressedKey = args.VirtualKey();
        newKeyPress = true; //flag to Main.cpp that a new key was pressed
    }
    
}

void InputProcessor::OnKeyUp(
    _In_ CoreWindow const& /* sender */,
    _In_ KeyEventArgs const& args
)
{
    if (m_keyboardState == KeyboardState::KeyPressed)
    {
        m_keyboardState = KeyboardState::WaitForInput;
    }
}

InputState* InputProcessor::update()
{
    //The main render loop happens so quickly that this update method in the 
    //Process Input class may be called hundreds of times in the amount of time
    //it takes to press a keyboard key or click a mouse button and then release.
    //To prevent certain logic tied to input commands from also happening hundreds
    //of times with a single click or key press we change the state of the mouse
    //and keyboard to "Processed" as soon as their desired action has been accomplished.
    //This prevents the multiple actions that would normally arise from the 
    //"Pressed" state being active
    if (m_keyboardState == KeyboardState::KeyProcessed)
    {
        m_state.currentPressedKey = KeyboardKeys::DeadKey;
        m_keyboardState = KeyboardState::KeyPressed; //TODO: Shouldn't this change to 'waiting for input'?
    }

    //My new mouse clicking scheme renders the below block of code obsolete
    //if (m_mouseState == MouseClickState::MouseHeld)
    //{
    //    m_state.mouseClick = false;
    //    m_mouseState = MouseClickState::MouseReleased; //TODO: Shouldn't this change to 'waiting for input'?
    //}

    return &m_state;
}

void InputProcessor::ResetState()
{
    // Reset the state of the InputProcessor.
    // Disable any active pointer IDs to stop all interaction.
    
    //TODO: Fill this out as code gets more complicated
}

//void InputProcessor::OnBackRequested(
//    _In_ IInspectable const& /* sender */,
//    _In_ BackRequestedEventArgs const& args
//)
//{
//    if (m_state == InputState::Active)
//    {
//        // The game is currently in active play mode, so hitting the hardware back button
//        // will cause the game to pause.
//        m_pausePressed = true;
//        args.Handled(true);
//    }
//    else
//    {
//        // The game is not currently in active play mode, so take the default behavior
//        // for the hardware back button.
//        args.Handled(false);
//    }
//}