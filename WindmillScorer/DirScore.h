#pragma once
#include "BaseScorer.h"
#include "..\..\ImageRLib\TSharedImage.h"

class CDirScore : public CBaseScorer
{
public:
	CDirScore(int iDir);
	~CDirScore();

	float Compute(CTImage<float>* pPrepDiff);
	void Display();

private:
	void ComputeDiffDirsAmp();
	void BoostConsistency();
	void Display1(CTSharedImage<float> &shared, CTImage<float> &image);

	int miDir;

	static CTSharedImage<float>* umpDirAmpShared;
	static CTSharedImage<float>* umpScoresShared;

	CTImage<float>* mpDirAmp;
	CTImage<float>* mpDirAmpCons;

	class CDirectedDiff* mpPosDir;
	class CDirectedDiff* mpNegDir;
};

