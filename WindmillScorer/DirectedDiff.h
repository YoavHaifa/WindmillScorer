#pragma once
#include "BaseScorer.h"
#include "..\..\ImageRLib\TImage.h"

class CDirectedDiff : public CBaseScorer
{
public:
	CDirectedDiff(CTImage<float>* pPrepDiff, bool bPositive, int iDir);
	~CDirectedDiff();

	void Update(CTImage<float>* pPrepDiff);
	CTImage<float>* GetAmp() { return mpScores; }

private:
	void GetSignedDiff(CTImage<float>* pPrepDiff);
	void ComputeDiffDir();
	void ComputeDiffDirAmp(int iDir);

	bool mbPositive;
	int miDir;

	CTImage<float>* mpDiff; // Always positive, >= 0
	CTImage<float>* mpDir; // Directed diff
};

