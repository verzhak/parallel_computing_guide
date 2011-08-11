
TProof * my_run(TProof * p, unsigned num, double from, double to)
{
	if(p != NULL)
		TProof * p = TProof::Open("localhost");

	TString sel = "TMySelector.cpp";

	unsigned u;
	double tfrom, tto, sum, rres;
	double step = (to - from) / num;
	char pname[1024];
	int nanosec;
	time_t sec;
	double tm[2];
	TTimeStamp tst;

	// ############################################################################ 

	for(u = 0, tfrom = from, tto = step; u < num; u++, tfrom += step, tto += step)
	{
		sprintf(pname, "from_%u", u);
		p->SetParameter(pname, tfrom);

		sprintf(pname, "to_%u", u);
		p->SetParameter(pname, tto);
	}

	sec = tst.GetSec();
	nanosec = tst.GetNanoSec();

	p->Process(sel, u);

	for(u = 0, sum = 0; u < num; u++)
	{
		sprintf(pname, "res_%u", u);
		sum += ((TParameter<Double_t> *) p->GetOutputList()->FindObject(pname))->GetVal();
	}

	tst.Set();
	tm[0] = tst.GetSec() - sec + 1 + (tst.GetNanoSec() - nanosec) / 1000000000.0;

	// ############################################################################ 

	tst.Set();
	sec = tst.GetSec();
	nanosec = tst.GetNanoSec();

	for(rres = 0, step = (to - from) / (num * 10000); from < to; from += step)
		rres += TMySelector::fun(from) * step;

	tst.Set();
	tm[1] = tst.GetSec() - sec + 1 + (tst.GetNanoSec() - nanosec) / 1000000000.0;

	// ############################################################################ 

	printf("\nРезультат:\n\n\tПараллельно = %lf (время: %lf секунд)\n\tПоследовательно = %lf (время: %lf секунд)\n\tАбсолютное отклонение = %lf\n\n",
			sum, tm[0], rres, tm[1], fabs(sum - rres));

	return p;
}

void my_down(TProof * p)
{
	delete p;
}

