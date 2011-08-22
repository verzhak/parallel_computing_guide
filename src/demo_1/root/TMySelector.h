
#ifndef TMySelector_H
#define TMySelector_H

#include <TSelector.h>

class TMySelector : public TSelector 
{
	public:

		TMySelector();
		~TMySelector();

		virtual Int_t   Version() const { return 2; };
		virtual void    Begin(TTree *tree); 
		virtual void    SlaveBegin(TTree *tree);
		virtual Bool_t  ProcessCut(Long64_t entry);
		virtual void    SetOption(const char *option) { fOption = option; };
		virtual void    SetObject(TObject *obj) { ; };
		virtual void    SetInputList(TList *input) { fInput = input; };
		virtual TList  *GetOutputList() const { return fOutput; };
		virtual void    SlaveTerminate();
		virtual void    Terminate();

		static double fun(double x);

		ClassDef(TMySelector, 0);
};

#endif

