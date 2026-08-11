// NFS Hot Pursuit 2 Widescreen Fix
// TODO: reduce & optimize expensive strcmp's
// TODO: tidy up the code a bit more

#include "stdafx.h"
#include "stdio.h"
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <direct.h>
#include <windows.h>
#include <shlobj.h>
#include "includes/injector/injector.hpp"
#include "includes/inireader.h"

#ifdef HP2DEBUG
#include "HP2_DBG_Addresses.h"
#else
#include "HP2_242_Addresses.h"
#endif

#define FOUR_BY_THREE_ASPECT 1.3333333333333333333333333333333
#define LETTERBOX_ASPECT 2.1052631578947368421052631578947
#define INTRO_LETTERBOX_ASPECT 2.9411764705882352941176470588235

void InjectRes();
int InitRenderCaps();

volatile int resX = 1280;
volatile int resY = 720;
volatile float resX_43f = 960.0;
volatile float resX_600height = 1067.0;
volatile float resX_480height = 853.333334;
volatile float aspect;
volatile float aspect_diff = 0.75;
volatile float xscale_800 = 1.33375;
volatile float xscale_640 = FOUR_BY_THREE_ASPECT;

char UserDir[MAX_PATH];
char RenderCapsIni[MAX_PATH];
char MkDirStr[MAX_PATH + 7];
char* CurrentFEElement;
char* CurrentFEShapePointer;
char CurrentFEShape[255];
bool bEnableConsole = false;
bool bDisableLetterboxing = false;
bool bUseModernHUD = false;
bool bAlternativeBackground = false;
int bAspectRatioFix = 2;
unsigned int RenderMemorySize = 0x732000;
unsigned int GeneralMemorySize = 0x5FB9000;
unsigned int AudioMemorySize = 0xA00000;
unsigned int TrackMemorySize = 0x2B00000;
unsigned int LevelMemorySize = 0x196000;
unsigned int UIMemorySize = 0x400000;
unsigned int CarsMemorySize = 0x700000;
unsigned int CharacterMemorySize = 0x17D000;
unsigned int ReplayMemorySize = 0x64000;
unsigned int IniFileMemorySize = 0x10000;

float FE_horscale = 1.0;
float FE_horposition = 0.0;
int OptionsMenuXsize = 650;


// coord offsets for HUD elements
int Tach_XOffset = 0;
int Map_XOffset = 0;
int Time_XOffset = 0;
int RVM_XOffset = 0;
int Pos_XOffset = 0;
int Cuffo_XOffset = 0;
int CopAmmo_XOffset = 0;
int Score_XOffset = 0;
int Dash_XOffset = 0;

int Tach_YOffset = 0;
int Map_YOffset = 0;
int Time_YOffset = 0;
int RVM_YOffset = 0;
int Pos_YOffset = 0;
int Cuffo_YOffset = 0;
int CopAmmo_YOffset = 0;
int Score_YOffset = 0;
int Dash_YOffset = 0;


int(__thiscall*ReadIni_Float)(unsigned int dis, char *section, char *key, float* value) = (int(__thiscall*)(unsigned int, char*, char*, float*))0x00527650;
int(__thiscall*ReadIni_Int)(unsigned int dis, char *section, char *key, int* value) = (int(__thiscall*)(unsigned int, char*, char*, int*))0x00527560;
int(__thiscall*WriteRenderCaps)(unsigned int dis) = (int(__thiscall*)(unsigned int))0x005410D0;
int(__thiscall*sub_59B840)(unsigned int dis, char *key, unsigned int unk1, unsigned int unk2) = (int(__thiscall*)(unsigned int, char*, unsigned int, unsigned int))0x59B840;
int(__thiscall*sub_5994D0)(unsigned int dis, unsigned int unk1) = (int(__thiscall*)(unsigned int, unsigned int))0x5994D0;
int(__thiscall*sub_5997B0)(unsigned int dis) = (int(__thiscall*)(unsigned int))0x5997B0;


struct UnkClass1
{
	void* vtable;
	int x;
	int y;
};

struct mBorders
{
	int unk;
	int topY;
	int topX;
	int botX;
	int botY;
};

void GetDesktopRes(volatile int* desktop_resX, volatile int* desktop_resY)
{
	HMONITOR monitor = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTONEAREST);
	MONITORINFO info = {};
	info.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &info);
	*desktop_resX = info.rcMonitor.right - info.rcMonitor.left;
	*desktop_resY = info.rcMonitor.bottom - info.rcMonitor.top;
}

volatile float RecalculateFOV_4by3(float InFOV)
{
	float hfovRad = InFOV * M_PI / 180.0;
	float vfovRad = 2.0f*atan(tan(hfovRad*0.5) / (4.0 / 3.0));
	float newfovRad = 2.0f*atan(aspect*tan(vfovRad*0.5f));
	float vfov = (vfovRad * 180.0) / M_PI;
	float newfov = (newfovRad * 180.0) / M_PI;

	return newfov;
}

bool CheckForPathAbsolution(const char* input)
{
	if ((input[0] == '.') || (input[1] == '\\') || (input[1] == ':') || (input[2] == '\\'))
		return true;

	return false;
}

int __stdcall sub_59BAE0_hook(int a2, int a3)
{
	int v3;      // eax@1
	int result;  // eax@2
	unsigned int thethis = 0;
	_asm mov thethis, ecx

	v3 = (*(int(__stdcall **)(int))(*(int*)thethis + 20))(a2);
	CurrentFEShapePointer = *(char**)(v3 + 4);
	strcpy(CurrentFEShape, CurrentFEShapePointer);

	// alternative titlescreen graphics start
	if (bAlternativeBackground)
		if (strcmp(CurrentFEShapePointer, "[PC:S_INTRO.LYR.PCD] back") == 0)
			strcpy(CurrentFEShapePointer, "[PC:S_FEAT.LYR.PCD] feat");

	// alternative titlescreen graphics end
	if (*(int*)(v3 + 12))
	{
		sub_5994D0(a3, v3);
		result = 1;
	}
	else
	{
		sub_5997B0(a3);
		result = 0;
	}
	return result;
}

int __cdecl sub_5954A0(int a1, int a2)
{
	float v8;  // [sp+1Ch] [bp+Ch]@1
	float v9;  // [sp+1Ch] [bp+Ch]@1
	float v10; // [sp+1Ch] [bp+Ch]@1
	float v11; // [sp+1Ch] [bp+Ch]@1

	v8 = (double)(*(int*)(a2 + 4) + *(int*)(a2 + 16)) * *(float*)FE_FUNCTIONS_YSCALE_ADDRESS - *(float*)FE_YPOS_ADDRESS;
	v10 = (double)*(signed int *)(a2 + 4) * *(float*)FE_FUNCTIONS_YSCALE_ADDRESS - *(float*)FE_YPOS_ADDRESS;

	// the subtraction by the FE X position causes weird text bugs... removing that fixes text
	v9 = (double)(*(int*)(a2 + 8) + *(int*)(a2 + 12)) * *(float*)FE_FUNCTIONS_XSCALE_ADDRESS;
	v11 = (double)*(signed int *)(a2 + 8) * *(float*)FE_FUNCTIONS_XSCALE_ADDRESS;

	*(int*)(a1 + 8) = (int)v11;
	*(int*)(a1 + 12) = (int)(v9 - v11);
	*(int*)a1 = 0x65E010;
	*(int*)(a1 + 4) = (int)v10;
	*(int*)(a1 + 16) = (int)(v8 - v10);
	return a1;
}

UnkClass1* __cdecl FE_CursorPos(UnkClass1* out, UnkClass1* in)
{
	float v5;
	float v6;

	v6 = (*in).x * *(float*)FE_FUNCTIONS_XSCALE_ADDRESS - (*(float*)FE_XPOS_ADDRESS / *(float*)FE_XSCALE_ADDRESS);
	v5 = (*in).y * *(float*)FE_FUNCTIONS_YSCALE_ADDRESS - *(float*)FE_YPOS_ADDRESS;

	(*out).vtable = (void*)0x65E1F8;
	(*out).x = (int)v6;
	(*out).y = (int)v5;
	return out;
}

int __stdcall sub_59B840_hook(char *key, mBorders* unk1, unsigned int unk2)
{
	unsigned int thethis = 0;
	_asm mov thethis, ecx
	unsigned int retval1 = sub_59B840(thethis, key, (int)unk1, unk2);

	if (bAspectRatioFix > 1)
		(*unk1).botX = resX_600height;

	if (bAspectRatioFix == 1)
		*(float*)FE_XPOS_ADDRESS = FE_horposition;

	return retval1;
}

