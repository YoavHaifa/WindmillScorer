#include "pch.h"
#include "Config.h"
#include "..\..\yUtils\MyWindows.h"

CConfig gConfig;

CConfig::CConfig()
	: mDebug(127)
	, mDump(127)
	, miCurrentImage(-1) // Not yet defiend
{
	msDataRoot = "e:\\Windmill\\DATA for Scorer\\SCANPLAN_314_FOV450";
	msHintsDir = msDataRoot + "\\WMAS_Hints";
	if (CMyWindows::IsDirectory(msDataRoot))
		CMyWindows::VerifyDirectory(msHintsDir);
}
void CConfig::SetPosition(int iImage)
{
	miCurrentImage = iImage;
}
