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
    static const float WidthMargin = 0.02f;
    static const float HeightMargin = 0.02f;

    //TextType::TITLE constants
    static const float TitleTextPointSize = 0.10f;
    static const float TitleRectangleX0 = WidthMargin;
    static const float TitleRectangleX1 = 1.0 - WidthMargin;
    static const float TitleRectangleY0 = HeightMargin;
    static const float TitleRectangleY1 = TitleRectangleY0 + 0.15;

    //TextType::SUB_TITLE constants
    static const float SubTitleTextPointSize = 0.033f;
    static const float SubTitleRectangleX0 = WidthMargin;
    static const float SubTitleRectangleX1 = 1.0 - WidthMargin;
    static const float SubTitleRectangleY0 = TitleRectangleY1 + HeightMargin;
    static const float SubTitleRectangleY1 = SubTitleRectangleY0 + 0.7;

    //TextType::BODY constants
    static const float BodyTextPointSize = 0.055f;
    static const float BodyRectangleX0 = WidthMargin;
    static const float BodyRectangleX1 = 1.0 - WidthMargin;
    static const float BodyRectangleY0 = SubTitleRectangleY1 + HeightMargin;
    static const float BodyRectangleY1 = BodyRectangleY0 + 0.4;

    //TextType::SENSOR_INFO constants
    static const float SensorInfoTextPointSize = 0.05f;
    static const float SensorInfoRectangleX0 = WidthMargin;
    static const float SensorInfoRectangleX1 = (1.0 - WidthMargin) / 3.0;
    static const float SensorInfoRectangleY0 = BodyRectangleY1 + HeightMargin;
    static const float SensorInfoRectangleY1 = 1.0 - HeightMargin;

    //TextType::ALERT constants
    static const float AlertTextPointSize = 0.033f;
    static const float AlertRectangleX0 = SensorInfoRectangleX1 + WidthMargin;
    static const float AlertRectangleX1 = SensorInfoRectangleX1 + (1.0 - WidthMargin) / 3.0;
    static const float AlertRectangleY0 = SensorInfoRectangleY0;
    static const float AlertRectangleY1 = 1.0 - HeightMargin;

    //TextType::FOOT_NOTE constants
    static const float FootNoteTextPointSize = 0.025f;
    static const float FootNoteRectangleX0 = AlertRectangleX1 + WidthMargin;
    static const float FootNoteRectangleX1 = 1.0 - WidthMargin;
    static const float FootNoteRectangleY0 = SensorInfoRectangleY0;
    static const float FootNoteRectangleY1 = 1.0 - HeightMargin;
};
