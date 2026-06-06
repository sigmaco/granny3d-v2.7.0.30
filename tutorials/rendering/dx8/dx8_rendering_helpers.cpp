// ========================================================================
// $File: //jeffr/granny/tutorial/rendering/dx8/dx8_rendering_helpers.cpp $
// $DateTime: 2007/01/03 17:29:53 $
// $Change: 14092 $
// $Revision: #2 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================
#include "dx8_rendering.h"

// ---------------------------------------------------------------------------------------
// These functions may be of interest, but aren't directly relevant to the tutorial, so we
// stuff them out of the documentation autogenerate section.
// ---------------------------------------------------------------------------------------

char const* const VertexShaderRigidFile   = "shaders\\rigid.vs";
char const* const VertexShaderSkinnedFile = "shaders\\skinned.vs";

#define HARDCODED_DISPLAY_WIDTH  1280
#define HARDCODED_DISPLAY_HEIGHT 720


#define CHECKED_RELEASE(x) { if (x) { (x)->Release(); } (x) = NULL; } 0

DWORD SkinnedVertexDecl[] =
{
    D3DVSD_STREAM(0),
    D3DVSD_REG(0, D3DVSDT_FLOAT3 ),
    D3DVSD_REG(1, D3DVSDT_D3DCOLOR ),
    D3DVSD_REG(2, D3DVSDT_D3DCOLOR ),
    D3DVSD_REG(3, D3DVSDT_FLOAT3 ),
    D3DVSD_REG(4, D3DVSDT_FLOAT2 ),
    D3DVSD_END()
};

DWORD RigidVertexDecl[] =
{
    D3DVSD_STREAM(0),
    D3DVSD_REG(0, D3DVSDT_FLOAT3 ),
    D3DVSD_REG(1, D3DVSDT_FLOAT3 ),
    D3DVSD_REG(2, D3DVSDT_FLOAT2 ),
    D3DVSD_END()
};

//-------------------------------------------------------------------------------------
// Loads a shader
//-------------------------------------------------------------------------------------
#define LOAD_SHADER(file, vert_decl, shader_var)                                            \
    {                                                                                       \
        ID3DXBuffer* CompiledShader = NULL;                                                 \
        ID3DXBuffer* ErrorBuffer = NULL;                                                    \
        if (FAILED(D3DXAssembleShaderFromFile(file, 0, NULL,                                \
                                              &CompiledShader, &ErrorBuffer)))              \
        {                                                                                   \
            CHECKED_RELEASE(CompiledShader);                                                \
            CHECKED_RELEASE(ErrorBuffer);                                                   \
            return false;                                                                   \
        }                                                                                   \
        g_pD3DDevice->CreateVertexShader(vert_decl,                                         \
                                         (DWORD const*)CompiledShader->GetBufferPointer(),  \
                                         &(shader_var), 0);                                 \
        CHECKED_RELEASE(CompiledShader);                                                    \
        CHECKED_RELEASE(ErrorBuffer);                                                       \
    } 0

bool InitializeD3D()
{
    // Create a window to draw in...
    {
        WNDCLASSEX WindowClass = {sizeof(WindowClass)};
        WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        WindowClass.lpfnWndProc = MainWindowCallback;
        WindowClass.hInstance = GetModuleHandle(0);
        //WindowClass.hIcon = LoadIcon(WindowClass.hInstance, MAKEINTRESOURCE(101));
        WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
        WindowClass.lpszClassName = "DX8Basic_SimpleClass";

        if(!RegisterClassEx(&WindowClass))
            return false;

        g_hwnd = CreateWindowEx(0, WindowClass.lpszClassName, MainWindowTitle,
                                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                HARDCODED_DISPLAY_WIDTH,
                                HARDCODED_DISPLAY_HEIGHT,
                                0, 0, GetModuleHandle(0), 0);
        if (!g_hwnd)
            return false;
    }

    // Create the D3D object.
    if((g_pD3D = Direct3DCreate8( D3D_SDK_VERSION )) == NULL)
    {
        return false;
    }

    // Set up the structure used to create the D3DDevice.
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.BackBufferWidth                 = HARDCODED_DISPLAY_WIDTH;
    d3dpp.BackBufferHeight                = HARDCODED_DISPLAY_HEIGHT;
    d3dpp.BackBufferFormat                = D3DFMT_X8R8G8B8;
    d3dpp.BackBufferCount                 = 1;
    d3dpp.SwapEffect                      = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow                   = g_hwnd;
    d3dpp.Windowed                        = TRUE;
    d3dpp.EnableAutoDepthStencil          = TRUE;
    d3dpp.AutoDepthStencilFormat          = D3DFMT_D24S8;

    // Create the Direct3D device.
    if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT,
                                    D3DDEVTYPE_HAL,
                                    g_hwnd,
                                    D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                    &d3dpp,
                                    &g_pD3DDevice)))
    {
        return false;
    }

	// Create our shaders...
    LOAD_SHADER(VertexShaderRigidFile,   RigidVertexDecl,   g_pVertexShader_Rigid);
    LOAD_SHADER(VertexShaderSkinnedFile, SkinnedVertexDecl, g_pVertexShader_Skinned);

    return true;
}


