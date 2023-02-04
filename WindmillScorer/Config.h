#pragma once

class CConfig
{
public:
	CConfig();
	void SetPosition(int iImage);

	CString msDataRoot;

	int miCurrentImage;
	int mDebug;
	int mDump;
};

extern CConfig gConfig;