int __stdcall sub_59B840_hook_2(char *key, mBorders* unk1, unsigned int unk2) // hook for fixing specific UI elements' mBorders
{
	unsigned int thethis = 0;
	_asm mov thethis, ecx
	CurrentFEElement = *(char**)(thethis + 8);
	unsigned int retval = sub_59B840(thethis, key, (int)unk1, unk2);
	bool ASPECT_DIFF_GREATHAN = aspect > ((float)1280.0 / (float)720.0);

	if (bAspectRatioFix > 1)
	{
		(*unk1).topX *= xscale_800;
		if ((*unk1).botX == 800)
			(*unk1).botX = resX_600height;
		if ((*unk1).botX == 640)
			(*unk1).botX = resX_480height;

		// SLIDER FIX START
		// sliders are a bit of their own kind, their boundary box has to increase with the screen size in order to draw properly
		if (strcmp(strrchr(CurrentFEElement, '.'), ".CmnSlider") == 0)
		{
			(*unk1).botX *= xscale_800;
			(*unk1).botX += 2;
		}
		if (strcmp(CurrentFEShape, "[PC:C_CNTRLS01.TPG.PCD] slBG") == 0)
			(*unk1).botX *= xscale_800;
		// SLIDER FIX END

		// fix for hud - this will have to be coded specially for every element that is broken...
		if (strcmp(CurrentFEElement, "s1.HudText") == 0)
		{
			(*unk1).topX = resX_600height - 82;
			if (bUseModernHUD)
			{
				(*unk1).topX -= Time_XOffset;
				(*unk1).topY = 64 + Time_YOffset;
			}
			else
				(*unk1).topX -= 48;
		}
		if (strcmp(CurrentFEElement, "s2.HudText") == 0)
		{
			(*unk1).topX = resX_600height - 82;
			if (bUseModernHUD)
			{
				(*unk1).topX -= Time_XOffset;
				(*unk1).topY = 81 + Time_YOffset;
			}
			else
				(*unk1).topX -= 48;
		}
		if (strcmp(CurrentFEElement, "s3.HudText") == 0)
		{
			(*unk1).topX = resX_600height - 82;
			if (bUseModernHUD)
			{
				(*unk1).topX -= Time_XOffset;
				(*unk1).topY = 98 + Time_YOffset;
			}
			else
				(*unk1).topX -= 48;
		}
		if (strcmp(CurrentFEElement, "sl1.HudText") == 0)
		{
			(*unk1).topX = resX_600height - (*unk1).botX - 98;
			if (bUseModernHUD)
			{
				(*unk1).topX -= Time_XOffset;
				(*unk1).topY = 64 + Time_YOffset;
			}
			else
				(*unk1).topX -= 48;
		}
		if (strcmp(CurrentFEElement, "sl2.HudText") == 0)
		{
			(*unk1).topX = resX_600height - (*unk1).botX - 96;
			if (bUseModernHUD)
			{
				(*unk1).topX -= Time_XOffset;
				(*unk1).topY = 81 + Time_YOffset;
			}
			else
				(*unk1).topX -= 48;
		}
		if (strcmp(CurrentFEElement, "sl3.HudText") == 0)
		{
			(*unk1).topX = resX_600height - (*unk1).botX - 97;
			if (bUseModernHUD)
			{
				(*unk1).topX -= Time_XOffset;
				(*unk1).topY = 98 + Time_YOffset;
			}
			else
				(*unk1).topX -= 48;
		}

		if (strcmp(CurrentFEElement, "p1.HudText") == 0)
		{
			if (bUseModernHUD)
			{
				(*unk1).topX = (resX_600height - 120) - Pos_XOffset;
				(*unk1).topY = Pos_YOffset;
			}
			else
				(*unk1).topX = 40;
		}
		if (strcmp(CurrentFEElement, "p2.HudText") == 0)
		{
			if (bUseModernHUD)
			{
				(*unk1).topX = (resX_600height - 72) - Pos_XOffset;
				(*unk1).topY = 8 + Pos_YOffset;
			}
			else
				(*unk1).topX = 88;
		}

		if (strcmp(CurrentFEElement, "score.HudText") == 0)
		{
			if (bUseModernHUD)
			{
				(*unk1).topX = Score_XOffset;
				(*unk1).topY = 16 + Score_YOffset;
			}
			else
				(*unk1).topX = 4;
		}
		if (strcmp(CurrentFEElement, "map.HudMap") == 0)
		{
			if (bUseModernHUD)
			{
				(*unk1).topX = 2 + Map_XOffset;
				(*unk1).topY = 473 - Map_YOffset;
			}
			else
				(*unk1).topX = 6;
		}

		if (strcmp(CurrentFEElement, "<TEXT>.FlashText") == 0)
			if ((*unk1).topX == 0 && (*unk1).topY == 125 && (*unk1).botX == 142 && (*unk1).botY == 12)
			{
				(*unk1).topX = 0;
				if (bUseModernHUD)
					(*unk1).topY = -16;
			}

		if (strcmp(CurrentFEElement, "bi1.HudStaticArt") == 0)
		{
			if (bUseModernHUD)
			{
				(*unk1).topX = (resX_600height - 108) - CopAmmo_XOffset;
				(*unk1).topY = 2 + CopAmmo_YOffset;
			}
			else
				(*unk1).topX = 38;
		}
		if (strcmp(CurrentFEElement, "bi2.HudStaticArt") == 0)
		{
			if (bUseModernHUD)
			{
				(*unk1).topX = (resX_600height - 108) - CopAmmo_XOffset;
				(*unk1).topY = 21 + CopAmmo_YOffset;
			}
			else
				(*unk1).topX = 38;
		}
		if (strcmp(CurrentFEElement, "bi3.HudStaticArt") == 0)
		{
			if (bUseModernHUD)
			{
				(*unk1).topX = (resX_600height - 108) - CopAmmo_XOffset;
				(*unk1).topY = 40 + CopAmmo_YOffset;
			}
			else
				(*unk1).topX = 38;
		}

		if (strcmp(CurrentFEElement, "bt1.HudText") == 0)
		{
			if (bUseModernHUD)
			{
				(*unk1).topX = (resX_600height - 108) - CopAmmo_XOffset;
				(*unk1).topY = 40 + CopAmmo_YOffset;
			}
			else
				(*unk1).topX = 38;
		}
		if (strcmp(CurrentFEElement, "bt2.HudText") == 0)
		{
			if (bUseModernHUD)
			{
				(*unk1).topX = (resX_600height - 108) - CopAmmo_XOffset;
				(*unk1).topY = 21 + CopAmmo_YOffset;
			}
			else
				(*unk1).topX = 38;
		}
		if (strcmp(CurrentFEElement, "bt3.HudText") == 0)
		{
			if (bUseModernHUD)
			{
				(*unk1).topX = (resX_600height - 108) - CopAmmo_XOffset;
				(*unk1).topY = 2 + CopAmmo_YOffset;
			}
			else
				(*unk1).topX = 38;
		}

		// RVM start
		if (strcmp(CurrentFEElement, "rearview.HudRearView") == 0)
		{
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
			if (bUseModernHUD)
			{
				(*unk1).topX -= RVM_XOffset;
				(*unk1).topY = 4 + RVM_YOffset;
			}
			else
				(*unk1).topX -= 14;
		}
		if (strcmp(CurrentFEShape, "[PC:H_MIRROR.TPG.PCD] edge") == 0)
		{
			if (strcmp(CurrentFEElement, "i0.GStaticImage") == 0)
			{
				(*unk1).topX = -8;
				if (bUseModernHUD)
					(*unk1).topY = -3;
			}
			if (strcmp(CurrentFEElement, "i0a.GStaticImage") == 0)
			{
				(*unk1).topX = 199;
				if (bUseModernHUD)
					(*unk1).topY = -3;
			}
		}
		// RVM end

		// tach start
		if (strcmp(CurrentFEElement, "digitach.HudTachometer") == 0)
		{
			(*unk1).topX = resX_600height - 60;
			(*unk1).topY += 20; // this will need fixing if we're gonna do 1:1 scaling...

			if (bUseModernHUD)
			{
				(*unk1).topX -= Tach_XOffset;
				(*unk1).topY -= Tach_YOffset;
			}
			else
			{
				(*unk1).topX -= 24;
				(*unk1).topY -= 24;
			}
		}
		if (strcmp(CurrentFEShape, "[PC:H_TACH.TPG.PCD] bck1") == 0)
			(*unk1).topX = -72;
		if (strcmp(CurrentFEElement, "5500.GSimpleImage") == 0)
		{
			(*unk1).topX = -24;
			(*unk1).topY = 36;
		}
		if (strcmp(CurrentFEElement, "6000.GSimpleImage") == 0)
		{
			(*unk1).topX = -17;
			(*unk1).topY = 34;
		}
		if (strcmp(CurrentFEElement, "6500.GSimpleImage") == 0)
		{
			(*unk1).topX = -10;
			(*unk1).topY = 32;
		}
		if (strcmp(CurrentFEElement, "7000.GSimpleImage") == 0)
		{
			(*unk1).topX = -2;
			(*unk1).topY = 32;
		}
		if (strcmp(CurrentFEElement, "7500.GSimpleImage") == 0)
		{
			(*unk1).topX = 6;
			(*unk1).topY = 32;
		}
		if (strcmp(CurrentFEElement, "8000.GSimpleImage") == 0)
		{
			(*unk1).topX = 12;
			(*unk1).topY = 32;
		}
		if (strcmp(CurrentFEElement, "8500.GSimpleImage") == 0)
		{
			(*unk1).topX = 19;
			(*unk1).topY = 34;
		}
		if (strcmp(CurrentFEElement, "9000.GSimpleImage") == 0)
		{
			(*unk1).topX = 26;
			(*unk1).topY = 36;
		}
		if (strcmp(CurrentFEElement, "speed.HudText") == 0)
			if ((*unk1).botX == 84 && (*unk1).botY == 17)
				(*unk1).topX = -35;
		if (strcmp(CurrentFEElement, "gear.HudText") == 0)
			if ((*unk1).botX == 36 && (*unk1).botY == 16)
				(*unk1).topX = 4;
		// tach end

		// dash start
		if (strcmp(CurrentFEElement, "dash.HudDashboard") == 0)
		{
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
			if (bUseModernHUD)
			{
				(*unk1).topX += Dash_XOffset;
				(*unk1).topY -= 6 - Dash_YOffset;
				if (Dash_YOffset < Cuffo_YOffset)
					(*unk1).topY -= 18 + Cuffo_YOffset;
			}
		}
		if (strcmp(CurrentFEElement, "speed.HudText") == 0)
			if (!((*unk1).botX == 84 && (*unk1).botY == 17))
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "gear.HudText") == 0)
			if (!((*unk1).botX == 36 && (*unk1).botY == 16))
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "speedo.HudSpeedometer") == 0)
			(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "tach.HudTachometer") == 0)
			(*unk1).topX /= xscale_800;
		// dash end

		// EA Trax Chyron start
		if (strcmp(CurrentFEElement, "<ARTIST>.GText") == 0)
			(*unk1).topX = 0;
		if (strcmp(CurrentFEElement, "<TITLE>.GText") == 0)
			(*unk1).topX = 0;
		if (strcmp(CurrentFEElement, "<ALBUM>.GText") == 0)
			(*unk1).topX = 0;
		if (strcmp(CurrentFEElement, "<BRAND>.GText") == 0)
			(*unk1).topX = 0;
		if (strcmp(CurrentFEElement, "b1.GImageBox") == 0)
			if ((*unk1).botX == 175 && (*unk1).botY == 17)
				(*unk1).topX = -2;
		if (strcmp(CurrentFEElement, "b2.GImageBox") == 0)
			if ((*unk1).botX == 175 && (*unk1).botY == 17)
				(*unk1).topX = -2;
		if (strcmp(CurrentFEElement, "a1.GImageBox") == 0)
			if ((*unk1).botX == 176 && (*unk1).botY == 71)
				(*unk1).topX = -2;
		if (strcmp(CurrentFEElement, "a1b.GImageBox") == 0)
			if ((*unk1).botX == 76 && (*unk1).botY == 71)
				(*unk1).topX = -78;
		if (strcmp(CurrentFEElement, "b1b.GImageBox") == 0)
			if ((*unk1).botX == 76 && (*unk1).botY == 17)
				(*unk1).topX = -78;
		if (strcmp(CurrentFEElement, "b2b.GImageBox") == 0)
			if ((*unk1).botX == 76 && (*unk1).botY == 17)
				(*unk1).topX = -78;
		// EA Trax Chyron end

		// cuffometer
		if (strcmp(CurrentFEElement, "<CUFFOMETER>.WCuffometer") == 0)
		{
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
			if (bUseModernHUD)
			{
				(*unk1).topX += Cuffo_XOffset;
				(*unk1).topY = ((600 - (*unk1).botY) - 2) - Cuffo_YOffset;
			}
			else
				(*unk1).topX -= 14;
		}
		if (strcmp(CurrentFEElement, "<star0>.GStaticImage") == 0)
		{
			(*unk1).topX = 26;
			if (bUseModernHUD)
				(*unk1).topY = 16;
		}
		if (strcmp(CurrentFEElement, "<star1>.GStaticImage") == 0)
		{
			(*unk1).topX = 46;
			if (bUseModernHUD)
				(*unk1).topY = 16;
		}
		if (strcmp(CurrentFEElement, "<star2>.GStaticImage") == 0)
		{
			(*unk1).topX = 66;
			if (bUseModernHUD)
				(*unk1).topY = 16;
		}
		if (strcmp(CurrentFEElement, "<star3>.GStaticImage") == 0)
		{
			(*unk1).topX = 87;
			if (bUseModernHUD)
				(*unk1).topY = 16;
		}
		if (strcmp(CurrentFEElement, "<star4>.GStaticImage") == 0)
		{
			(*unk1).topX = 107;
			if (bUseModernHUD)
				(*unk1).topY = 16;
		}
		if (strcmp(CurrentFEElement, "Reset.FlashText") == 0)
		{
			(*unk1).topX = -24;
			if (bUseModernHUD)
				(*unk1).topY = -20;
		}
		if (strcmp(CurrentFEElement, "<bar>.ProgressBar") == 0)
		{
			(*unk1).topX = 5;
			if (bUseModernHUD)
				(*unk1).topY = 2;
		}
		if (strcmp(CurrentFEElement, "<bar2>.ProgressBar") == 0)
		{
			(*unk1).topX = 5;
			if (bUseModernHUD)
				(*unk1).topY = 2;
		}
		if (strcmp(CurrentFEElement, "i1.GImageBox") == 0)
			if ((*unk1).botX == 75 && (*unk1).botY == 14)
			{
				(*unk1).topX = 47;
				if (bUseModernHUD)
					(*unk1).topY = 20;
			}
		// cuffometer end

		// ping display start
		if (strcmp(CurrentFEElement, "<PINGDISPLAY>.GWidget") == 0)
		{
			(*unk1).topX = 130;
			(*unk1).topY = 565;
		}
		// ping display end

		// letterbox scaling
		if ((strcmp(CurrentFEElement, "boxT.GImageBox") == 0) || (strcmp(CurrentFEElement, "tttt.GImageBox") == 0 || (strcmp(CurrentFEElement, "T.GImageBox") == 0)))
		{
			if (bUseModernHUD || bDisableLetterboxing)
			{
				(*unk1).botX = 0;
				(*unk1).botY = 0;
			}
			else
				(*unk1).botY = (600 - (resX_600height / LETTERBOX_ASPECT)) / 2;
		}
		
		if ((strcmp(CurrentFEElement, "boxB.GImageBox") == 0) || (strcmp(CurrentFEElement, "bbbb.GImageBox") == 0 || (strcmp(CurrentFEElement, "B.GImageBox") == 0)))
		{
			if (bUseModernHUD || bDisableLetterboxing)
			{
				(*unk1).botX = 0;
				(*unk1).botY = 0;
			}
			else
			{
				(*unk1).botY = (600 - (resX_600height / LETTERBOX_ASPECT)) / 2;
				(*unk1).topY += 110 - (*unk1).botY;
			}
		}

		// title screen & load screen
		// logo
		if ((strcmp(CurrentFEElement, "LOGO.GStaticImage") == 0) || (strcmp(CurrentFEElement, "logo.GStaticImage") == 0))
		{
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
			// alternative titlescreen graphics start
			if (!bAlternativeBackground)
			{
				(*unk1).botX = 0;
				(*unk1).botY = 0;
			}
			// alternative titlescreen graphics end
		}
		if (strcmp(CurrentFEShape, "[PC:S_INTRO.LYR.PCD] nfsl") == 0)
		{
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
			// alternative titlescreen graphics start
			if (bAlternativeBackground)
			{
				(*unk1).botX = 0;
				(*unk1).botY = 0;
			}
			// alternative titlescreen graphics end
		}
		// alternative titlescreen graphics start
		if (bAlternativeBackground)
			if (strcmp(CurrentFEElement, "title.GText") == 0)
				if ((*unk1).botX == 225 && (*unk1).botY == 42 && (*unk1).topY == 116)
					(*unk1).topY = 60;
		// alternative titlescreen graphics end
		if (strcmp(CurrentFEElement, "EATip.GText") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "loadbar.GStaticImage") == 0)
			(*unk1).topX = (resX_600height / 2) - 70;
		if (strcmp(CurrentFEElement, "<BAR>.GImageBox") == 0)
			(*unk1).topX = (resX_600height / 2) - 64;
		// title screen end

		// titlebar fix
		if (strcmp(CurrentFEElement, "ul.GImageBox") == 0)
			(*unk1).botX = resX_600height / 2;
		if (strcmp(CurrentFEElement, "bl.GImageBox") == 0)
			(*unk1).botX = resX_600height / 2;
		if (strcmp(CurrentFEElement, "cl.GImageBox") == 0)
			(*unk1).botX = resX_600height / 2;
		if (strcmp(CurrentFEElement, "ur.GImageBox") == 0)
			(*unk1).botX = resX_600height / 2;
		if (strcmp(CurrentFEElement, "br.GImageBox") == 0)
			(*unk1).botX = resX_600height / 2;
		if (strcmp(CurrentFEElement, "cr.GImageBox") == 0)
			(*unk1).botX = resX_600height / 2;
		// titlebar end

		// console fix
		if (strcmp(CurrentFEElement, "!console.GStaticImage") == 0)
			(*unk1).botX = resX_600height * 0.77875;
		// console end

		// button edges
		if (strcmp(CurrentFEElement, "L0.GImageBox") == 0)
			(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "R0.GImageBox") == 0)
			(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "L1.GImageBox") == 0)
			(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "R1.GImageBox") == 0)
			(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "!H0left.GImageBox") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 9;
		if (strcmp(CurrentFEElement, "!H1left.GImageBox") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 9;
		// button edges end

		// Dialogbox start
		if (strcmp(CurrentFEElement, "botart.GStaticImage") == 0)
			(*unk1).topX = -4;
		// Dialogbox end

		// Pause menu start
		if (strcmp(CurrentFEElement, "Accept.CmnPauseNav") == 0)
			(*unk1).topX /= xscale_800;
		// hud menu
		if (strcmp(CurrentFEElement, "<HUD>.ToggleButton") == 0)
			if ((*unk1).botX == 233 && (*unk1).botY == 20 && (*unk1).topY == 60)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "<SPEEDO>.ToggleButton") == 0)
			if ((*unk1).botX == 233 && (*unk1).botY == 20 && (*unk1).topY == 165)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "<MAP>.ToggleButton") == 0)
			if ((*unk1).botX == 233 && (*unk1).botY == 20 && (*unk1).topY == 95)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "<REAR_VIEW>.ToggleButton") == 0)
			if ((*unk1).botX == 233 && (*unk1).botY == 20 && (*unk1).topY == 130)
				(*unk1).topX /= xscale_800;
		// gameopt menu
		if (strcmp(CurrentFEElement, "<JUMPCAM>.ToggleButton") == 0)
			if ((*unk1).botX == 233 && (*unk1).botY == 20 && (*unk1).topY == 60)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "<360CAM>.ToggleButton") == 0)
			if ((*unk1).botX == 233 && (*unk1).botY == 20 && (*unk1).topY == 95)
				(*unk1).topX /= xscale_800;
		// main menu stuff
		if (strcmp(CurrentFEElement, "i_back2.GSimpleImage") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "i_logo.GSimpleImage") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<HotPursuit>.CmnScreenNav") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<TopCop>.CmnScreenNav") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<Championship>.CmnScreenNav") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<SingleChallenge>.CmnScreenNav") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<Multiplayer>.CmnScreenNav") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<QuickRace>.GButton") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<Options>.CmnScreenNav") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "p5.FEParticle") == 0)
			if ((*unk1).botX == 150 && (*unk1).botY == 150 && (*unk1).topY == 5)
				(*unk1).topX = (resX_600height - (*unk1).botX) / 2.5793650793650793650793650793651;
		if (strcmp(CurrentFEElement, "p5a.FEParticle") == 0)
			if ((*unk1).botX == 150 && (*unk1).botY == 150 && (*unk1).topY == 5)
				(*unk1).topX = (resX_600height - (*unk1).botX) / 1.8258426966292134831460674157303;
		if (strcmp(CurrentFEElement, "p3.FEParticle") == 0)
			if ((*unk1).botX == 25 && (*unk1).botY == 25 && (*unk1).topY == 230)
				(*unk1).topX = (resX_600height - (*unk1).botX) / 3.3695652173913043478260869565217;
		if (strcmp(CurrentFEElement, "p3a.FEParticle") == 0)
			if ((*unk1).botX == 25 && (*unk1).botY == 25 && (*unk1).topY == 325)
				(*unk1).topX = (resX_600height - (*unk1).botX) / 1.4168190127970749542961608775137;
		if (strcmp(CurrentFEElement, "Back.GButton") == 0 || strcmp(CurrentFEElement, "back.GButton") == 0 || strcmp(CurrentFEElement, "<BACK>.GButton") == 0 || strcmp(CurrentFEElement, "!Back.GText") == 0 || strcmp(CurrentFEElement, "Back.CmnScreenNav") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "Next.GButton") == 0 || strcmp(CurrentFEElement, "next.GButton") == 0 || strcmp(CurrentFEElement, "<NEXT>.GButton") == 0 || strcmp(CurrentFEElement, "!Next.GText" ) == 0 || strcmp(CurrentFEElement, "Next.CmnScreenNav") == 0)
			(*unk1).topX = ((resX_600height - (*unk1).botX) / 2) * 1.47;
		// main menu stuff end

		// general options menu start
		// typespin control fix start
		if (strcmp(CurrentFEElement, "p1.FEParticle") == 0)
			if ((*unk1).botX == 557 && (*unk1).botY == 50 && (*unk1).topY == 14)
				(*unk1).botX *= xscale_800;
		// typespin control fix end
		if (strcmp(CurrentFEElement, "<VIEW>.HP2OptionsView") == 0)
		{
			(*unk1).botX *= xscale_800;
			OptionsMenuXsize = (*unk1).botX;
		}
		// general options menu end

		// challenge setup start
		if (strcmp(CurrentFEElement, "<VIEW>.HP2SingleChallengeView") == 0)
			(*unk1).botX *= xscale_800;
		// challenge setup end

		// tournament status start
		if (strcmp(CurrentFEElement, "<RACE>.GText") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<LIST>.GListBox") == 0)
			if ((*unk1).botY == 162 && (*unk1).topY == 305)
				(*unk1).topX = (resX_600height - 640) / 2;
		if (strcmp(CurrentFEElement, "tx02.GText") == 0)
			if ((*unk1).botX == 185 && (*unk1).botY == 20 && (*unk1).topY == 283)
				(*unk1).topX = (resX_600height / 2) - 288;
		if (strcmp(CurrentFEElement, "tx03.GText") == 0)
			if ((*unk1).botX == 106 && (*unk1).botY == 20 && (*unk1).topY == 283)
				(*unk1).topX = (resX_600height / 2) - 103;
		if (strcmp(CurrentFEElement, "tx04.GText") == 0)
			if ((*unk1).botX == 105 && (*unk1).botY == 20 && (*unk1).topY == 283)
				(*unk1).topX = (resX_600height / 2) + 4;
		if (strcmp(CurrentFEElement, "tx05.GText") == 0)
			if ((*unk1).botX == 106 && (*unk1).botY == 20 && (*unk1).topY == 283)
				(*unk1).topX = (resX_600height / 2) + 106;
		if (strcmp(CurrentFEElement, "tx06.GText") == 0)
			if ((*unk1).botX == 102 && (*unk1).botY == 20 && (*unk1).topY == 283)
				(*unk1).topX = (resX_600height / 2) + 217;
		if (strcmp(CurrentFEElement, "tx01b.GText") == 0)
			if ((*unk1).botX == 275 && (*unk1).botY == 20 && (*unk1).topY == 249)
			{
				(*unk1).topX = (resX_600height - 165) / 2;
				(*unk1).topY = 470; // (600 - 12) / 2 // this will have to get changed if 1:1 scale will be done...
			}
		if (strcmp(CurrentFEElement, "B1.GImageBox") == 0)
			if ((*unk1).botX == 34 && (*unk1).botY == 160 && (*unk1).topY == 0)
				(*unk1).topX = (resX_480height / 2) - 332;
		if (strcmp(CurrentFEElement, "B2.GImageBox") == 0)
			if ((*unk1).botX == 105 && (*unk1).botY == 160 && (*unk1).topY == 0)
				(*unk1).topX = (resX_480height / 2) - 100;
		if (strcmp(CurrentFEElement, "B3.GImageBox") == 0)
			if ((*unk1).botX == 105 && (*unk1).botY == 160 && (*unk1).topY == 0)
				(*unk1).topX = (resX_480height / 2) + 109;
		// tournament status end

		// tournament trax start
		if (strcmp(CurrentFEElement, "TournTrax.GWidget") == 0)
			if ((*unk1).botX == 590 && (*unk1).botY == 131 && (*unk1).topY == 360)
				(*unk1).topX = (resX_600height / 2) - 295;
		if (strcmp(CurrentFEElement, "H0.GText") == 0)
			if ((*unk1).botX == 36 && (*unk1).botY == 17 && (*unk1).topY == -17)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "H1.GText") == 0)
			if ((*unk1).botX == 211 && (*unk1).botY == 17 && (*unk1).topY == -17)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "H2.GText") == 0)
			if ((*unk1).botX == 219 && (*unk1).botY == 17 && (*unk1).topY == -17)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "H3.GText") == 0)
			if ((*unk1).botX == 113 && (*unk1).botY == 17 && (*unk1).topY == -17)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "C2.GImageBox") == 0)
			if ((*unk1).botX == 221 && (*unk1).botY == 130 && (*unk1).topY == 1)
				(*unk1).topX /= xscale_800;
		// tournament trax end

		// controller options start
		if (strcmp(CurrentFEElement, "B1.GImageBox") == 0)
			if ((*unk1).botX == 471 && (*unk1).botY == 20 && (*unk1).topY == 1)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "B2.GImageBox") == 0)
			if ((*unk1).botX == 471 && (*unk1).botY == 20 && (*unk1).topY == 41)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "B3.GImageBox") == 0)
			if ((*unk1).botX == 471 && (*unk1).botY == 20 && (*unk1).topY == 81)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "B4.GImageBox") == 0)
			if ((*unk1).botX == 471 && (*unk1).botY == 20 && (*unk1).topY == 121)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "C1.GImageBox") == 0)
			if ((*unk1).botX == 268 && (*unk1).botY == 160 && (*unk1).topY == 1)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "C2.GImageBox") == 0)
			if ((*unk1).botX == 226 && (*unk1).botY == 160 && (*unk1).topY == 1)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "A1.GImageBox") == 0)
			if ((*unk1).botX == 471 && (*unk1).botY == 160 && (*unk1).topY == 1)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "D1.GImageBox") == 0)
			if ((*unk1).botX == 471 && (*unk1).botY == 20 && (*unk1).topY == -19)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "H1.GText") == 0)
			if ((*unk1).botX == 264 && (*unk1).botY == 12 && (*unk1).topY == -16)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "H2.GText") == 0)
			if ((*unk1).botX == 208 && (*unk1).botY == 12 && (*unk1).topY == -16)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "!h0L.GImageBox") == 0)
			if ((*unk1).botX == 240 && (*unk1).botY == 2 && (*unk1).topY == -20)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "!h0R.GImageBox") == 0)
			if ((*unk1).botX == 240 && (*unk1).botY == 2 && (*unk1).topY == -20)
				(*unk1).topX /= xscale_800;
		// ffb & deadzone start
		if (strcmp(CurrentFEElement, "STICK.CmnSlider") == 0)
		{
			(*unk1).topX /= xscale_800;
			(*unk1).topX -= (*unk1).botX - ((*unk1).botX / xscale_800);
		}
		if (strcmp(CurrentFEElement, "ROADFX.CmnSlider") == 0)
		{
			(*unk1).topX /= xscale_800;
			(*unk1).topX -= (*unk1).botX - ((*unk1).botX / xscale_800);
		}
		if (strcmp(CurrentFEElement, "COLLISION.CmnSlider") == 0)
		{
			(*unk1).topX /= xscale_800;
			(*unk1).topX -= (*unk1).botX - ((*unk1).botX / xscale_800);
		}
		if (strcmp(CurrentFEElement, "ENGINE.CmnSlider") == 0)
		{
			(*unk1).topX /= xscale_800;
			(*unk1).topX -= (*unk1).botX - ((*unk1).botX / xscale_800);
		}
		if (strcmp(CurrentFEElement, "GRIP.CmnSlider") == 0)
		{
			(*unk1).topX /= xscale_800;
			(*unk1).topX -= (*unk1).botX - ((*unk1).botX / xscale_800);
		}
		if (strcmp(CurrentFEElement, "<AXIS_0>.CmnSlider") == 0)
		{
			(*unk1).topX /= xscale_800;
			(*unk1).topX -= (*unk1).botX - ((*unk1).botX / xscale_800);
		}
		if (strcmp(CurrentFEElement, "<AXIS_1>.CmnSlider") == 0)
		{
			(*unk1).topX /= xscale_800;
			(*unk1).topX -= (*unk1).botX - ((*unk1).botX / xscale_800);
		}
		if (strcmp(CurrentFEElement, "<AXIS_2>.CmnSlider") == 0)
		{
			(*unk1).topX /= xscale_800;
			(*unk1).topX -= (*unk1).botX - ((*unk1).botX / xscale_800);
		}
		if (strcmp(CurrentFEElement, "<AXIS_3>.CmnSlider") == 0)
		{
			(*unk1).topX /= xscale_800;
			(*unk1).topX -= (*unk1).botX - ((*unk1).botX / xscale_800);
		}
		if (strcmp(CurrentFEElement, "<AXIS_4>.CmnSlider") == 0)
		{
			(*unk1).topX /= xscale_800;
			(*unk1).topX -= (*unk1).botX - ((*unk1).botX / xscale_800);
		}
		if (strcmp(CurrentFEElement, "<AXIS_5>.CmnSlider") == 0)
		{
			(*unk1).topX /= xscale_800;
			(*unk1).topX -= (*unk1).botX - ((*unk1).botX / xscale_800);
		}
		if (strcmp(CurrentFEElement, "<AXIS_6>.CmnSlider") == 0)
		{
			(*unk1).topX /= xscale_800;
			(*unk1).topX -= (*unk1).botX - ((*unk1).botX / xscale_800);
		}
		if (strcmp(CurrentFEElement, "<AXIS_7>.CmnSlider") == 0)
		{
			(*unk1).topX /= xscale_800;
			(*unk1).topX -= (*unk1).botX - ((*unk1).botX / xscale_800);
		}
		if (strcmp(CurrentFEElement, "<OK>.GButton") == 0)
			if ((*unk1).botX == 106 && (*unk1).botY == 18 && ((*unk1).topY == 210 || (*unk1).topY == 300))
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "<CANCEL>.GButton") == 0)
			if ((*unk1).botX == 101 && (*unk1).botY == 18 && ((*unk1).topY == 240 || (*unk1).topY == 330))
				(*unk1).topX /= xscale_800;
		// ffb & deadzone end
		if (strcmp(CurrentFEElement, "<CC>.GButton") == 0)
			if ((*unk1).botX == 200 && (*unk1).botY == 17 && (*unk1).topY == 320)
				(*unk1).topX = (OptionsMenuXsize / 2.1103896103896103896103896103896) - 100;
		if (strcmp(CurrentFEElement, "<RTD>.GButton") == 0)
			if ((*unk1).botX == 200 && (*unk1).botY == 17 && (*unk1).topY == 230)
				(*unk1).topX = (OptionsMenuXsize / 2.1103896103896103896103896103896) - 100;
		if (strcmp(CurrentFEElement, "<FFB>.GButton") == 0)
			if ((*unk1).botX == 200 && (*unk1).botY == 17 && (*unk1).topY == 260)
				(*unk1).topX = (OptionsMenuXsize / 2.1103896103896103896103896103896) - 100;
		if (strcmp(CurrentFEElement, "<DEADZONE>.GButton") == 0)
			if ((*unk1).botX == 200 && (*unk1).botY == 17 && (*unk1).topY == 290)
				(*unk1).topX = (OptionsMenuXsize / 2.1103896103896103896103896103896) - 100;
		if (strcmp(CurrentFEElement, "<CONTROLS>.GWidget") == 0)
			if ((*unk1).botX == 512 && (*unk1).botY == 164 && (*unk1).topY == 52)
				(*unk1).topX = (OptionsMenuXsize / 3.7790697674418604651162790697674) - 100;
		// controller options end

		// credits screen start
		if (strcmp(CurrentFEElement, "!H0left.GImageBox") == 0)
			(*unk1).botX *= xscale_800;
		if (strcmp(CurrentFEElement, "!H1left.GImageBox") == 0)
			(*unk1).botX *= xscale_800;
		// credits screen end

		// driver profile start
		if (OptionsMenuXsize)
		{
			if (strcmp(CurrentFEElement, "<LOAD>.GButton") == 0)
				if ((*unk1).botX == 222 && (*unk1).botY == 16 && (*unk1).topY == 210)
					(*unk1).topX = (OptionsMenuXsize / 2.2260273972602739726027397260274) - 100;
			if (strcmp(CurrentFEElement, "<DELETE>.GButton") == 0)
				if ((*unk1).botX == 222 && (*unk1).botY == 16 && (*unk1).topY == 240)
					(*unk1).topX = (OptionsMenuXsize / 2.2260273972602739726027397260274) - 100;
			if (strcmp(CurrentFEElement, "<NEW>.GButton") == 0)
				if ((*unk1).botX == 222 && (*unk1).botY == 16 && (*unk1).topY == 270)
					(*unk1).topX = (OptionsMenuXsize / 2.2260273972602739726027397260274) - 100;
			if (strcmp(CurrentFEElement, "<SAVE>.GButton") == 0)
				if ((*unk1).botX == 222 && (*unk1).botY == 16 && (*unk1).topY == 300)
					(*unk1).topX = (OptionsMenuXsize / 2.2260273972602739726027397260274) - 100;
			if (strcmp(CurrentFEElement, "<DPL>.ToggleButton") == 0)
				(*unk1).topX = (OptionsMenuXsize / 1.625) - 100;
			if (strcmp(CurrentFEElement, "<DSTATS>.GListBox") == 0)
				(*unk1).topX = (OptionsMenuXsize / 2.9017857142857142857142857142857) - 100;
			if (strcmp(CurrentFEElement, "<CURRP>.GText") == 0)
				(*unk1).topX = (OptionsMenuXsize / 2.9017857142857142857142857142857) - 100;
			// the create profile dialog thingie start
			if (strcmp(CurrentFEElement, "<EDIT>.HP2EditText") == 0)
				if ((*unk1).botX == 162 && (*unk1).botY == 12 && (*unk1).topY == 61)
					(*unk1).topX /= xscale_800;
			if (strcmp(CurrentFEElement, "editback00.GImageBar") == 0)
				if ((*unk1).botX == 180 && (*unk1).botY == 22 && (*unk1).topY == 55)
					(*unk1).topX /= xscale_800;
			if (strcmp(CurrentFEElement, "<B1>.GButton") == 0)
				if ((*unk1).botX == 222 && (*unk1).botY == 14 && (*unk1).topY == 110)
					(*unk1).topX /= xscale_800;
			if (strcmp(CurrentFEElement, "<B3>.GButton") == 0)
				if ((*unk1).botX == 222 && (*unk1).botY == 14 && (*unk1).topY == 140)
					(*unk1).topX /= xscale_800;
			// the create profile dialog thingie end
		}
		// driver profile end

		// multiplayer stuff start
		if (strcmp(CurrentFEElement, "!H0Left.GImageBox") == 0)
			if ((*unk1).botY == 2 && (*unk1).topY == 29)
				(*unk1).topX = (resX_600height - (*unk1).botX) / 2 - 130;
		if (strcmp(CurrentFEElement, "!H0left.GImageBox") == 0)
			if ((*unk1).botY == 2 && (*unk1).topY == 20)
				(*unk1).botX += (*unk1).topX + 10;
		if (strcmp(CurrentFEElement, "!H1left.GImageBox") == 0)
			if ((*unk1).botY == 2 && (*unk1).topY == 121)
				(*unk1).botX += (*unk1).topX + 10;
		if (strcmp(CurrentFEElement, "ttlbarL0.GImageBox") == 0)
			if ((*unk1).botX == 320 && (*unk1).botY == 26 && (*unk1).topY == 21)
				(*unk1).topX = (resX_600height - (*unk1).botX) / 2 - 159;
		if (strcmp(CurrentFEElement, "ttlbarL1.GImageBox") == 0)
			if ((*unk1).botX == 320 && (*unk1).botY == 26 && (*unk1).topY == 122)
				(*unk1).topX = (resX_600height - (*unk1).botX) / 2 - 159;
		// multiplayer stuff end

		// car select
		if (strcmp(CurrentFEElement, "<CAR>.CarWidget") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2 + 57;
		if (strcmp(CurrentFEElement, "<Showcase>.GButton") == 0 && ASPECT_DIFF_GREATHAN)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<CAR_IL>.HPImgList") == 0 && ASPECT_DIFF_GREATHAN)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<TRANY>.ToggleButton") == 0 && ASPECT_DIFF_GREATHAN)
			if ((*unk1).botX == 182 && (*unk1).botY == 21 && (*unk1).topY == 241)
				(*unk1).topX = (resX_600height - (*unk1).botX) / 2 + 78;
		if (strcmp(CurrentFEElement, "<btl>.RolloverButton") == 0)
			if ((*unk1).botX == 45 && (*unk1).botY == 73 && (*unk1).topY == 42)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "<btr>.RolloverButton") == 0)
			if ((*unk1).botX == 45 && (*unk1).botY == 73 && (*unk1).topY == 42)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "p6.FEParticle") == 0)
			if ((*unk1).botX == 128 && (*unk1).botY == 128 && (*unk1).topY == -6)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "p7.FEParticle") == 0)
			if ((*unk1).botX == 128 && (*unk1).botY == 128 && (*unk1).topY == -6)
				(*unk1).topX /= xscale_800;
		// car select end

		// track select
		if (strcmp(CurrentFEElement, "<Descrip>.GButton") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<tdir>.GText") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<lock>.GStaticImage") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<Cost>.GText") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<bup>.RolloverButton") == 0)
			if ((*unk1).botX == 36 && (*unk1).botY == 22 && (*unk1).topY == -3)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "<bdn>.RolloverButton") == 0)
			if ((*unk1).botX == 35 && (*unk1).botY == 22 && (*unk1).topY == 68)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "p1.FEParticle") == 0)
			if ((*unk1).botX == 25 && (*unk1).botY == 25 && (*unk1).topY == 33)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "p2.FEParticle") == 0)
			if ((*unk1).botX == 25 && (*unk1).botY == 25 && (*unk1).topY == 33)
				(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "<Dir>.GAnimImage") == 0)
			if ((*unk1).botX == 45 && (*unk1).botY == 45 && (*unk1).topY == 22)
				(*unk1).topX /= xscale_800;
		// track select end

		// event tree
		if (strcmp(CurrentFEElement, "<EVENT_ID>.GText") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<EVENT_OBJECTIVE>.GText") == 0)
		{
			(*unk1).botX *= xscale_800;
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		}
		if (strcmp(CurrentFEElement, "<EVENT_STATUS>.GText") == 0)
			(*unk1).topX = (resX_600height / 2) + 25;
		if (strcmp(CurrentFEElement, "ET1a.GText") == 0)
			(*unk1).topX = (resX_600height / 2) - 130;
		if (strcmp(CurrentFEElement, "ET7.GText") == 0)
			(*unk1).topX = (resX_600height / 2) - 273;
		if (strcmp(CurrentFEElement, "<EVENT_CAR>.GText") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<EVENT_OPPONENTS>.GText") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<EVENT_LAPS>.GText") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<EVENT_TRACKS>.GText") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<REWARD>.GText") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<OK>.GButton") == 0)
			if ((*unk1).botX == 192 && (*unk1).botY == 17 && (*unk1).topY == 350)
				(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<CANCEL>.GButton") == 0)
			if ((*unk1).botX == 190 && (*unk1).botY == 17 && (*unk1).topY == 380)
				(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		// event tree end

		// starting/ending grid
		if (strcmp(CurrentFEElement, "t0.GText") == 0)
			if ((*unk1).botX == 195 && (*unk1).botY == 45 && (*unk1).topY == 65)
			{
				(*unk1).topX = 8;
				(*unk1).topY = 8;
			}
		if (strcmp(CurrentFEElement, "t0.FlashText") == 0)
			if ((*unk1).botX == 191 && (*unk1).botY == 35 && (*unk1).topY == 40)
			{
				(*unk1).topX = 8;
				(*unk1).topY = 8;
			}
		if (strcmp(CurrentFEElement, "GRID.GWidget") == 0)
			if ((*unk1).botX == 422 && (*unk1).botY == 160 && (*unk1).topY == 247)
				(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<StartRace>.GButton") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "field.GWidget") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<LIST>.GListBox") == 0)
			if ((*unk1).botX == 422 && (*unk1).botY == 160)
				(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<RESULTS>.GListBox") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "<TRACK>.GText") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "titlebar.GImageBox") == 0)
			if ((*unk1).topX)
			{
				if ((*unk1).botX == 463 && (*unk1).botY == 40)
					(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
				if ((*unk1).botX == 349 && (*unk1).botY == 30)
					(*unk1).topX = (resX_600height - (*unk1).botX) / 2 - 57;
				if ((*unk1).botX == 464 && (*unk1).botY == 20)
					(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
			}
		if (strcmp(CurrentFEElement, "B1.GButton") == 0)
			if ((*unk1).botX == 222 && (*unk1).botY == 18 && (*unk1).topY == 250)
				(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "B2.GButton") == 0)
			if ((*unk1).botX == 222 && (*unk1).botY == 18 && (*unk1).topY == 290)
				(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "B3.GButton") == 0)
			if ((*unk1).botX == 222 && (*unk1).botY == 18 && (*unk1).topY == 330)
				(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "title.GText") == 0)
			if ((*unk1).botX == 464 && (*unk1).botY == 30)
				(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		if (strcmp(CurrentFEElement, "h2.GText") == 0)
		{
			if ((*unk1).botX == 176 && (*unk1).botY == 20)
				(*unk1).topX = ((resX_600height - (*unk1).botX) / 2) - 98;
			if ((*unk1).botX == 164 && (*unk1).botY == 18)
				(*unk1).topX = ((resX_600height - (*unk1).botX) / 2) - 119;
		}
		if (strcmp(CurrentFEElement, "h3.GText") == 0)
		{
			if ((*unk1).botX == 221 && (*unk1).botY == 20)
				(*unk1).topX = ((resX_600height - (*unk1).botX) / 2) + 101;
			if ((*unk1).botX == 166 && (*unk1).botY == 18)
				(*unk1).topX = ((resX_600height - (*unk1).botX) / 2) + 47;
		}
		if (strcmp(CurrentFEElement, "h4.GText") == 0)
			if ((*unk1).botX == 110 && (*unk1).botY == 18)
				(*unk1).topX = ((resX_600height - (*unk1).botX) / 2) + 166;
		if (strcmp(CurrentFEElement, "<COLUMN0>.GText") == 0)
			(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "<COLUMN1>.GText") == 0)
			(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "<COLUMN2>.GText") == 0)
			(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "<COLUMN3>.GText") == 0)
			(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "<COLUMN4>.GText") == 0)
			(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "<COLUMN5>.GText") == 0)
			(*unk1).topX /= xscale_800;
		if (strcmp(CurrentFEElement, "c2.GImageBox") == 0)
		{
			if ((*unk1).botX == 223 && (*unk1).botY == 160)
				(*unk1).topX = 200;
			if ((*unk1).botX == 167 && (*unk1).botY == 160)
				(*unk1).topX = 188;
		}
		if (strcmp(CurrentFEElement, "<TEXT>.GText") == 0)
			if ((*unk1).botX == 464 && (*unk1).botY == 63 && (*unk1).topY == 253)
				(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		// starting/ending grid end

		// race end stuff start
		if (strcmp(CurrentFEElement, "02Points.GWidget") == 0)
			(*unk1).topX = (resX_600height - (*unk1).botX) / 2;
		// race end stuff end

		// misc. stuff
		if (strcmp(CurrentFEElement, "BNK1.GText") == 0)
			(*unk1).topX = ((resX_600height - (*unk1).botX) / 2) * 1.63;
		if (strcmp(CurrentFEElement, "tm.GText") == 0)
			(*unk1).topX = ((resX_600height) / 2) + 196;
		// misc. stuff end

		CurrentFEShape[0] = 0;
	}
	return retval;
}

