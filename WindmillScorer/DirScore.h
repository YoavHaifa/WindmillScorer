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
	void StartWrite();

	int miDir;

	CTSharedImage<float>* mpDirAmp;
	CTImage<float>* mpDirAmpCons;
	//CTSharedImage<float>* mpDirAmpSmooth;

	class CDirectedDiff* mpPosDir;
	class CDirectedDiff* mpNegDir;
};

