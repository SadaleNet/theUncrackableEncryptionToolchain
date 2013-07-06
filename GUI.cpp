#include "GUI.h"
#include "lib/uni.h"
#include "lib/native.h"

///TODO: set the style of GUI to wxDEFAULT_FRAME_STYLE, ~wxRESIZE_BORDER, ~wxRESIZE_BOX, ~wxMAXIMIZE_BOX) . It seems that wxSmith does not provide an option to set this.

std::string workingDir;

//define stuffs in GUIObjects.h here.
wxWindowList getObjects(wxWindow* parent, wxWindowID id){
	wxWindowList ret;
	wxWindowList& childList = parent->GetChildren();
	for(const auto& i: childList){
		if(i->GetId()==id)
			ret.push_back(i);
		wxWindowList subChildList = getObjects(i, id);
		for(const auto& j: subChildList)
			ret.push_back(j);
	}
	return ret;
}

template <class T>
wxWindowList getObjects(wxWindow* parent){
	wxWindowList ret;
	wxWindowList& childList = parent->GetChildren();
	for(wxWindowList::iterator it=childList.begin(); it!=childList.end(); it++){
		if(dynamic_cast<T*>(*it)!=nullptr)
			ret.push_back(*it);
		wxWindowList subChildList = getObjects<T>(*it);
		for(const auto& j: subChildList)
			ret.push_back(j);
	}
	return ret;
}
FileObjectsGroup* FileObjectsGroup::currentFirstPanel;

#ifndef WX_PRECOMP
	//(*InternalHeadersPCH(GUI)
	#include <wx/intl.h>
	#include <wx/string.h>
	//*)
#endif
//(*InternalHeaders(GUI)
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/image.h>
//*)

//(*IdInit(GUI)
const long GUI::ID_CUSTOM7 = wxNewId();
const long GUI::ID_STATICTEXT4 = wxNewId();
const long GUI::ID_CUSTOM24 = wxNewId();
const long GUI::ID_CUSTOM2 = wxNewId();
const long GUI::ID_CUSTOM8 = wxNewId();
const long GUI::ID_CUSTOM9 = wxNewId();
const long GUI::ID_CUSTOM4 = wxNewId();
const long GUI::ID_CUSTOM1 = wxNewId();
const long GUI::CRYPTER_PANEL = wxNewId();
const long GUI::ID_STATICTEXT1 = wxNewId();
const long GUI::ID_CUSTOM10 = wxNewId();
const long GUI::ID_CUSTOM3 = wxNewId();
const long GUI::ID_CUSTOM11 = wxNewId();
const long GUI::KEY_GEN_PANEL = wxNewId();
const long GUI::ID_CUSTOM21 = wxNewId();
const long GUI::ID_CUSTOM22 = wxNewId();
const long GUI::ID_CUSTOM19 = wxNewId();
const long GUI::ID_CUSTOM23 = wxNewId();
const long GUI::ID_CUSTOM20 = wxNewId();
const long GUI::ID_CUSTOM18 = wxNewId();
const long GUI::INJECTOR_PANEL = wxNewId();
const long GUI::ID_CUSTOM17 = wxNewId();
const long GUI::ID_CUSTOM16 = wxNewId();
const long GUI::ID_CUSTOM15 = wxNewId();
const long GUI::ID_CUSTOM6 = wxNewId();
const long GUI::ID_CUSTOM13 = wxNewId();
const long GUI::REDATE_PANEL = wxNewId();
const long GUI::ID_STATICTEXT2 = wxNewId();
const long GUI::ID_CUSTOM14 = wxNewId();
const long GUI::ID_STATICTEXT3 = wxNewId();
const long GUI::ID_CUSTOM5 = wxNewId();
const long GUI::ID_CUSTOM12 = wxNewId();
const long GUI::NUKE_PANEL = wxNewId();
const long GUI::ID_NOTEBOOK1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(GUI,wxFrame)
	//(*EventTable(GUI)
	//*)
END_EVENT_TABLE()

