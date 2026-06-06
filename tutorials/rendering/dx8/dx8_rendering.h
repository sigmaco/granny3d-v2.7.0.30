#if !defined(DX8_RENDERING_H)
// ========================================================================
// $File: //jeffr/granny/tutorial/rendering/dx8/dx8_rendering.h $
// $DateTime: 2007/01/03 17:29:53 $
// $Change: 14092 $
// $Revision: #2 $
//
// See .cpp file for details...
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================
#include "granny.h"
#include <d3d8.h>
#include <d3dx8.h>
#include <vector>

#define VSCONSTANT_NUM768_NUM1OVER4096_NUM0_NUM1 0
#define VSCONSTANT_NUM1OVER2_NUM2_NUM1_x 1
#define VSCONSTANT_LIGHT_DIRECTION_ALPHA 2
#define VSCONSTANT_LIGHT_COLOUR_AMBIENT 3
#define VSCONSTANT_FIRST_VIEWPROJ_MATRIX4 4
#define VSCONSTANT_FIRST_BONE_MATRIX3 8

struct DemoTexture
{
    char* Name;
    IDirect3DTexture8* TextureBuffer;

    DemoTexture();
    ~DemoTexture();
};

struct DemoMesh
{
    granny_mesh*         Mesh;
    granny_mesh_binding* MeshBinding;
    std::vector<DemoTexture*> MaterialBindings;

    IDirect3DIndexBuffer8*  IndexBuffer;
    IDirect3DVertexBuffer8* VertexBuffer;

    DemoMesh();
    ~DemoMesh();
};

struct DemoModel
{
    granny_model_instance* ModelInstance;
    std::vector<DemoMesh*>      BoundMeshes;

    granny_real32 InitialMatrix[16];

    DemoModel();
    ~DemoModel();
};

struct DemoScene
{
    granny_camera DemoCamera;

    granny_file      *SceneFile;
    granny_file_info *SceneFileInfo;

    std::vector<DemoTexture*> Textures;
    std::vector<DemoModel*>   Models;

    // For the purposes of this sample, we'll be sampling the model
    // animation immediately before rendering, which allows us to share
    // the local pose across all models.
    granny_int32x      MaxBoneCount;
    granny_local_pose* SharedLocalPose;
    granny_world_pose* SharedWorldPose;

    // Simple directional light
    float DirFromLight[4];
    float LightColour[4];
    float AmbientColour[4];

    DemoScene();
    ~DemoScene();
};

// ---------------------------------------------------------
// Global D3D Objects:
extern IDirect3D8* g_pD3D;
extern IDirect3DDevice8* g_pD3DDevice;
extern DWORD g_pVertexShader_Rigid;
extern DWORD g_pVertexShader_Skinned;

// ---------------------------------------------------------
// Global Win32 objects
extern const char* MainWindowTitle;
extern HWND g_hwnd;
extern bool GlobalRunning;

// ---------------------------------------------------------
// Handy functions.
bool InitializeD3D();
void CleanupD3D();
void SetTransposedMatrix4x4(LPDIRECT3DDEVICE8 D3DDevice,
                            int ShaderConstant,
                            float const *Matrix );
void SetTransposedMatrix4x3(LPDIRECT3DDEVICE8 D3DDevice,
                            int ShaderConstant,
                            float const *Matrix );

void Render(int Width, int Height);

LRESULT CALLBACK MainWindowCallback(HWND Window, UINT Message,
                                    WPARAM WParam, LPARAM LParam);


#define DX8_RENDERING_H
#endif
