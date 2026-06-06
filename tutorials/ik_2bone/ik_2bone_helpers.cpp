// ========================================================================
// $File: //jeffr/granny/tutorial/ik_2bone/ik_2bone_helpers.cpp $
// $DateTime: 2007/06/05 15:57:56 $
// $Change: 15105 $
// $Revision: #1 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================
#include "ik_2bone.h"
#include <assert.h>

// Note that we're using extremely simple objects, and a global scene
// database that you'll want to completely ignore when you build your
// own app on top of Granny.  This simply keeps everything very
// straightfoward.

// ---------------------------------------------------------
// Global D3D Objects: Note that we only have a few shader
// objects, so we'll just make them global.
IDirect3D9*       g_pD3D = NULL;
IDirect3DDevice9* g_pD3DDevice = NULL;

IDirect3DVertexDeclaration9*  g_pVertexDecl_Rigid = NULL;
IDirect3DVertexDeclaration9*  g_pVertexDecl_Skinned = NULL;

ID3DXEffect* g_pEffect = NULL;
D3DXHANDLE   g_RigidTechnique = NULL;
D3DXHANDLE   g_SkinnedTechnique = NULL;

D3DXHANDLE g_hMatObj2World;
D3DXHANDLE g_hMatWorld2View;
D3DXHANDLE g_hMatView2Clip;
D3DXHANDLE g_hDirFromLight;
D3DXHANDLE g_hLightColor;
D3DXHANDLE g_hAmbientColor;
D3DXHANDLE g_hSkinningMatrices;
D3DXHANDLE g_hDiffuseTexture;


// ---------------------------------------------------------
// Win32 objects
const char* MainWindowTitle = "DirectX9 GrannySample";
HWND g_hwnd = NULL;
bool GlobalRunning = false;

bool CreateDemoTextures();
bool CreateDemoModels();
bool CreateSharedPoses();
bool InitCameraAndLights();
void RenderModel(DemoModel*);

extern DemoScene GlobalScene;

// ---------------------------------------------------------------------------------------
// These functions may be of interest, but aren't directly relevant to the tutorial, so we
// stuff them out of the documentation autogenerate section.
// ---------------------------------------------------------------------------------------
char const* const GlobalSceneFilename = "..\\media\\pixo_run.gr2";
char const* const ShaderFile         = "Shaders\\Shaders.hlsl";

#define HARDCODED_DISPLAY_WIDTH  1280
#define HARDCODED_DISPLAY_HEIGHT 720

// #define DEBUG_VS

