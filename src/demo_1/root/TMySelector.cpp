
#include "TMySelector.h"

TMySelector::TMySelector()
{
	;
}

TMySelector::~TMySelector()
{
	;
}

void TMySelector::Begin(TTree *)
{
	;
}

void TMySelector::Terminate()
{
	;
}

void TMySelector::SlaveBegin()
{
	;
}

void TMySelector::SlaveTerminate()
{
	;
}

Bool_t TMySelector::Process(Long64_t entry)
{
	char pname[1024];
	double from, to, sum, step;

	sprintf(pname, "from_%lld", entry);
	from = ((TParameter<Double_t> *) fInput->FindObject(pname))->GetVal();

	sprintf(pname, "to_%lld", entry);
	to = ((TParameter<Double_t> *) fInput->FindObject(pname))->GetVal();

	for(sum = 0, step = (to - from) / 10000; from < to; from += step)
		sum += TMySelector::fun(from) * step;

	sprintf(pname, "res_%ld", entry);
	fOutput->Add(new TParameter<Double_t>(pname, sum));
}

double TMySelector::fun(double x)
{
	return x * x;
}

