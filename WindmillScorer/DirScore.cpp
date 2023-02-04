#include "pch.h"
#include "DirScore.h"
#include "DirectedDiff.h"
#include "..\..\ImageRLib\Smoother.h"
#include "..\..\ImageRLib\ImageRIF.h"

CDirScore::CDirScore(int iDir)
	: CBaseScorer("DirScore", iDir)
	, miDir(iDir)
	, mpNegDir(NULL)
	, mpPosDir(NULL)
	, mpDirAmp(NULL)
	, mpDirAmpCons(NULL)
{
	mpDirAmp = new CTSharedImage<float>("DirAmp", umnLines, umnCols);
	mpDirAmpCons = new CTImage<float>("DirAmpCons", umnLines, umnCols);
	//mpDirAmpSmooth = new CTSharedImage<float>("DirAmpSmooth", umnLines, umnCols);

	mpDirAmp->SetWindowName("Amp");
	mpScores->SetWindowName("score");
}
CDirScore::~CDirScore()
{
	if (mpNegDir)
		delete mpNegDir;

	if (mpPosDir)
		delete mpPosDir;

	if (mpDirAmp)
		delete mpDirAmp;

	if (mpDirAmpCons)
		delete mpDirAmpCons;
}
void CDirScore::StartWrite()
{
	mpDirAmp->StartWrite();
	mpScores->StartWrite();
}
float CDirScore::Compute(CTImage<float>* pPrepDiff)
{
	StartWrite();

	if (!mpPosDir)
	{
		mpPosDir = new CDirectedDiff(pPrepDiff, true, miDir);
		mpNegDir = new CDirectedDiff(pPrepDiff, false, miDir);
	}
	else
	{
		mpPosDir->Update(pPrepDiff);
		mpNegDir->Update(pPrepDiff);
	}

	ComputeDiffDirsAmp();
	mScore = FindMax(mpScores);
	return mScore;
}
void CDirScore::Display()
{
	mpDirAmp->OnPageUpdate(0);
	mpScores->OnPageUpdate(0);

	umpImageRIF->DisplayShared(mpDirAmp);
	umpImageRIF->DisplayShared(mpScores);

	umpImageRIF->SetAutoWindow(mpDirAmp->Name(), mScoreCoo);
	umpImageRIF->SetAutoWindow(mpScores->Name(), mScoreCoo);
}
void CDirScore::ComputeDiffDirsAmp()
{
	Add(mpNegDir->GetAmp(), mpPosDir->GetAmp(), mpDirAmp);

	DumpImage(mpDirAmp);

	BoostConsistency();

	umpSmoother->SetSmoothFactor(5);
	umpSmoother->SmoothFloat(*mpScores, *mpDirAmpCons);

	DumpImage(mpDirAmpCons);
	DumpImage(mpScores);
}
void CDirScore::BoostConsistency()
{
	mpDirAmpCons->Zero();

	int delta = 4;
	float* pAmp = mpDirAmp->GetData();
	float* pCons = mpDirAmpCons->GetData();
	int envSide = delta * 2 + 1;
	int nInEnv = envSide * envSide;

	for (int iLine = delta; iLine < umnLines - delta; iLine++)
	{
		float* pAmpLine = pAmp + iLine * umnCols;
		float* pConsLine = pCons + iLine * umnCols;
		for (int iCol = delta; iCol < umnCols - delta; iCol++)
		{
			float amp = pAmpLine[iCol];
			if (amp == 0)
				continue;

			float low = amp / 2;
			float high = amp * 2;

			int n = 0;
			for (int iL = iLine - delta; iL <= iLine + delta; iL++)
			{
				float* pEnvLine = pAmp + iL * umnCols;
				for (int iC = iCol - delta; iC <= iCol + delta; iC++)
				{
					float ampEnv = pEnvLine[iC];
					if (ampEnv >= low && ampEnv <= high)
						n++;
				}
			}
			pConsLine[iCol] = amp * n / nInEnv;
		}
	}
}
