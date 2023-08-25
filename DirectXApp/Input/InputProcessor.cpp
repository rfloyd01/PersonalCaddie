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
    m_keyboardState(KeyboardState::None)
{
    m_state.currentPressedKey = KeyboardKeys::DeadKey;
    InitWindow(window);
}

void InputProcessor::InitWindow(_In_ CoreWindow const& window)
{
    ResetState();

    /*window.PointerPressed({ this, &InputProcessor::OnPointerPressed });

    window.PointerMoved({ this, &InputProcessor::OnPointerMoved });

    window.PointerReleased({ this, &InputProcessor::OnPointerReleased });

    window.PointerExited({ this, &InputProcessor::OnPointerExited });*/

    window.KeyDown({ this, &InputProcessor::OnKeyDown });

    window.KeyUp({ this, &InputProcessor::OnKeyUp });

    // There is a separate handler for mouse-only relative mouse movement events.
    //MouseDevice::GetForCurrentView().MouseMoved({ this, &InputProcessor::OnMouseMoved });

    //SystemNavigationManager::GetForCurrentView().BackRequested({ this, &InputProcessor::OnBackRequested });
}

void InputProcessor::setKeyboardState(KeyboardState ks)
{
    m_keyboardState = ks;
}

//void InputProcessor::OnPointerPressed(
//    _In_ CoreWindow const& /* sender */,
//    _In_ PointerEventArgs const& args
//)
//{
//    PointerPoint point = args.CurrentPoint();
//    uint32_t pointerID = point.PointerId();
//    Point pointerPosition = point.Position();
//    PointerPointProperties pointProperties = point.Properties();
//    auto pointerDevice = point.PointerDevice();
//    auto pointerDeviceType = pointerDevice.PointerDeviceType();
//
//    XMFLOAT2 position = XMFLOAT2(pointerPosition.X, pointerPosition.Y);
//
//#ifdef InputProcessor_TRACE
//    DebugTrace(L"%-7s (%d) at (%4.0f, %4.0f)", L"Pressed", pointerID, position.x, position.y);
//#endif
//
//    OutputDebugString(L"The Mouse is being pressed.\n");
//
//    switch (m_state)
//    {
//    case InputState::WaitForInput:
//        if (position.x > m_buttonUpperLeft.x &&
//            position.x < m_buttonLowerRight.x &&
//            position.y > m_buttonUpperLeft.y &&
//            position.y < m_buttonLowerRight.y)
//        {
//            // Wait until button released before setting variable.
//            m_buttonPointerID = pointerID;
//            m_buttonInUse = true;
//#ifdef InputProcessor_TRACE
//            DebugTrace(L"\tWaitForInput(%d) - BUTTON in USE", pointerID);
//#endif
//        }
//        break;
//
//    case InputState::Active:
//        switch (pointerDeviceType)
//        {
//        case winrt::Windows::Devices::Input::PointerDeviceType::Touch:
//            if (position.x > m_moveUpperLeft.x &&
//                position.x < m_moveLowerRight.x &&
//                position.y > m_moveUpperLeft.y &&
//                position.y < m_moveLowerRight.y)
//            {
//                // This pointer is in the move control.
//                if (!m_moveInUse)
//                {
//                    // There is not an active pointer in this control yet.
//                    // Process a DPad touch down event.
//                    m_moveFirstDown = position;                 // Save location of initial contact.
//                    m_movePointerID = pointerID;                // Store the pointer using this control.
//                    m_moveInUse = true;
//                }
//            }
//            else if (position.x > m_fireUpperLeft.x &&
//                position.x < m_fireLowerRight.x &&
//                position.y > m_fireUpperLeft.y &&
//                position.y < m_fireLowerRight.y)
//            {
//                // This pointer is in the fire control.
//                if (!m_fireInUse)
//                {
//                    m_fireLastPoint = position;
//                    m_firePointerID = pointerID;
//                    m_fireInUse = true;
//                    if (!m_autoFire)
//                    {
//                        m_firePressed = true;
//                    }
//                }
//            }
//            else
//            {
//                if (!m_lookInUse)
//                {
//                    // There is not an active pointer in this control yet.
//                    m_lookLastPoint = position;                   // Save point for later move.
//                    m_lookPointerID = pointerID;                  // Store the pointer using this control.
//                    m_lookLastDelta.x = m_lookLastDelta.y = 0;    // These are for smoothing.
//                    m_lookInUse = true;
//                }
//            }
//            break;
//
//        default:
//            bool rightButton = pointProperties.IsRightButtonPressed();
//            bool leftButton = pointProperties.IsLeftButtonPressed();
//
//            if (!m_autoFire && (!m_mouseLeftInUse && leftButton))
//            {
//                m_firePressed = true;
//            }
//
//            if (!m_mouseInUse)
//            {
//                m_mouseInUse = true;
//                m_mouseLastPoint = position;
//                m_mousePointerID = pointerID;
//                m_mouseLeftInUse = leftButton;
//                m_mouseRightInUse = rightButton;
//                m_lookLastDelta.x = m_lookLastDelta.y = 0;          // These are for smoothing.
//            }
//            else
//            {
//#ifdef InputProcessor_TRACE
//                DebugTrace(L"\tWARNING: OnPointerPressed()  Mouse aleady in use (%d-%s%s) and new event id: %d %s%s",
//                    m_mousePointerID,
//                    m_mouseLeftInUse ? "L" : "",
//                    m_mouseRightInUse ? "R" : "",
//                    pointerID,
//                    leftButton ? "L" : "",
//                    rightButton ? "R" : ""
//                );
//#endif
//            }
//            break;
//        }
//#ifdef InputProcessor_TRACE
//        DebugTrace(
//            L"\t%s%s%s %s%s%s",
//            m_moveInUse ? L"Move " : L"",
//            m_lookInUse ? L"Look " : L"",
//            m_fireInUse ? L"Fire " : L"",
//            m_mouseInUse ? L"Mouse:" : L"",
//            m_mouseLeftInUse ? L"L" : L"-",
//            m_mouseRightInUse ? L"R" : L"-"
//        );
//#endif
//        break;
//    }
//#ifdef InputProcessor_TRACE
//    DebugTrace(L"\n");
//#endif
//    return;
//}

