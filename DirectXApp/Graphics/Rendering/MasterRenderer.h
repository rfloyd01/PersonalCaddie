#pragma once

#include "UIElementRenderer.h"
#include "UIConstants.h"

// MasterRenderer:
// This is the main renderer for the application.
// It is responsble for creating and maintaining all the D3D11 and D2D objects
// used to generate the app visuals. It maintains a reference to the Mode
// object to retrieve the list of objects to render.
//
// The renderer is designed to use a standard vertex layout across all objects. This
// simplifies the shader design and allows for easy changes between shaders independent
// of the objects' geometry. Each vertex is defined by a position, a normal and one set of
// 2D texture coordinates. The shaders all expect one 2D texture and 4 constant buffers:
//     m_constantBufferNeverChanges - general parameters that are set only once. This includes
//         all the lights used in scene generation.
//     m_constantBufferChangeOnResize - the projection matrix. It is typically only changed when
//         the window is resized
//     m_constantBufferChangesEveryFrame - the view transformation matrix. This is set once per frame.
//     m_constantBufferChangesEveryPrim - the parameters for each object. It includes the object to world
//         transformation matrix as well as material properties like color and specular exponent for lighting
//         calculations.
//
// The renderer also maintains a set of texture resources that will be associated with particular objects.
// It knows which textures are to be associated with which objects and will do that association once the
// textures have been loaded.
//
// The renderer provides a set of methods to allow for a "standard" sequence to be executed for loading general
// resources and for mode specific resources. Because D3D11 allows free threaded creation of objects,
// textures will be loaded asynchronously and in parallel, however D3D11 does not allow for multiple threads to
// be using the DeviceContext at the same time.
//
// Use co_await with IAsyncAction to perform a sequence of asynchronous operations in the same thread context.

class MasterRenderer : public std::enable_shared_from_this<MasterRenderer>
{
public:
	MasterRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources, std::shared_ptr<ModeScreen> const& modeScreen);

    IAsyncAction CreateModeResourcesAsync();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
    void ReleaseDeviceDependentResources();
    void Render();

    void setTextLayoutPixels(UIText* text);

    winrt::Windows::Foundation::Size getCurrentScreenSize();

private:
    // Cached pointer to device resources.
    std::shared_ptr<DX::DeviceResources>        m_deviceResources;

    bool                                        m_initialized;
    bool                                        m_gameResourcesLoaded;
    bool                                        m_levelResourcesLoaded;

    UIElementRenderer                           m_uiElementRenderer;

    std::shared_ptr<ModeScreen>                 m_mode;
    D2D_RECT_F                                  m_gameInfoOverlayRect;
    D2D_SIZE_F                                  m_gameInfoOverlaySize;

    winrt::com_ptr<ID3D11ShaderResourceView>    m_sphereTexture;
    winrt::com_ptr<ID3D11ShaderResourceView>    m_cylinderTexture;
    winrt::com_ptr<ID3D11ShaderResourceView>    m_ceilingTexture;
    winrt::com_ptr<ID3D11ShaderResourceView>    m_floorTexture;
    winrt::com_ptr<ID3D11ShaderResourceView>    m_wallsTexture;

    // Constant Buffers
    winrt::com_ptr<ID3D11Buffer>                m_constantBufferNeverChanges;
    winrt::com_ptr<ID3D11Buffer>                m_constantBufferChangeOnResize;
    winrt::com_ptr<ID3D11Buffer>                m_constantBufferChangesEveryFrame;
    winrt::com_ptr<ID3D11Buffer>                m_constantBufferChangesEveryPrim;
    winrt::com_ptr<ID3D11SamplerState>          m_samplerLinear;
    winrt::com_ptr<ID3D11VertexShader>          m_vertexShader;
    winrt::com_ptr<ID3D11VertexShader>          m_vertexShaderFlat;
    winrt::com_ptr<ID3D11PixelShader>           m_pixelShader;
    winrt::com_ptr<ID3D11PixelShader>           m_pixelShaderFlat;
    winrt::com_ptr<ID3D11InputLayout>           m_vertexLayout;
};