int __stdcall WriteRenderCaps_Hook()
{
	unsigned int thethis = 0;
	_asm mov thethis, ecx
	unsigned int returnval = WriteRenderCaps(thethis);
	InitRenderCaps();
	InjectRes();

	return returnval;
}

int __stdcall ReadIni_Float_Hook(char *section, char *key, float* value)
{
	unsigned int thethis = 0;
	_asm mov thethis, ecx
	int returnval = ReadIni_Float(thethis, section, key, value);
	float NewFOV = RecalculateFOV_4by3(*value);
	printf("FOVHAX: [%s] %s | READ: %.2f | WRITE: %.2f\n", section, key, *value, NewFOV);
	*value = NewFOV;

	return returnval;
}

char* __stdcall GetUserDir_Hook()
{
	return UserDir;
}

int InitRenderCaps()
{
	bool bPathWasAbsolute = false;
	if (CheckForPathAbsolution(UserDir))
	{
		sprintf(RenderCapsIni, "%srendercaps.ini", UserDir);
		bPathWasAbsolute = true;
	}
	else // relative to the game exe
		sprintf(RenderCapsIni, "..\\%srendercaps.ini", UserDir); // due to inireader's path defaulting to the scripts folder

	// read the resolution from rendercaps.ini file directly
	CIniReader renderCaps(RenderCapsIni);
	// workaround due to silly inireader stuff...
	if (bPathWasAbsolute)
		strcpy(renderCaps.m_szFileName, RenderCapsIni);

	resX = renderCaps.ReadInteger("Graphics", "Width", 0);
	resY = renderCaps.ReadInteger("Graphics", "Height", 0);

	if (!(resX || resY))
		GetDesktopRes(&resX, &resY);

	// TEMPORARY HUD FIX
	if (bAspectRatioFix)
	{
		resX_43f = ((float)resY * (4.0f / 3.0f));

		FE_horscale = resX_43f / 800.0;
		FE_horposition = (resX - resX_43f) / 2;

		*(float*)FE_XSCALE_ADDRESS = FE_horscale;
		if (bAspectRatioFix == 1)
			*(float*)FE_XPOS_ADDRESS = FE_horposition;

		// fix function scaling...
		*(float*)FE_FUNCTIONS_YSCALE_ADDRESS = 1 / (resY / 600.0);
		*(float*)FE_FUNCTIONS_XSCALE_ADDRESS = *(float*)FE_FUNCTIONS_YSCALE_ADDRESS;
	}

	// equalize the GraphicsFE with Graphics & kill res limiter
	renderCaps.WriteInteger("Graphics", "LimitResolution", 0);
	renderCaps.WriteInteger("GraphicsFE", "Width", resX);
	renderCaps.WriteInteger("GraphicsFE", "Height", resY);
	renderCaps.WriteInteger("GraphicsFE", "ScreenModeIndex", renderCaps.ReadInteger("Graphics", "ScreenModeIndex", 0));
	renderCaps.WriteInteger("GraphicsFE", "BDepth", renderCaps.ReadInteger("Graphics", "BDepth", 32));
	renderCaps.WriteInteger("GraphicsFE", "ZDepth", renderCaps.ReadInteger("Graphics", "ZDepth", 24));
	renderCaps.WriteInteger("GraphicsFE", "Stencil", renderCaps.ReadInteger("Graphics", "Stencil", 8));

	aspect = (float)resX / (float)resY;
	aspect_diff = (float)FOUR_BY_THREE_ASPECT / aspect;

	resX_600height = aspect * 600;
	resX_480height = aspect * 480;
	xscale_800 = resX_600height / 800;
	xscale_640 = resX_480height / 640;

	return 0;
}

