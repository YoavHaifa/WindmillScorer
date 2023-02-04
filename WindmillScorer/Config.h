#pragma once

class CConfig
{
public:
	CConfig();
	CString msDataRoot;

	int mDebug;
	int mDump;
};

extern CConfig gConfig;

