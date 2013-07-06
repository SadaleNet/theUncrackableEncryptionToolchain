#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include <wx/app.h>
#endif

//(*AppHeaders
#include "GUI.h"
#include <wx/image.h>
//*)

GUI* frame;

class MyApp : public wxApp
{
	public:
		virtual bool OnInit();
};

IMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    GUI* Frame = new GUI(0);
    Frame->Show();
    SetTopWindow(Frame);
    }
    //*)
    return wxsOK;
}

