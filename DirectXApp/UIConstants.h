#pragma once

namespace UIConstants
{
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
    static const float TitleTextLocationX = (TitleRectangleX0 + TitleRectangleX1) / 2.0f;
    static const float TitleTextLocationY = (TitleRectangleY0 + TitleRectangleY1) / 2.0f;
    static const float TitleTextSizeX = TitleRectangleX1 - TitleRectangleX0;
    static const float TitleTextSizeY = TitleRectangleY1 - TitleRectangleY0;

    //TextType::SUB_TITLE constants
    static const float SubTitleTextPointSize = 0.033f;
    static const float SubTitleRectangleX0 = WidthMargin;
    static const float SubTitleRectangleX1 = 1.0 - WidthMargin;
    static const float SubTitleRectangleY0 = TitleRectangleY1 + HeightMargin;
    static const float SubTitleRectangleY1 = SubTitleRectangleY0 + 0.07;
    static const float SubTitleTextLocationX = (SubTitleRectangleX0 + SubTitleRectangleX1) / 2.0f;
    static const float SubTitleTextLocationY = (SubTitleRectangleY0 + SubTitleRectangleY1) / 2.0f;
    static const float SubTitleTextSizeX = SubTitleRectangleX1 - SubTitleRectangleX0;
    static const float SubTitleTextSizeY = SubTitleRectangleY1 - SubTitleRectangleY0;

    //TextType::BODY constants
    static const float BodyTextPointSize = 0.055f;
    static const float BodyRectangleX0 = WidthMargin;
    static const float BodyRectangleX1 = 1.0 - WidthMargin;
    static const float BodyRectangleY0 = SubTitleRectangleY1 + HeightMargin;
    static const float BodyRectangleY1 = BodyRectangleY0 + 0.5;
    static const float BodyTextLocationX = (BodyRectangleX0 + BodyRectangleX1) / 2.0f;
    static const float BodyTextLocationY = (BodyRectangleY0 + BodyRectangleY1) / 2.0f;
    static const float BodyTextSizeX = BodyRectangleX1 - BodyRectangleX0;
    static const float BodyTextSizeY = BodyRectangleY1 - BodyRectangleY0;

    //TextType::SENSOR_INFO constants
    static const float SensorInfoTextPointSize = 0.05f;
    static const float SensorInfoRectangleX0 = WidthMargin;
    static const float SensorInfoRectangleX1 = (1.0 - WidthMargin) / 3.0;
    static const float SensorInfoRectangleY0 = BodyRectangleY1 + HeightMargin;
    static const float SensorInfoRectangleY1 = 1.0 - HeightMargin;
    static const float SensorInfoTextLocationX = (SensorInfoRectangleX0 + SensorInfoRectangleX1) / 2.0f;
    static const float SensorInfoTextLocationY = (SensorInfoRectangleY0 + SensorInfoRectangleY1) / 2.0f;
    static const float SensorInfoTextSizeX = SensorInfoRectangleX1 - SensorInfoRectangleX0;
    static const float SensorInfoTextSizeY = SensorInfoRectangleY1 - SensorInfoRectangleY0;

    //TextType::ALERT constants
    static const float AlertTextPointSize = 0.025f;
    static const float AlertRectangleX0 = SensorInfoRectangleX1 + WidthMargin;
    static const float AlertRectangleX1 = SensorInfoRectangleX1 + (1.0 - WidthMargin) / 3.0;
    static const float AlertRectangleY0 = SensorInfoRectangleY0;
    static const float AlertRectangleY1 = 1.0 - HeightMargin;
    static const float AlertTextLocationX = (AlertRectangleX0 + AlertRectangleX1) / 2.0f;
    static const float AlertTextLocationY = (AlertRectangleY0 + AlertRectangleY1) / 2.0f;
    static const float AlertTextSizeX = AlertRectangleX1 - AlertRectangleX0;
    static const float AlertTextSizeY = AlertRectangleY1 - AlertRectangleY0;

    //TextType::FOOT_NOTE constants
    static const float FootNoteTextPointSize = 0.025f;
    static const float FootNoteRectangleX0 = AlertRectangleX1 + WidthMargin;
    static const float FootNoteRectangleX1 = 1.0 - WidthMargin;
    static const float FootNoteRectangleY0 = SensorInfoRectangleY0;
    static const float FootNoteRectangleY1 = 1.0 - HeightMargin;
    static const float FootNoteTextLocationX = (FootNoteRectangleX0 + FootNoteRectangleX1) / 2.0f;
    static const float FootNoteTextLocationY = (FootNoteRectangleY0 + FootNoteRectangleY1) / 2.0f;
    static const float FootNoteTextSizeX = FootNoteRectangleX1 - FootNoteRectangleX0;
    static const float FootNoteTextSizeY = FootNoteRectangleY1 - FootNoteRectangleY0;
};