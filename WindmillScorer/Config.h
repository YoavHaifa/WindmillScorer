#pragma once

class CConfig
{
public:
	CConfig();
	void SetPosition(int iImage);

	CString msDataRoot;
	CString msHintsDir;
	CString msMasksDir;

	int miCurrentImage;
	int mDebug;
	int mDump;
};

extern CConfig gConfig;