GUI::GUI(wxWindow* parent,wxWindowID,const wxPoint&,const wxSize&)
{
	//(*Initialize(GUI)
	wxBoxSizer* FlexGridSizer2;
	wxStaticBoxSizer* StaticBoxSizer2;
	wxBoxSizer* FlexGridSizer7;
	wxBoxSizer* FlexGridSizer5;
	wxStaticBoxSizer* StaticBoxSizer4;
	wxFlexGridSizer* FlexGridSizer10;
	wxBoxSizer* wxBoxSizer1;
	wxFlexGridSizer* FlexGridSizer9;
	wxStaticBoxSizer* StaticBoxSizer7;
	wxBoxSizer* FlexGridSizer6;
	wxBoxSizer* FlexGridSizer3;
	wxBoxSizer* FlexGridSizer8;
	wxBoxSizer* FlexGridSizer1;
	wxStaticBoxSizer* StaticBoxSizer8;
	wxStaticBoxSizer* StaticBoxSizer3;
	wxStaticBoxSizer* StaticBoxSizer6;
	wxBoxSizer* FlexGridSizer4;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxStaticBoxSizer* StaticBoxSizer5;
	
	Create(parent, wxID_ANY, _("GUI of The Uncrackable Encryption Tool Chain"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
	SetClientSize(wxSize(600,550));
	{
	wxIcon FrameIcon;
	FrameIcon.CopyFromBitmap(wxBitmap(wxImage(_T("icon.png"))));
	SetIcon(FrameIcon);
	}
	Notebook1 = new wxNotebook(this, ID_NOTEBOOK1, wxPoint(264,368), wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
	crypterPanel = new ProgramPanel(Notebook1, CRYPTER_PANEL, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("CRYPTER_PANEL"));
	wxBoxSizer1 = new wxBoxSizer(wxVERTICAL);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxVERTICAL, crypterPanel, _("Key file"));
	FlexGridSizer4 = new wxBoxSizer(wxHORIZONTAL);
	Custom7 = new OptionCheckBox(crypterPanel, 1, "Generate a key", 'g');
	FlexGridSizer4->Add(Custom7, 2, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(crypterPanel, ID_STATICTEXT4, _("Read offset:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer4->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Custom23 = new ParamSpinCtrl(crypterPanel, 4, 'o',0,2147483647,0);
	FlexGridSizer4->Add(Custom23, 3, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Custom2 = new FileObjectsGroup(crypterPanel, 5, true);
	StaticBoxSizer1->Add(Custom2, 3, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	wxBoxSizer1->Add(StaticBoxSizer1, 4, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxVERTICAL, crypterPanel, _("File to be en-decrypted"));
	FlexGridSizer2 = new wxBoxSizer(wxHORIZONTAL);
	Custom8 = new OptionCheckBox(crypterPanel, 2, "Decrypt mode", 'd');
	FlexGridSizer2->Add(Custom8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Custom9 = new OptionCheckBox(crypterPanel, 3, "Nuke this file afterward", 'n');
	FlexGridSizer2->Add(Custom9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer2->Add(FlexGridSizer2, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Custom4 = new FileObjectsGroup(crypterPanel, 6);
	StaticBoxSizer2->Add(Custom4, 3, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	wxBoxSizer1->Add(StaticBoxSizer2, 4, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer4 = new wxStaticBoxSizer(wxVERTICAL, crypterPanel, _("Output file"));
	Custom1 = new FileObjectsGroup(crypterPanel, 7);
	StaticBoxSizer4->Add(Custom1, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	wxBoxSizer1->Add(StaticBoxSizer4, 3, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	integratedCE = new CommandExecutor(crypterPanel, "./crypter -f");
	wxBoxSizer1->Add(integratedCE, 3, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	crypterPanel->SetSizer(wxBoxSizer1);
	wxBoxSizer1->Fit(crypterPanel);
	wxBoxSizer1->SetSizeHints(crypterPanel);
	keyGenPanel = new ProgramPanel(Notebook1, KEY_GEN_PANEL, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("KEY_GEN_PANEL"));
	FlexGridSizer1 = new wxBoxSizer(wxVERTICAL);
	StaticBoxSizer3 = new wxStaticBoxSizer(wxVERTICAL, keyGenPanel, _("Key Generator"));
	FlexGridSizer3 = new wxBoxSizer(wxHORIZONTAL);
	StaticText1 = new wxStaticText(keyGenPanel, ID_STATICTEXT1, _("Size of the key flie:"), wxDefaultPosition, wxSize(-1,-1), 0, _T("ID_STATICTEXT1"));
	FlexGridSizer3->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Custom10 = new FileSizeCtrl(keyGenPanel,1);
	FlexGridSizer3->Add(Custom10, 3, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer3->Add(FlexGridSizer3, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Custom3 = new FileObjectsGroup(keyGenPanel, 2, true);
	StaticBoxSizer3->Add(Custom3, 3, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(StaticBoxSizer3, 1, wxALL|wxEXPAND|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Custom11 = new CommandExecutor(keyGenPanel, "./keyGen -f");
	FlexGridSizer1->Add(Custom11, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	keyGenPanel->SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(keyGenPanel);
	FlexGridSizer1->SetSizeHints(keyGenPanel);
	injectorPanel = new ProgramPanel(Notebook1, INJECTOR_PANEL, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("INJECTOR_PANEL"));
	FlexGridSizer7 = new wxBoxSizer(wxVERTICAL);
	StaticBoxSizer7 = new wxStaticBoxSizer(wxVERTICAL, injectorPanel, _("Source file"));
	FlexGridSizer9 = new wxFlexGridSizer(0, 3, 0, 0);
	Custom20 = new OptionCheckBox(injectorPanel, 1, "Nuke this file after the operation", 'n');
	FlexGridSizer9->Add(Custom20, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Custom21 = new OptionCheckBox(injectorPanel, 3, "Discard the injected content after extraction.", 'd');
	FlexGridSizer9->Add(Custom21, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer7->Add(FlexGridSizer9, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Custom18 = new FileObjectsGroup(injectorPanel, 4, true);
	StaticBoxSizer7->Add(Custom18, 2, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7->Add(StaticBoxSizer7, 2, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer8 = new wxStaticBoxSizer(wxVERTICAL, injectorPanel, _("Destination of injection/extraction"));
	FlexGridSizer10 = new wxFlexGridSizer(0, 3, 0, 0);
	Custom22 = new OptionCheckBox(injectorPanel, 2, "Extraction mode", 'e');
	FlexGridSizer10->Add(Custom22, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer8->Add(FlexGridSizer10, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Custom19 = new FileObjectsGroup(injectorPanel, 5);
	StaticBoxSizer8->Add(Custom19, 2, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7->Add(StaticBoxSizer8, 2, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Custom17 = new CommandExecutor(injectorPanel, "./injector -f");
	FlexGridSizer7->Add(Custom17, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	injectorPanel->SetSizer(FlexGridSizer7);
	FlexGridSizer7->Fit(injectorPanel);
	FlexGridSizer7->SetSizeHints(injectorPanel);
	redatePanel = new ProgramPanel(Notebook1, REDATE_PANEL, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("REDATE_PANEL"));
	FlexGridSizer8 = new wxBoxSizer(wxVERTICAL);
	StaticBoxSizer6 = new wxStaticBoxSizer(wxVERTICAL, redatePanel, _("File(s) to be redated"));
	Custom16 = new DateTimeCtrl(redatePanel, 1, "atime", 'a');
	StaticBoxSizer6->Add(Custom16, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Custom15 = new DateTimeCtrl(redatePanel, 2, "mtime", 'm');
	StaticBoxSizer6->Add(Custom15, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	atimeCtrl = new DateTimeCtrl(redatePanel, 3, "ctime", 'c', true);
	StaticBoxSizer6->Add(atimeCtrl, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Custom6 = new FileObjectsGroup(redatePanel, 4, true);
	StaticBoxSizer6->Add(Custom6, 2, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer8->Add(StaticBoxSizer6, 2, wxALL|wxEXPAND|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
	FlexGridSizer8->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Custom13 = new CommandExecutor(redatePanel, "./redate");
	FlexGridSizer8->Add(Custom13, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	redatePanel->SetSizer(FlexGridSizer8);
	FlexGridSizer8->Fit(redatePanel);
	FlexGridSizer8->SetSizeHints(redatePanel);
	nukePanel = new ProgramPanel(Notebook1, NUKE_PANEL, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("NUKE_PANEL"));
	FlexGridSizer5 = new wxBoxSizer(wxVERTICAL);
	StaticBoxSizer5 = new wxStaticBoxSizer(wxVERTICAL, nukePanel, _("File(s) to be nuked"));
	FlexGridSizer6 = new wxBoxSizer(wxHORIZONTAL);
	StaticText2 = new wxStaticText(nukePanel, ID_STATICTEXT2, _("The file is to be overwritten"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer6->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Custom14 = new ParamSpinCtrl(nukePanel,1,'n',1,100,3);
	FlexGridSizer6->Add(Custom14, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(nukePanel, ID_STATICTEXT3, _("times."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer6->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer5->Add(FlexGridSizer6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Custom5 = new FileObjectsGroup(nukePanel, 2, true);
	StaticBoxSizer5->Add(Custom5, 3, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5->Add(StaticBoxSizer5, 1, wxALL|wxEXPAND|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
	FlexGridSizer5->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Custom12 = new CommandExecutor(nukePanel, "./nuke -f");
	FlexGridSizer5->Add(Custom12, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	nukePanel->SetSizer(FlexGridSizer5);
	FlexGridSizer5->Fit(nukePanel);
	FlexGridSizer5->SetSizeHints(nukePanel);
	Notebook1->AddPage(crypterPanel, _("Crypter"), false);
	Notebook1->AddPage(keyGenPanel, _("Key Gen"), false);
	Notebook1->AddPage(injectorPanel, _("Injector"), false);
	Notebook1->AddPage(redatePanel, _("Redate"), false);
	Notebook1->AddPage(nukePanel, _("Nuke"), false);
	Center();
	
	Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&GUI::OnClose);
	//*)
}

GUI::~GUI()
{
	//(*Destroy(GUI)
	//*)
}

void GUI::OnClose(wxCloseEvent& event)
{
	Close(true);
}