#define CHECKED_RELEASE(x) { if (x) { (x)->Release(); } (x) = NULL; } 0

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
        WindowClass.lpszClassName = "DX9Basic_SimpleClass";

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
    g_pD3D = Direct3DCreate9( D3D_SDK_VERSION );
    if( !g_pD3D )
    {
        return false;
    }

    // Set up the structure used to create the D3DDevice.
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.BackBufferWidth        = HARDCODED_DISPLAY_WIDTH;
    d3dpp.BackBufferHeight       = HARDCODED_DISPLAY_HEIGHT;
    d3dpp.BackBufferFormat       = D3DFMT_X8R8G8B8;
    d3dpp.BackBufferCount        = 1;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow          = g_hwnd;
    d3dpp.Windowed               = TRUE;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
    d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_ONE;

    // Create the Direct3D device.
    HRESULT hr = g_pD3D->CreateDevice(D3DADAPTER_DEFAULT,
                                      D3DDEVTYPE_HAL,
                                      g_hwnd,
#ifdef DEBUG_VS
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
#else
                                      D3DCREATE_HARDWARE_VERTEXPROCESSING,
#endif
                                      &d3dpp,
                                      &g_pD3DDevice);
    if (FAILED(hr))
    {
        return false;
    }


    ID3DXBuffer* ErrorBuffer = NULL;
    if (D3DXCreateEffectFromFile(g_pD3DDevice,
                                        ShaderFile,
                                        NULL,
                                        NULL,
#if !defined(DEBUG_VS)
                                        0,
#else
                                        D3DXSHADER_DEBUG | D3DXSHADER_SKIPOPTIMIZATION,
#endif
                                        NULL,
                                        &g_pEffect, &ErrorBuffer) != S_OK)
    {
        OutputDebugString((char*)ErrorBuffer->GetBufferPointer());
        CHECKED_RELEASE(ErrorBuffer);
        return false;
    }

    g_hMatObj2World     = g_pEffect->GetParameterByName ( NULL, "ObjToWorld" );
    g_hMatWorld2View    = g_pEffect->GetParameterByName ( NULL, "WorldToView" );
    g_hMatView2Clip     = g_pEffect->GetParameterByName ( NULL, "ViewToClip" );
    g_hDirFromLight     = g_pEffect->GetParameterByName ( NULL, "DirFromLight" );
    g_hLightColor       = g_pEffect->GetParameterByName ( NULL, "LightColour" );
    g_hAmbientColor     = g_pEffect->GetParameterByName ( NULL, "AmbientColour" );
    g_hSkinningMatrices = g_pEffect->GetParameterByName ( NULL, "BoneMatrices" );
    g_hDiffuseTexture   = g_pEffect->GetParameterByName ( NULL, "diffuse_texture" );

    g_SkinnedTechnique = g_pEffect->GetTechniqueByName ( "Skinned" );
    if (FAILED(g_pEffect->ValidateTechnique(g_SkinnedTechnique)))
        return false;

    g_RigidTechnique = g_pEffect->GetTechniqueByName ( "Rigid" );
    if (FAILED(g_pEffect->ValidateTechnique(g_RigidTechnique)))
        return false;


    // Create the rigid and skinned vertex declarations
    {
        D3DVERTEXELEMENT9 VertexElementsRigid[] = {
            { 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
            { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
            { 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
            D3DDECL_END()
        };

        // Note the position[1-3] elements are pulled from vertex
        // stream 1, rather than 0.  We'll be binding stream 1 to our
        // matrix palette.  Essentially we're hijacking those vertex
        // semantics to represent a set of 4x3 matrices.
        const D3DVERTEXELEMENT9 VertexElementsSkinned[] = {
            { 0,  0, D3DDECLTYPE_FLOAT3,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,     0 },
            { 0, 12, D3DDECLTYPE_UBYTE4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT,  0 },
            { 0, 16, D3DDECLTYPE_UBYTE4,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
            { 0, 20, D3DDECLTYPE_FLOAT3,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,       0 },
            { 0, 32, D3DDECLTYPE_FLOAT2,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,     0 },
            D3DDECL_END()
        };

        g_pD3DDevice->CreateVertexDeclaration( VertexElementsRigid,   &g_pVertexDecl_Rigid   );
        g_pD3DDevice->CreateVertexDeclaration( VertexElementsSkinned, &g_pVertexDecl_Skinned );
    }

    return true;
}


void CleanupD3D()
{
    CHECKED_RELEASE(g_pEffect);
    CHECKED_RELEASE(g_pVertexDecl_Skinned);
    CHECKED_RELEASE(g_pVertexDecl_Rigid);
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

void SetTransposedMatrix4x4(D3DXHANDLE ShaderConstant,
                            float const *Matrix )
{
    float TMatrix[16];
    memcpy(TMatrix, Matrix, sizeof(TMatrix));
    TransposedMatrix4x4(TMatrix);

    g_pEffect->SetMatrix(ShaderConstant, (D3DXMATRIX const*)Matrix);
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


bool CreateDemoTextures()
{
    assert(GlobalScene.SceneFileInfo);
    assert(g_pD3DDevice);

    /* Iterate across all the textures */
    granny_file_info* Info = GlobalScene.SceneFileInfo;
    {for(granny_int32x TexIdx = 0; TexIdx < Info->TextureCount; ++TexIdx)
    {
        granny_texture* GrannyTexture = Info->Textures[TexIdx];
        assert(GrannyTexture);

        if (GrannyTexture->TextureType == GrannyColorMapTextureType &&
            GrannyTexture->ImageCount == 1)
        {
            /* We select the appropriate 8-bit/channel format based on whether or not the
               image has an alpha channel. */
            D3DFORMAT D3DTexFormat;
            granny_pixel_layout const* GrannyTexFormat;
            if (GrannyTextureHasAlpha(GrannyTexture))
            {
                D3DTexFormat = D3DFMT_A8R8G8B8;
                GrannyTexFormat = GrannyBGRA8888PixelFormat;
            }
            else
            {
                D3DTexFormat = D3DFMT_R8G8B8;
                GrannyTexFormat = GrannyBGR888PixelFormat;
            }

            /* Create and lock the texture surface, and use $CopyTextureImage to move the
               pixels.  This function will handle format conversion, if necessary,
               including adding or removing alpha components, decompressing from Bink or
               S3TC formats, etc. */
            DemoTexture* NewTex = new DemoTexture;
            g_pD3DDevice->CreateTexture(GrannyTexture->Width, GrannyTexture->Height,
                                        1,
                                        0, D3DTexFormat,
                                        D3DPOOL_MANAGED,
                                        &NewTex->TextureBuffer, NULL);
            if (!NewTex->TextureBuffer)
                return false;

            D3DLOCKED_RECT LockedRect;
            NewTex->TextureBuffer->LockRect(0, &LockedRect, 0, 0);

            GrannyCopyTextureImage(GrannyTexture, 0, 0,
                                   GrannyTexFormat,
                                   GrannyTexture->Width,
                                   GrannyTexture->Height,
                                   LockedRect.Pitch,
                                   LockedRect.pBits);

            NewTex->TextureBuffer->UnlockRect(0);


            /* Add the texture to our demo list, after copying the file name. */
            NewTex->Name = new char[strlen(GrannyTexture->FromFileName) + 1];
            strcpy(NewTex->Name, GrannyTexture->FromFileName);

            GlobalScene.Textures.push_back(NewTex);
        }
    }}

    /* Now that we've created all the surfaces required to texture our scene, we can get
       rid of the copy of the pixels still stored in the $file_info.  We'll free the file
       section that contains pixel data.  By default, the exporter creates a $file section
       exclusively for the pixel data ($StandardTextureSection), and stores the $texture
       metadata elsewhere.  So we can free the bulk data, but still make use of the Name,
       Height, Width, etc.

       Once we've freed the file section, we obviously can't access the pixels without
       reloading the file, in a real application, you will need a plan to handle lost
       surfaces, or similar events. */
    if (GlobalScene.SceneFile->SectionCount >= GrannyStandardTextureSection)
    {
        GrannyFreeFileSection(GlobalScene.SceneFile,
                              GrannyStandardTextureSection);
    }

    return true;
}

DemoMesh* CreateBoundMesh(granny_mesh* GrannyMesh,
                          granny_model_instance* ModelInstance)
{
    assert(GrannyMesh);
    assert(ModelInstance);
    assert(g_pD3DDevice);

    granny_model* SourceModel = GrannyGetSourceModel(ModelInstance);

    /* Create the binding.  $NewMeshBinding takes two $skeleton parameters to allow you to
       attach the mesh to a $skeleton other than the one it was modeled on.  Check
       $BindingAnimationsToMeshes_BindingMeshes for more details.  In this case, we're
       binding the mesh to the original $skeleton, so we just pass it twice. */
    DemoMesh* NewMesh = new DemoMesh;
    NewMesh->Mesh = GrannyMesh;
    NewMesh->MeshBinding = GrannyNewMeshBinding(GrannyMesh,
                                                SourceModel->Skeleton,
                                                SourceModel->Skeleton);

    /* Lookup the texture bindings in the list we created in CreateDemoTextures.  Note
       that this process can insert NULL pointers in to the MaterialBindings array, which
       we will handle in Render(). */
    {for(granny_int32x MatIdx = 0; MatIdx < GrannyMesh->MaterialBindingCount; ++MatIdx)
    {
        granny_material* Material = GrannyMesh->MaterialBindings[MatIdx].Material;
        DemoTexture* Found = NULL;
        if (Material && Material->MapCount >= 1)
        {
            {for(granny_int32x MapIdx = 0; MapIdx < Material->MapCount; ++MapIdx)
            {
                granny_material_map& Map = Material->Maps[MapIdx];

                if (stricmp(Map.Usage, "color") == 0 && Map.Material->Texture)
                {
                    {for(granny_uint32x i = 0; i < GlobalScene.Textures.size(); ++i)
                    {
                        if (stricmp(Map.Material->Texture->FromFileName, GlobalScene.Textures[i]->Name) == 0)
                        {
                            Found = GlobalScene.Textures[i];
                            break;
                        }
                    }}
                }
            }}
        }

        NewMesh->MaterialBindings.push_back(Found);
    }}

    /* Now create the d3d index buffer */
    D3DFORMAT IndexFormat;
    int BytesPerIndex;
    int IndexCount = GrannyGetMeshIndexCount(GrannyMesh);
    if (GrannyGetMeshBytesPerIndex(GrannyMesh) == 2)
    {
        IndexFormat = D3DFMT_INDEX16;
        BytesPerIndex = 2;
    }
    else
    {
        assert(GrannyGetMeshBytesPerIndex(GrannyMesh) == 4);
        IndexFormat = D3DFMT_INDEX32;
        BytesPerIndex = 4;
    }

    int const IndexBufferSize = BytesPerIndex * IndexCount;
    g_pD3DDevice->CreateIndexBuffer(IndexBufferSize,
                                    D3DUSAGE_WRITEONLY,
                                    IndexFormat,
                                    D3DPOOL_MANAGED,
                                    &NewMesh->IndexBuffer, NULL);
    if (NewMesh->IndexBuffer == 0)
    {
        delete NewMesh;
        return NULL;
    }

    BYTE *Indices;
    NewMesh->IndexBuffer->Lock(0, IndexBufferSize, (void**)&Indices, 0);
    memcpy(Indices, GrannyGetMeshIndices(GrannyMesh), IndexBufferSize);
    NewMesh->IndexBuffer->Unlock();

    /* And the vertex buffer.  You can examine the D3D format specifications that we
       selected for g_pVertexDecl_Rigid and g_pVertexDecl_Skinned in
       dx9_rendering_helpers.cpp.  We chose the formats to correspond to the Granny vertex
       formats $pnt332_vertex (rigid) and $pwnt3432_vertex (skinned).  You are not
       restricted to built-in formats, check the main Granny docs for details on how to
       create your own format.

       For skinned meshes, we also need to check at this point that the number of bone
       bindings in the $mesh doesn't exceed the number of constant register slots we have
       available in our vertex shader.  If a source mesh exceeds the number of constant
       registers available to you on your minimum spec hardware, you can use the Granny
       Preprocessor to break the mesh up into renderable chunks without imposing
       restrictions on your artists. */
    granny_data_type_definition* VertexFormat;
    int VertexBufferSize;
    if (GrannyMeshIsRigid(GrannyMesh))
    {
        VertexFormat = GrannyPNT332VertexType;
        VertexBufferSize = (sizeof(granny_pnt332_vertex) *
                            GrannyGetMeshVertexCount(GrannyMesh));
    }
    else
    {
        VertexFormat = GrannyPWNT3432VertexType;
        VertexBufferSize = (sizeof(granny_pwnt3432_vertex) *
                            GrannyGetMeshVertexCount(GrannyMesh));
    }

    g_pD3DDevice->CreateVertexBuffer(VertexBufferSize,
                                     D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED,
                                     &NewMesh->VertexBuffer, NULL);
    if (NewMesh->VertexBuffer == 0)
    {
        delete NewMesh;
        return NULL;
    }

    BYTE *Vertices;
    NewMesh->VertexBuffer->Lock(0, VertexBufferSize, (void**)&Vertices, 0);
    GrannyCopyMeshVertices(GrannyMesh, VertexFormat, Vertices);
    NewMesh->VertexBuffer->Unlock();

    return NewMesh;
}

bool CreateDemoModels()
{
    assert(GlobalScene.SceneFileInfo);
    assert(g_pD3DDevice);

    /* Iterate across all the models */
    granny_file_info* Info = GlobalScene.SceneFileInfo;
    {for(granny_int32x ModelIdx = 0; ModelIdx < Info->ModelCount; ++ModelIdx)
    {
        granny_model* GrannyModel = Info->Models[ModelIdx];
        assert(GrannyModel);

        if (GrannyModel->MeshBindingCount > 0)
        {
            DemoModel* NewModel = new DemoModel;
            GlobalScene.Models.push_back(NewModel);

            NewModel->ModelInstance = GrannyInstantiateModel(GrannyModel);
            GrannyBuildCompositeTransform4x4(&GrannyModel->InitialPlacement,
                                             NewModel->InitialMatrix);

            /* Create the meshes that are bound to this model. */
            {for(granny_int32x MeshIdx = 0;
                 MeshIdx < GrannyModel->MeshBindingCount;
                 ++MeshIdx)
            {
                granny_mesh* GrannyMesh = GrannyModel->MeshBindings[MeshIdx].Mesh;
                assert(GrannyMesh);

                DemoMesh* NewMesh = CreateBoundMesh(GrannyMesh, NewModel->ModelInstance);
                assert(NewMesh);
                NewModel->BoundMeshes.push_back(NewMesh);
            }}

            /* We're going to use a "fire-and-forget" animation technique here, since
               we're simply going to loop a single animation until the application
               finishes.  Once we create the control, we tell Granny that it's allowed to
               free it when we free the $model_instance, and set the loop count to 0 to
               indicate that we want it to loop forever. */
            {for(granny_int32x AnimIdx = 0; AnimIdx < Info->AnimationCount; ++AnimIdx)
            {
                granny_animation* Animation = Info->Animations[AnimIdx];

                // Simple control, starting at t = 0.0f
                granny_control *Control =
                    GrannyPlayControlledAnimation(0.0f, Animation,
                                                  NewModel->ModelInstance);
                if (Control)
                {
                    // Loop forever
                    GrannySetControlLoopCount(Control, 0);

                    // Allow Granny to free the control when we delete the instance
                    GrannyFreeControlOnceUnused(Control);

                    // Only bind one animation to the model
                    break;
                }
            }}
        }
    }}

    return true;
}

bool CreateSharedPoses()
{
    GlobalScene.MaxBoneCount = -1;
    {for(granny_uint32x i = 0; i < GlobalScene.Models.size(); ++i)
    {
        granny_skeleton* Skeleton =
            GrannyGetSourceSkeleton(GlobalScene.Models[i]->ModelInstance);

        if (Skeleton->BoneCount > GlobalScene.MaxBoneCount)
            GlobalScene.MaxBoneCount = Skeleton->BoneCount;
    }}

    if (GlobalScene.MaxBoneCount != -1)
    {
        GlobalScene.SharedLocalPose = GrannyNewLocalPose(GlobalScene.MaxBoneCount);

        // More on this later (in the Render() description).  GrannyNewWorldPose would work
        // equally well here.
        GlobalScene.SharedWorldPose =
            GrannyNewWorldPose(GlobalScene.MaxBoneCount);

        return true;
    }
    else
    {
        // Very odd.  An error loading the file most likely.
        return false;
    }
}

bool InitCameraAndLights()
{
    assert(GlobalScene.SceneFileInfo);

    GrannyInitializeDefaultCamera(&GlobalScene.DemoCamera);
    GlobalScene.DemoCamera.NearClipPlane = 0.1f;
    GlobalScene.DemoCamera.FarClipPlane  = 1000.f;
    GlobalScene.DemoCamera.FOVY = 0.2f * 3.14f;
    GlobalScene.DemoCamera.Offset[1] = 25.0f;
    GlobalScene.DemoCamera.Offset[2] = 200.0f;
    GlobalScene.DemoCamera.ElevAzimRoll[1] = -0.8f;

    /* Setup our hardcoded light.  This points it in the same general direction as the camera, but offset slightly. */
    GlobalScene.DirFromLight[0]  = -0.8660f;
    GlobalScene.DirFromLight[1]  = 0.5f;
    GlobalScene.DirFromLight[2]  = 0;
    GlobalScene.LightColour[0]   = 0.8f;
    GlobalScene.LightColour[1]   = 0.8f;
    GlobalScene.LightColour[2]   = 0.8f;
    GlobalScene.LightColour[3]   = 0.8f;
    GlobalScene.AmbientColour[0] = 0.2f;
    GlobalScene.AmbientColour[1] = 0.2f;
    GlobalScene.AmbientColour[2] = 0.2f;
    GlobalScene.AmbientColour[3] = 0.2f;

    return true;
}

void Render(int Width, int Height)
{
    if (!GlobalRunning)
        return;
    assert(g_pD3DDevice);

    /* Note that I set the camera aspect ratios every frame, because the width and height
       of the window may have changed which affects the aspect ratio correction.  However,
       if you're a full screen game and you know when you're changing screen modes and
       such, then you'd only have to call GrannySetCameraAspectRatios() when you actually
       change modes. */
    GrannySetCameraAspectRatios(&GlobalScene.DemoCamera,
                                GrannyGetMostLikelyPhysicalAspectRatio(Width, Height),
                                (float)Width, (float)Height,
                                (float)Width, (float)Height);
    GrannyBuildCameraMatrices(&GlobalScene.DemoCamera);

    /* Clear the backbuffer to a blue color, and start rendering our scene */
    g_pD3DDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,
                         D3DCOLOR_XRGB(0,0,255), 1.0f, 0L );

    if(SUCCEEDED(g_pD3DDevice->BeginScene()))
    {
        /* Setup the camera and lighting parameters, which don't change over the course of
           the scene.  Note that Granny matrices are column-major, by default, while D3D
           matrices are row-major, which is why we call a utility function for the set
           here. */
        SetTransposedMatrix4x4(g_hMatWorld2View, (FLOAT*)GlobalScene.DemoCamera.View4x4 );
        SetTransposedMatrix4x4(g_hMatView2Clip, (FLOAT*)GlobalScene.DemoCamera.Projection4x4 );
        g_pEffect->SetFloatArray(g_hDirFromLight, (FLOAT*)GlobalScene.DirFromLight, 3);
        g_pEffect->SetFloatArray(g_hLightColor, (FLOAT*)GlobalScene.LightColour, 3);
        g_pEffect->SetFloatArray(g_hAmbientColor, (FLOAT*)GlobalScene.AmbientColour, 3);

        g_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

        // Loop over the models, and render their meshes
        for (size_t Idx = 0; Idx < GlobalScene.Models.size(); Idx++)
        {
            RenderModel(GlobalScene.Models[Idx]);
        }
        g_pD3DDevice->EndScene();
    }

    // Present the backbuffer contents to the display
    g_pD3DDevice->Present( NULL, NULL, NULL, NULL );
}


INT WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, INT )
{
    // Check the versions
    if(!GrannyVersionsMatch)
    {
        printf("Warning: the Granny DLL currently loaded "
               "doesn't match the .h file used during compilation\n");
        return EXIT_FAILURE;
    }

    /* Create the D3D objects */
    if (InitializeD3D() == false)
    {
        printf("Error: unable to initialize D3D device.  "
               "DX9 compliant renderer required.\n");
        return EXIT_FAILURE;
    }

    /* Load the scene file, and bind the meshes, textures, etc. */
    {
        GlobalScene.SceneFile = GrannyReadEntireFile(GlobalSceneFilename);
        if (GlobalScene.SceneFile == NULL)
            return EXIT_FAILURE;

        GlobalScene.SceneFileInfo = GrannyGetFileInfo(GlobalScene.SceneFile);
        if (GlobalScene.SceneFileInfo == NULL)
            return EXIT_FAILURE;
    }

    /* Create our demo objects.  Note that the order is important here, CreateDemoModels
       will be looking up textures as it iterates, and CreateSharedPoses will scan the
       created models. */
    if (CreateDemoTextures() == false ||
        CreateDemoModels()   == false ||
        CreateSharedPoses()  == false ||
        InitCameraAndLights() == false)
    {
        return EXIT_FAILURE;
    }


    /* Mark the start time so we can compute update intervals */
    granny_system_clock StartClock;
    GrannyGetSystemSeconds(&StartClock);
    granny_system_clock LastClock = StartClock;
    GlobalScene.CurrentTime = 0.0f;

    /* Enter our "game" loop.  Each frame, we compute the current time, and the time
       elapsed since the last frame, and call Update to advance the model animations.  The
       rest is simply Win32 boilerplate to keep the message pump filled. */
    GlobalRunning = true;
    while (GlobalRunning)
    {
        // Extract the current time and the frame delta
        granny_real32 CurrentTime, DeltaTime;
        {
            granny_system_clock CurrClock;
            GrannyGetSystemSeconds(&CurrClock);

            // Ignore clock recentering issues for this example
            CurrentTime = GrannyGetSecondsElapsed(&StartClock, &CurrClock);
            DeltaTime   = GrannyGetSecondsElapsed(&LastClock, &CurrClock);
            LastClock = CurrClock;
        }

        // Update the world
        GlobalScene.CurrentTime = CurrentTime;

        InvalidateRect(g_hwnd, 0, false);
        MSG Message;
        while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
    }

    CleanupD3D();
    return EXIT_SUCCESS;
}



void RenderModel(DemoModel* Model)
{
    /* We sample the model's animations "just in time", delaying until we know that we're
       going to render the $model_instance.  In the case of this demo app, we render all
       of the models, all of the time, but if you cull at the object level, you can avoid
       doing a lot of work by simply never calling $SampleModelAnimations (or similar
       functions) on the culled $model_instance.  Here, since we don't need the
       $local_pose, we use $SampleModelAnimationsAccelerated to build the $world_pose in
       whatever Granny decides is the most efficient manner.

       We've enabled "move to origin" for all these matrices (which is a very good idea,
       see: $RootMotion), so we need to send in a matrix representing the initial
       placement to ensure that the models show up in the expected location.  We computed
       that when we created the DemoModel object.  */
    ComputeLegPosition(Model);

    /* Iterate across all meshes */
    {for (size_t MeshIndex = 0; MeshIndex < Model->BoundMeshes.size(); ++MeshIndex)
    {
        DemoMesh* Mesh = Model->BoundMeshes[MeshIndex];

        // Load the mesh's index buffer.
        g_pD3DDevice->SetIndices(Mesh->IndexBuffer);

        /* The ToBoneIndices holds the mapping from the contiguously index mesh bones to
           original source indices in the bound skeleton.  This is necessary for both
           rigid and skinned meshes.  There is an analogous call to get the mapping to the
           source skeleton, $GetMeshBindingFromBoneIndices, but you'll almost always want
           to use $GetMeshBindingToBoneIndices.  In our case, the two mappings are
           identical, but this is not always true. */
        int const *ToBoneIndices =
            GrannyGetMeshBindingToBoneIndices(Mesh->MeshBinding);

        if (GrannyMeshIsRigid(Mesh->Mesh))
        {
            /* It's a rigid mesh, so load the appropriate technique.  Note that this is
               pretty slow, normally you'd order your meshes to minimize the number of
               state switches. */
            g_pEffect->SetTechnique(g_RigidTechnique);
            g_pD3DDevice->SetVertexDeclaration( g_pVertexDecl_Rigid );
            g_pD3DDevice->SetStreamSource(0, Mesh->VertexBuffer, 0,
                                          sizeof(granny_pnt332_vertex));

            /* Now I look up the transform for this mesh, and load it.  (Note that we're
               making the assumption that Bone 0 represents the rigid transform for the
               model.  In most cases, this is a safe assumption.) */
            granny_matrix_4x4 CompositeMatrix;
            GrannyBuildIndexedCompositeBuffer(
                GrannyGetMeshBindingToSkeleton(Mesh->MeshBinding),
                GlobalScene.SharedWorldPose,
                ToBoneIndices, 1,
                &CompositeMatrix);

            g_pEffect->SetMatrix(g_hMatObj2World, (D3DXMATRIX const*)CompositeMatrix);
        }
        else
        {
            /* It's a skinned mesh, activate the appropriate technique. */
            int const NumMeshBones = GrannyGetMeshBindingBoneCount(Mesh->MeshBinding);
            assert(NumMeshBones <= 80);

            g_pEffect->SetTechnique(g_SkinnedTechnique);
            g_pD3DDevice->SetVertexDeclaration( g_pVertexDecl_Skinned );

            /* Our source vertices are set as stream source 0 */
            g_pD3DDevice->SetStreamSource(0, Mesh->VertexBuffer, 0,
                                          sizeof(granny_pwnt3432_vertex));

            /* Load the matrices into the constant registers.  This is about the simplest
               way to go about this.  Remember that I said we'd be talking about the call
               to $NewWorldPoseNoComposite?  Here's the payoff.  When you call that
               routine, it creates a $world_pose without a Composite array attached, which
               allows Granny to skip those matrices when $BuildWorldPose is called.  (You
               can get the same effect conditionally by calling $BuildWorldPoseNoComposite
               for $(world_pose)s that do have the composite array.)  Since we're just
               going to copy a few of the matrices into the constant buffer, we delay
               until this point, at which we know exactly which matrices we need, and
               exactly where they need to go.  $BuildIndexedCompositeBuffer is designed to
               do this for you.  (If you need the matrices transposed, use
               $BuildIndexedCompositeBufferTransposed.)

               The static buffer is necessary, since you cannot lock a constant buffer in
               DirectX 9.  (Wait for DX10, the most efficient way to do this will change
               again.)  We know that no mesh in our scene will bind to more than 80 bones,
               since we've preprocessed the data to ensure that all meshes are broken up
               into pieces of at most that size. */
            granny_matrix_4x4 CompositeBuffer[80];

            GrannyBuildIndexedCompositeBuffer(
                GrannyGetMeshBindingToSkeleton(Mesh->MeshBinding),
                GlobalScene.SharedWorldPose,
                ToBoneIndices, NumMeshBones,
                CompositeBuffer);

            g_pEffect->SetMatrixArray(g_hSkinningMatrices,
                                      (D3DXMATRIX const*)CompositeBuffer,
                                      NumMeshBones);
        }

        /* Now both the indices and vertices are loaded, so I can render.  Grab the
           material groups and spin over them, changing to the appropriate texture and
           rendering each batch.  A more savvy rendering loop might have instead built a
           sorted list of material groups to minimize texture changes, etc., but this is
           the most basic way to render. */
        int const VertexCount = GrannyGetMeshVertexCount(Mesh->Mesh);
        int GroupCount = GrannyGetMeshTriangleGroupCount(Mesh->Mesh);
        granny_tri_material_group *Group =
            GrannyGetMeshTriangleGroups(Mesh->Mesh);
        while(GroupCount--)
        {
            if(Group->MaterialIndex < int(Mesh->MaterialBindings.size()) &&
               Mesh->MaterialBindings[Group->MaterialIndex] != NULL)
            {
                DemoTexture* Texture = Mesh->MaterialBindings[Group->MaterialIndex];
                g_pEffect->SetTexture(g_hDiffuseTexture, Texture->TextureBuffer);
            }
            else
            {
                g_pEffect->SetTexture(g_hDiffuseTexture, NULL);
            }

            // We know that there's only one pass, but let's be general about this
            UINT numPasses;
            g_pEffect->Begin(&numPasses, 0);
            for (UINT i = 0; i < numPasses; i++)
            {
                g_pEffect->BeginPass(i);

                g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
                                                   0,
                                                   0, VertexCount,
                                                   3*Group->TriFirst,
                                                   Group->TriCount);

                g_pEffect->EndPass();
            }
            g_pEffect->End();

            ++Group;
        }
    }}
}