void CleanupD3D()
{
    CHECKED_RELEASE(g_pD3DDevice);
    CHECKED_RELEASE(g_pD3D);
}

void TransposedMatrix4x4(granny_real32 *Matrix)
{
    granny_real32 TMatrix[16];
    static const int remaps[16] = {
        0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15
    };

    for (int i = 0; i < 16; i++)
        TMatrix[i] = Matrix[remaps[i]];
    memcpy(Matrix, TMatrix, sizeof(TMatrix));
}

void SetTransposedMatrix4x4(LPDIRECT3DDEVICE8 D3DDevice,
                            int ShaderConstant,
                            float const *Matrix )
{
    float TMatrix[16];
    memcpy(TMatrix, Matrix, sizeof(TMatrix));
    TransposedMatrix4x4(TMatrix);

     D3DDevice->SetVertexShaderConstant( ShaderConstant, TMatrix, 4 );
}

void SetTransposedMatrix4x3(LPDIRECT3DDEVICE8 D3DDevice,
                            int ShaderConstant,
                            float const *Matrix )
{
    float TMatrix[16];
    memcpy(TMatrix, Matrix, sizeof(TMatrix));
    TransposedMatrix4x4(TMatrix);

    D3DDevice->SetVertexShaderConstant( ShaderConstant, TMatrix, 3 );
}



//-------------------------------------------------------------------------------------
// Name: MainWindowCallback
// Desc: Our message handler...
//-------------------------------------------------------------------------------------
LRESULT CALLBACK
MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;

    switch(Message)
    {
        case WM_PAINT:
        {
            // Let Win32 know we're repainting
            HDC RenderingDC;
            PAINTSTRUCT PaintInfo;
            RenderingDC = BeginPaint(Window, &PaintInfo);

            // Render with D3D
            RECT ClientRectangle;
            GetClientRect(Window, &ClientRectangle);

            Render(ClientRectangle.right, ClientRectangle.bottom);

            // Let Win32 know we're finished
            EndPaint(Window, &PaintInfo);
        } break;

        case WM_DESTROY:
        {
            GlobalRunning = false;
        } break;

        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }

    return(Result);
}


DemoTexture::DemoTexture()
  : Name(NULL),
    TextureBuffer(NULL)
{

}

DemoTexture::~DemoTexture()
{
    delete [] Name;
    Name = NULL;

    CHECKED_RELEASE(TextureBuffer);
}

DemoMesh::DemoMesh()
  : Mesh(NULL),
    MeshBinding(NULL),
    IndexBuffer(NULL),
    VertexBuffer(NULL)
{

}

DemoMesh::~DemoMesh()
{
    // We don't own these
    Mesh = NULL;
    MaterialBindings.clear();

    // We do own these
    if (MeshBinding)
    {
        GrannyFreeMeshBinding(MeshBinding);
        MeshBinding = NULL;
    }

    CHECKED_RELEASE(IndexBuffer);
    CHECKED_RELEASE(VertexBuffer);
}

DemoModel::DemoModel()
  : ModelInstance(NULL)
{

}

DemoModel::~DemoModel()
{
    if (ModelInstance)
    {
        GrannyFreeModelInstance(ModelInstance);
        ModelInstance = NULL;
    }

    {for(granny_uint32x i = 0; i < BoundMeshes.size(); ++i)
    {
        delete BoundMeshes[i];
        BoundMeshes[i] = NULL;
    }}

    BoundMeshes.clear();
}

DemoScene::DemoScene()
  : SceneFile(NULL),
    SceneFileInfo(NULL),
    SharedLocalPose(NULL),
    SharedWorldPose(NULL),
    MaxBoneCount(-1)
{
    // Nada
}

DemoScene::~DemoScene()
{
    {for(granny_uint32x i = 0; i < Textures.size(); ++i)
    {
        delete Textures[i];
        Textures[i] = NULL;
    }}

    {for(granny_uint32x i = 0; i < Models.size(); ++i)
    {
        delete Models[i];
        Models[i] = NULL;
    }}

    GrannyFreeLocalPose(SharedLocalPose);
    GrannyFreeWorldPose(SharedWorldPose);
    SharedLocalPose = NULL;
    SharedWorldPose = NULL;

    GrannyFreeFile(SceneFile);
    SceneFile = NULL;
    SceneFileInfo = NULL;
}
