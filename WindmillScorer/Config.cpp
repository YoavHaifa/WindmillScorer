#include "pch.h"
#include "Config.h"

CConfig gConfig;

CConfig::CConfig()
	: mDebug(127)
	, mDump(127)
{
	msDataRoot = "e:\\Windmill\\DATA for Scorer\\SCANPLAN_314_FOV450";
}