int InitConfig()
{
	CIniReader iniReader("");
	struct stat st = { 0 };
	const char* InputDirString = iniReader.ReadString("MAIN", "CustomUserFilesDirectoryInGameDir", UserDir);
	bEnableConsole = iniReader.ReadInteger("MAIN", "EnableConsole", 0);
	bAspectRatioFix = iniReader.ReadInteger("MAIN", "AspectRatioFix", 1);
	bAlternativeBackground = iniReader.ReadInteger("MAIN", "AlternativeBackground", 0);

	if (bAspectRatioFix)
	{
		bDisableLetterboxing = iniReader.ReadInteger("MAIN", "DisableLetterboxing", 0);
		bUseModernHUD = iniReader.ReadInteger("MAIN", "UseModernHUD", 0);

		if (bUseModernHUD)
		{
			Dash_XOffset = iniReader.ReadInteger("HUDoffsets", "Dash_X", 0);
			Tach_XOffset = iniReader.ReadInteger("HUDoffsets", "Tach_X", 0);
			Map_XOffset = iniReader.ReadInteger("HUDoffsets", "Map_X", 0);
			Score_XOffset = iniReader.ReadInteger("HUDoffsets", "Score_X", 0);
			Time_XOffset = iniReader.ReadInteger("HUDoffsets", "Time_X", 0);
			Pos_XOffset = iniReader.ReadInteger("HUDoffsets", "Pos_X", 0);
			CopAmmo_XOffset = iniReader.ReadInteger("HUDoffsets", "CopAmmo_X", 0);
			RVM_XOffset = iniReader.ReadInteger("HUDoffsets", "RVM_X", 0);
			Cuffo_XOffset = iniReader.ReadInteger("HUDoffsets", "Cuffo_X", 0);

			Dash_YOffset = iniReader.ReadInteger("HUDoffsets", "Dash_Y", 0);
			Tach_YOffset = iniReader.ReadInteger("HUDoffsets", "Tach_Y", 0);
			Map_YOffset = iniReader.ReadInteger("HUDoffsets", "Map_Y", 0);
			Score_YOffset = iniReader.ReadInteger("HUDoffsets", "Score_Y", 0);
			Time_YOffset = iniReader.ReadInteger("HUDoffsets", "Time_Y", 0);
			Pos_YOffset = iniReader.ReadInteger("HUDoffsets", "Pos_Y", 0);
			CopAmmo_YOffset = iniReader.ReadInteger("HUDoffsets", "CopAmmo_Y", 0);
			RVM_YOffset = iniReader.ReadInteger("HUDoffsets", "RVM_Y", 0);
			Cuffo_YOffset = iniReader.ReadInteger("HUDoffsets", "Cuffo_Y", 0);
		}
	}

	RenderMemorySize = iniReader.ReadInteger("MEMORY", "CLASS_RENDER", 0x732000);
	GeneralMemorySize = iniReader.ReadInteger("MEMORY", "GENERAL", 0x5FB9000);
	AudioMemorySize = iniReader.ReadInteger("MEMORY", "CLASS_AUDIO", 0xA00000);
	TrackMemorySize = iniReader.ReadInteger("MEMORY", "CLASS_TRACK", 0x2B00000);
	LevelMemorySize = iniReader.ReadInteger("MEMORY", "CLASS_LEVEL", 0x196000);
	UIMemorySize = iniReader.ReadInteger("MEMORY", "CLASS_UI", 0x400000);
	CarsMemorySize = iniReader.ReadInteger("MEMORY", "CLASS_CARS", 0x700000);
	CharacterMemorySize = iniReader.ReadInteger("MEMORY", "CLASS_CHARACTER", 0x17D000);
	ReplayMemorySize = iniReader.ReadInteger("MEMORY", "CLASS_REPLAY", 0x64000);
	IniFileMemorySize = iniReader.ReadInteger("MEMORY", "CLASS_INIFILE", 0x10000);

	if (InputDirString && (InputDirString[0] != '0'))
	{
		strncpy(UserDir, InputDirString, 255);

		if (CheckForPathAbsolution(InputDirString))
			sprintf(UserDir, "%s", InputDirString);
		else
			sprintf(UserDir, "..\\%s", InputDirString);
	
		if (stat(UserDir, &st) == -1)
		{
			// NOT crossplatform compatible - made exclusively due to simplicity's sake
			sprintf(MkDirStr, "mkdir \"%s\"", UserDir);
			system(MkDirStr);
		}
		sprintf(UserDir, "%s\\", InputDirString);
	}
	else
	{
		// this is what the game does anyways, and since hooking into this piece of code causes crashes, I've rewritten it here
		SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, UserDir);
		if (*(int*)LANGUAGE_ADDRESS == 1)
			strcat(UserDir, "\\EA Games\\Need For Speed poursuite infernale 2");
		else
			strcat(UserDir, "\\EA Games\\Need For Speed Hot Pursuit 2");

		if (stat(UserDir, &st) == -1)
		{
			// NOT crossplatform compatible - made exclusively due to simplicity's sake
			sprintf(MkDirStr, "mkdir \"%s\"", UserDir);
			system(MkDirStr);
		}
		strcat(UserDir, "\\");
	}

	InitRenderCaps();
	return 0;
}