//void InputProcessor::OnPointerMoved(
//    _In_ CoreWindow const& /* sender */,
//    _In_ PointerEventArgs const& args
//)
//{
//    PointerPoint point = args.CurrentPoint();
//    uint32_t pointerID = point.PointerId();
//    Point pointerPosition = point.Position();
//    PointerPointProperties pointProperties = point.Properties();
//    auto pointerDevice = point.PointerDevice();
//
//    XMFLOAT2 position = XMFLOAT2(pointerPosition.X, pointerPosition.Y);     // convert to allow math
//
//#ifdef InputProcessor_TRACE
//    DebugTrace(L"%-7s (%d) at (%4.0f, %4.0f)", L"Moved", pointerID, position.x, position.y);
//#endif
//
//    switch (m_state)
//    {
//    case InputState::Active:
//        // Decide which control this pointer is operating.
//        if (pointerID == m_movePointerID)
//        {
//            m_movePointerPosition = position;       // Save the current position.
//        }
//        else if (pointerID == m_lookPointerID)
//        {
//            // Look control.
//            XMFLOAT2 pointerDelta;
//            pointerDelta.x = position.x - m_lookLastPoint.x;        // How far did pointer move?
//            pointerDelta.y = position.y - m_lookLastPoint.y;
//
//            XMFLOAT2 rotationDelta;
//            rotationDelta.x = pointerDelta.x * MoveLookConstants::RotationGain;       // Scale for control sensitivity.
//            rotationDelta.y = pointerDelta.y * MoveLookConstants::RotationGain;
//            m_lookLastPoint = position;                             // Save for next time through.
//
//#ifdef InputProcessor_TRACE
//            DebugTrace(L"\tDelta (%4.0f, %4.0f)", pointerDelta.x, pointerDelta.y);
//#endif
//
//            // Update our orientation based on the command.
//            m_pitch -= rotationDelta.y;
//            m_yaw += rotationDelta.x;
//
//            // Limit pitch to straight up or straight down.
//            float limit = XM_PI / 2.0f - 0.01f;
//            m_pitch = __max(-limit, m_pitch);
//            m_pitch = __min(+limit, m_pitch);
//
//            // Keep longitude in sane range by wrapping.
//            if (m_yaw > XM_PI)
//            {
//                m_yaw -= XM_PI * 2.0f;
//            }
//            else if (m_yaw < -XM_PI)
//            {
//                m_yaw += XM_PI * 2.0f;
//            }
//        }
//        else if (pointerID == m_firePointerID)
//        {
//            m_fireLastPoint = position;
//        }
//        else if (pointerID == m_mousePointerID)
//        {
//            m_mouseLeftInUse = pointProperties.IsLeftButtonPressed();
//            m_mouseRightInUse = pointProperties.IsRightButtonPressed();
//            m_mouseLastPoint = position;                            // Save for next time through.
//        }
//
//        // Mouse movement is handled via a separate relative mouse movement handler (OnMouseMoved).
//
//        break;
//    }
//#ifdef InputProcessor_TRACE
//    DebugTrace(L"\n");
//#endif
//}
//
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

//void InputProcessor::OnPointerReleased(
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
//    DebugTrace(L"%-7s (%d) at (%4.0f, %4.0f)\n", L"Release", pointerID, position.x, position.y);
//#endif
//
//    switch (m_state)
//    {
//    case InputState::WaitForInput:
//        if (m_buttonInUse && (pointerID == m_buttonPointerID))
//        {
//            m_buttonInUse = false;
//            m_buttonPressed = true;
//#ifdef InputProcessor_TRACE
//            DebugTrace(L"\tWaitForInput: ButtonPressed = true\n");
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
//            bool rightButton = pointProperties.IsRightButtonPressed();
//            bool leftButton = pointProperties.IsLeftButtonPressed();
//
//            m_mouseInUse = false;
//
//            // Don't clear the mouse Pointer ID so that Move events still result in Look changes.
//            m_mouseLeftInUse = leftButton;
//            m_mouseRightInUse = rightButton;
//        }
//        break;
//    }
//}
//
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
        newInput = true; //flag to Main.cpp that a new key was pressed
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
        m_state.currentPressedKey = KeyboardKeys::DeadKey;
    }
}

InputState* InputProcessor::update()
{
    //This method gets called in every iteration of the main application loop
    //to see if anything new has happened
    if (newInput)
    {
        //only update if there's something new to actually process
        newInput = false;
        return &m_state;
    }
    else return nullptr;
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