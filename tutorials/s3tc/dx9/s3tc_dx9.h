#if !defined(S3TC_DX9_H)
// ========================================================================
// $File: //jeffr/granny/tutorial/s3tc/dx9/s3tc_dx9.h $
// $DateTime: 2006/11/29 15:58:12 $
// $Change: 13818 $
// $Revision: #1 $
//
// (C) Copyright 1999-2007 by RAD Game Tools, All Rights Reserved.
// ========================================================================
#include "granny.h"
#include <d3d9.h>
#include <vector>

// ---------------------------------------------------------
// Global D3D Objects
extern IDirect3D9* g_pD3D;
extern IDirect3DDevice9* g_pD3DDevice;
extern IDirect3DTexture9* g_pTexture;

// ---------------------------------------------------------
// Global Win32 objects
extern const char* MainWindowTitle;
extern HWND g_hwnd;
extern bool GlobalRunning;

// ---------------------------------------------------------
// Handy functions.
bool InitializeD3D();
void CleanupD3D();
void Render();

LRESULT CALLBACK MainWindowCallback(HWND Window, UINT Message,
                                    WPARAM WParam, LPARAM LParam);


#define S3TC_DX9_H
#endif