void InjectRes()
{
	*(int*)LIMITRES_ADDRESS = 0;
	injector::MakeJMP(LIMITRES_ADDRESS_JMP1, LIMITRES_ADDRESS_JMP2, true);

	injector::WriteMemory<float>(ASPECT_ADDRESS_1, aspect, true);
	injector::WriteMemory<float>(ASPECT_ADDRESS_2, aspect, true);
	injector::WriteMemory<float>(ASPECT_ADDRESS_3, aspect, true);
	// gui
	injector::WriteMemory<float>(ASPECT_ADDRESS_4, aspect, true);

#ifdef HP2DEBUG // extra values
	injector::WriteMemory<float>(ASPECT_ADDRESS_5, aspect, true);
	injector::WriteMemory<float>(ASPECT_ADDRESS_6, aspect, true);
#endif

	// mov
	injector::WriteMemory<int>(RESX_ADDRESS_01, resX, true);
	injector::WriteMemory<int>(RESY_ADDRESS_01, resY, true);

	// push
	injector::WriteMemory<int>(RESX_ADDRESS_02, resX, true);
	injector::WriteMemory<int>(RESY_ADDRESS_02, resY, true);

	// push
	injector::WriteMemory<int>(RESX_ADDRESS_03, resX, true);
	injector::WriteMemory<int>(RESY_ADDRESS_03, resY, true);

	// push
	injector::WriteMemory<int>(RESX_ADDRESS_04, resX, true);
	injector::WriteMemory<int>(RESY_ADDRESS_04, resY, true);

	// cmp
	injector::WriteMemory<int>(RESX_ADDRESS_05, resX, true);
	injector::WriteMemory<int>(RESY_ADDRESS_05, resY, true);

	// mov
	injector::WriteMemory<int>(RESX_ADDRESS_06, resX, true);
	injector::WriteMemory<int>(RESY_ADDRESS_06, resY, true);

	// cmp
	injector::WriteMemory<int>(RESX_ADDRESS_07, resX, true);
	injector::WriteMemory<int>(RESY_ADDRESS_07, resY, true);
	injector::WriteMemory<float>(RESX_ADDRESS_08, (float)resX, true);
	injector::WriteMemory<float>(RESY_ADDRESS_08, (float)resY, true);
	injector::WriteMemory<float>(RESX_ADDRESS_09, (float)resX, true);
	injector::WriteMemory<float>(RESY_ADDRESS_09, (float)resY, true);
	injector::WriteMemory<float>(RESX_ADDRESS_10, (float)resX, true);
	injector::WriteMemory<float>(RESY_ADDRESS_10, (float)resY, true);

	// METHOD 2 - FIX Y FE SCALING BY CHANGING SCALING FACTOR
	injector::WriteMemory<float>(FE_3DYSCALE_ADDRESS_1, aspect_diff, true); // FE 3D map actor Y scale
	injector::WriteMemory<float>(FE_3DYSCALE_ADDRESS_2, aspect_diff, true); // FE car actor Y scale
	injector::WriteMemory<float>(FE_3DYSCALE_ADDRESS_3, aspect_diff, true); // FE 3D event tree actor Y scale

	// GUI hax
	if (bAspectRatioFix)
	{
		injector::WriteMemory<float>(RESX_ADDRESS_11, (float)resX, true);
		injector::WriteMemory<float>(RESY_ADDRESS_11, (float)resY, true);
	}
}

