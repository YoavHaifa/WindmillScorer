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
	void CleanPreviousHints();
	void SaveAll(CList<CDataRoi*, CDataRoi*>& hints);
	void Display(CList<CDataRoi*, CDataRoi*>& hints);
	void DisplayRestoredHints();
	void DisplayRoi(CDataRoi* pRoi);

	void SaveDataMask(CDataRoi* pRoi);
	void AddToTarget(CTImage<bool> &mask);

	class CMultiDataF* mpTarget;
	CList<CDataRoi*, CDataRoi*> mWMAHints;
	CList<CDataRoi*, CDataRoi*> mCleanHints;

};

extern CHints gHints;

