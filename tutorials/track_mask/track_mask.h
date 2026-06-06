#if !defined(DX9_RENDERING_H)
// ========================================================================
// $File: //jeffr/granny/tutorial/track_mask/track_mask.h $
// $DateTime: 2007/01/02 17:23:14 $
// $Change: 14072 $
// $Revision: #1 $
//
// See .cpp file for details...
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================
#include "granny.h"
#include <d3d9.h>
#include <vector>

struct DemoTexture
{
    char* Name;
    IDirect3DTexture9* TextureBuffer;

    DemoTexture();
    ~DemoTexture();
};

struct DemoMesh
{
    granny_mesh*         Mesh;
    granny_mesh_binding* MeshBinding;
    std::vector<DemoTexture*> MaterialBindings;

    IDirect3DIndexBuffer9*  IndexBuffer;
    IDirect3DVertexBuffer9* VertexBuffer;

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
// Global D3D Objects: Note that we only have a few shader
// objects, so we'll just make them global.
extern IDirect3D9* g_pD3D;
extern IDirect3DDevice9* g_pD3DDevice;
extern IDirect3DVertexDeclaration9* g_pVertexDecl_Rigid;
extern IDirect3DVertexDeclaration9* g_pVertexDecl_Skinned;
extern IDirect3DVertexShader9* g_pVertexShader_Rigid;
extern IDirect3DVertexShader9* g_pVertexShader_Skinned;
extern IDirect3DPixelShader9* g_pPixelShader;

// ---------------------------------------------------------
// Global Win32 objects
extern const char* MainWindowTitle;
extern HWND g_hwnd;
extern bool GlobalRunning;

bool CreateDemoTextures();
bool CreateDemoModels();
bool CreateSharedPoses();
bool InitCameraAndLights();
void RenderModel(DemoModel*);
void Update(granny_real32 const CurrentTime,
            granny_real32 const DeltaTime);


// ---------------------------------------------------------
// Handy functions.
bool InitializeD3D();
void CleanupD3D();
void SetTransposedMatrix4x4(LPDIRECT3DDEVICE9 D3DDevice,
                            int ShaderConstant,
                            float const *Matrix );
void SetTransposedMatrix4x3(LPDIRECT3DDEVICE9 D3DDevice,
                            int ShaderConstant,
                            float const *Matrix );

void Render(int Width, int Height);

LRESULT CALLBACK MainWindowCallback(HWND Window, UINT Message,
                                    WPARAM WParam, LPARAM LParam);


#define DX9_RENDERING_H
#endif