int InitInjector()
{
	InjectRes();

	// dirty FOV ini read hooks (WILL GET REPLACED BY PROPER SCALING HACKS LATER)
	injector::MakeCALL(READINI_FLOAT_ADDRESS_01, ReadIni_Float_Hook, true);
	injector::MakeCALL(READINI_FLOAT_ADDRESS_02, ReadIni_Float_Hook, true);
	injector::MakeCALL(READINI_FLOAT_ADDRESS_03, ReadIni_Float_Hook, true);
	injector::MakeCALL(READINI_FLOAT_ADDRESS_04, ReadIni_Float_Hook, true);
	injector::MakeCALL(READINI_FLOAT_ADDRESS_05, ReadIni_Float_Hook, true);
	injector::MakeCALL(READINI_FLOAT_ADDRESS_06, ReadIni_Float_Hook, true);
	injector::MakeCALL(READINI_FLOAT_ADDRESS_07, ReadIni_Float_Hook, true);
	injector::MakeCALL(READINI_FLOAT_ADDRESS_08, ReadIni_Float_Hook, true);
	injector::MakeCALL(READINI_FLOAT_ADDRESS_09, ReadIni_Float_Hook, true);
	injector::MakeCALL(READINI_FLOAT_ADDRESS_10, ReadIni_Float_Hook, true);
	injector::MakeCALL(READINI_FLOAT_ADDRESS_11, ReadIni_Float_Hook, true);
	injector::MakeCALL(READINI_FLOAT_ADDRESS_12, ReadIni_Float_Hook, true);
	injector::MakeCALL(READINI_FLOAT_ADDRESS_13, ReadIni_Float_Hook, true);
	injector::MakeCALL(READINI_FLOAT_ADDRESS_14, ReadIni_Float_Hook, true);
	injector::MakeCALL(READINI_FLOAT_ADDRESS_15, ReadIni_Float_Hook, true);
	injector::MakeCALL(READINI_FLOAT_ADDRESS_16, ReadIni_Float_Hook, true);

	// User directory hax
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_01, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_02, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_03, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_04, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_05, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_06, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_07, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_08, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_09, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_10, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_11, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_12, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_13, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_14, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_15, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_16, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_17, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_18, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_19, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_20, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_21, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_22, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_23, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_24, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_25, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_26, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_27, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_28, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_29, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_30, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_31, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_32, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_33, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_34, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_35, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_36, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_37, GetUserDir_Hook, true);
	injector::MakeCALL(GETUSERDIR_HOOK_ADDR_38, GetUserDir_Hook, true);

	// WriteRenderCaps
	injector::MakeCALL(WRITERENDERCAPS_ADDRESS_1, WriteRenderCaps_Hook, true);
	injector::MakeCALL(WRITERENDERCAPS_ADDRESS_2, WriteRenderCaps_Hook, true);
	injector::MakeCALL(WRITERENDERCAPS_ADDRESS_3, WriteRenderCaps_Hook, true);

	// "Hi-Poly" fixes
	// an actual attempt at a hi-poly fix
	injector::WriteMemory<int>(RENDERMEMORYSIZE_ADDR_1, RenderMemorySize, true);
	injector::WriteMemory<int>(RENDERMEMORYSIZE_ADDR_2, RenderMemorySize, true); // korean lang.
	injector::WriteMemory<int>(RENDERMEMORYSIZE_ADDR_3, RenderMemorySize, true); // for printout only
	// other memory adjusters
	injector::WriteMemory<int>(GENERALMEMORYSIZE_ADDR_1, GeneralMemorySize, true);
	injector::WriteMemory<int>(GENERALMEMORYSIZE_ADDR_2, GeneralMemorySize, true); // korean lang.
	injector::WriteMemory<int>(AUDIOMEMORYSIZE_ADDR_1, AudioMemorySize, true);
	injector::WriteMemory<int>(AUDIOMEMORYSIZE_ADDR_2, AudioMemorySize, true); // for printout only
	injector::WriteMemory<int>(TRACKMEMORYSIZE_ADDR_1, TrackMemorySize, true);
	injector::WriteMemory<int>(TRACKMEMORYSIZE_ADDR_2, TrackMemorySize, true); // for printout only
	injector::WriteMemory<int>(LEVELMEMORYSIZE_ADDR_1, LevelMemorySize, true);
	injector::WriteMemory<int>(LEVELMEMORYSIZE_ADDR_2, LevelMemorySize, true); // for printout only
	injector::WriteMemory<int>(UIMEMORYSIZE_ADDR_1, UIMemorySize, true);
	injector::WriteMemory<int>(UIMEMORYSIZE_ADDR_2, UIMemorySize, true); // korean lang.
	injector::WriteMemory<int>(UIMEMORYSIZE_ADDR_3, UIMemorySize, true); // for printout only
	injector::WriteMemory<int>(CARSMEMORYSIZE_ADDR_1, CarsMemorySize, true);
	injector::WriteMemory<int>(CARSMEMORYSIZE_ADDR_2, CarsMemorySize, true); // for printout only
	injector::WriteMemory<int>(CHARACTERMEMORYSIZE_ADDR_1, CharacterMemorySize, true);
	injector::WriteMemory<int>(CHARACTERMEMORYSIZE_ADDR_2, CharacterMemorySize, true); // for printout only
	injector::WriteMemory<int>(REPLAYMEMORYSIZE_ADDR_1, ReplayMemorySize, true);
	injector::WriteMemory<int>(REPLAYMEMORYSIZE_ADDR_2, ReplayMemorySize, true); // for printout only
	injector::WriteMemory<int>(INIFILEMEMORYSIZE_ADDR_1, IniFileMemorySize, true);
	injector::WriteMemory<int>(INIFILEMEMORYSIZE_ADDR_2, IniFileMemorySize, true); // for printout only

	// GUI sub_595440
	if (bAspectRatioFix)
	{
		injector::MakeCALL(HOOK_ADDRESS_1, sub_59B840_hook, true);
		if (bAspectRatioFix > 1)
		{
			injector::MakeCALL(HOOK_ADDRESS_2, sub_59B840_hook_2, true);
			injector::MakeCALL(HOOK_ADDRESS_3, sub_59BAE0_hook, true);
			injector::MakeCALL(HOOK_ADDRESS_4, sub_59BAE0_hook, true);

			// cuffometer position fix - disable writes to its mBounds
			// X
			injector::MakeNOP(CUFFOMETER_FIX_ADDR_1, 3, true);
			injector::MakeNOP(CUFFOMETER_FIX_ADDR_2, 3, true);
			if (bUseModernHUD)
			{
				// Y
				injector::MakeNOP(CUFFOMETER_FIX_ADDR_3, 3, true);
				injector::MakeNOP(CUFFOMETER_FIX_ADDR_4, 3, true);
			}
		}
		injector::MakeCALL(HOOK_ADDRESS_5, sub_5954A0, true);
		injector::MakeCALL(HOOK_ADDRESS_6, sub_5954A0, true);

		// X scale
		injector::MakeNOP(FE_XSCALENOP_ADDR, 6, true);

		// Y functions scale
		injector::MakeNOP(FE_YFUNCSCALENOP_ADDR, 6, true);

		// X functions scale
		injector::MakeNOP(FE_XFUNCSCALENOP_ADDR, 6, true);
		injector::MakeCALL(FE_CURSORPOS_HOOK_ADDR, FE_CursorPos, true);

		// X pos
		injector::MakeNOP(FE_XPOSNOP_ADDR, 6, true);
	}

#ifndef HP2DEBUG
	if (bEnableConsole)
		injector::MakeJMP(PRINTF_HOOK_ADDR, printf, true);
#endif
	// Linux fix
	injector::MakeNOP(0x00537FD3, 5, true);

	return 0;
}

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD reason, LPVOID /*lpReserved*/)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		InitConfig();
		if (bEnableConsole)
		{
			AttachConsole(ATTACH_PARENT_PROCESS);
			AllocConsole();
			freopen("CON", "r", stdin);
			freopen("CON", "w", stdout);
			freopen("CON", "w", stderr);
		}
		InitInjector();
	}
	return TRUE;
}
