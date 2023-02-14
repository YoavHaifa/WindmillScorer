#pragma once
#include "BaseScorer.h"

class CHints : public CBaseScorer
{
public:
	CHints();

	void AddRoi(bool bWMA);
	void SaveAll();
	void RestoreSavedHints();

private:
	void SaveAll(CList<CDataRoi*, CDataRoi*>& hints);
	void Display(CList<CDataRoi*, CDataRoi*>& hints);
	void DisplayRestoredHints();
	void DisplayRoi(CDataRoi* pRoi);

	CList<CDataRoi*, CDataRoi*> mWMAHints;
	CList<CDataRoi*, CDataRoi*> mCleanHints;

};

extern CHints gHints;

