#if !defined(IK_2BONE_H)
// ========================================================================
// $File: //jeffr/granny/tutorial/ik_2bone/ik_2bone.h $
// $DateTime: 2007/06/05 15:57:56 $
// $Change: 15105 $
// $Revision: #1 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================
#include "granny.h"
#include <d3d9.h>
#include <d3dx9.h>
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
	float CurrentTime;
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

extern ID3DXEffect* g_pEffect;

extern D3DXHANDLE g_RigidTechnique;
extern D3DXHANDLE g_SkinnedTechnique;

extern D3DXHANDLE g_hMatObj2World;
extern D3DXHANDLE g_hMatWorld2View;
extern D3DXHANDLE g_hMatView2Clip;
extern D3DXHANDLE g_hDirFromLight;
extern D3DXHANDLE g_hLightColor;
extern D3DXHANDLE g_hAmbientColor;
extern D3DXHANDLE g_hSkinningMatrices;

extern D3DXHANDLE g_hDiffuseTexture;



extern IDirect3DVertexDeclaration9* g_pVertexDecl_Rigid;
extern IDirect3DVertexDeclaration9* g_pVertexDecl_Skinned;

// ---------------------------------------------------------
// Global Win32 objects
extern const char* MainWindowTitle;
extern HWND g_hwnd;
extern bool GlobalRunning;

// ---------------------------------------------------------
// Handy functions.
void ComputeLegPosition(DemoModel* Model);

bool InitializeD3D();
void CleanupD3D();
void SetTransposedMatrix4x4(D3DXHANDLE ShaderConstant,
                            float const *Matrix );

void Render(int Width, int Height);
void RenderModel(DemoModel* Model);

LRESULT CALLBACK MainWindowCallback(HWND Window, UINT Message,
                                    WPARAM WParam, LPARAM LParam);


#define IK_2BONE_H
#endif
