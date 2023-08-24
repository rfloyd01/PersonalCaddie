#pragma once

namespace InfoOverlayConstant
{
    static const float Width = 750.0f;
    static const float Height = 380.0f;
    static const float TitlePointSize = 32.0f;
    static const float TitleHeight = 50.0f;
    static const float Separator = 8.0f;
    static const float BodyPointSize = 24.0f;
    static const float ActionHeight = 40.0f;
    static const float TopMargin = 20.0f;
    static const float SideMargin = 20.0f;
    static const float BottomMargin = 5.0f;
};

namespace UIConstants
{
    static const float CrossHairHalfSize = 20.0f;
    static const float HudSafeWidth = 200.0f;
    static const float HudRightOffset = 175.0f;
    static const float HudTopOffset = 15.0f;
    static const float Margin = 5.0f;
    static const float HudBodyPointSize = 20.0f;
    static const float HudTitleHeaderPointSize = 16.0f;
    static const float HudTitleBodyPointSize = 20.0f;
    static const float HudLicensePointSize = 14.0f;

    static const float TouchRectangleSize = 125.0f;
    static const float MinPlayableWidth = 320.0f;

    //The below values are expressed as a percentage of the full width or height
    //of the app window
    static const float TitleTextPointSize = 0.10f;
    static const float SubTitleTextPointSize = 0.033f;
    static const float BodyTextPointSize = 0.055f;
    static const float SensorInfoTextPointSize = 0.05f;
    static const float FootNoteTextPointSize = 0.025f;
    static const float AlertTextPointSize = 0.033f;
    
    static const float WidthMargin = 0.02f;
    static const float HeightMargin = 0.02f;
    static const float TitleRectangleWidth = 1.0 - 2 * WidthMargin;
    static const float TitleRectangleHeight = 0.15f - HeightMargin;
    static const float SubTitleRectangleWidth = 1.0 - 2 * WidthMargin;
    static const float SubTitleRectangleHeight = 0.30f - TitleRectangleHeight - 2 * HeightMargin;
    static const float BodyRectangleWidth = 1.0 - 2 * WidthMargin;
    static const float BodyRectangleHeight = 0.80 - (0.35 + HeightMargin);
    static const float SensorInfoRectangleWidth = (1.0f - 4.0 * WidthMargin) / 3.0;
    static const float SensorInfoRectangleHeight = 0.97 - (0.75 + HeightMargin);
    static const float AlertRectangleWidth = (1.0f - 4.0 * WidthMargin) / 3.0;
    static const float AlertRectangleHeight = 0.97 - (0.75 + HeightMargin);
    static const float FootNoteRectangleWidth = (1.0f - 4.0 * WidthMargin) / 3.0;
    static const float FootNoteRectangleHeight = 0.97 - (0.75 + HeightMargin);
};
