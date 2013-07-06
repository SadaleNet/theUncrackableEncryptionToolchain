#ifndef GUI_H
#define GUI_H

#ifndef WX_PRECOMP
	//(*HeadersPCH(GUI)
	#include <wx/sizer.h>
	#include <wx/stattext.h>
	#include <wx/panel.h>
	#include <wx/frame.h>
	//*)
#endif
//(*Headers(GUI)
#include <wx/notebook.h>
//*)

class FileObjectsGroup;
class CommandExecutor;
class ProgramPanel;
class OptionCheckBox;
class FileSizeCtrl;
class ParamSpinCtrl;
class DateTimeCtrl;

class GUI: public wxFrame
{
	public:

		GUI(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~GUI();

		//(*Declarations(GUI)
		ParamSpinCtrl* Custom14;
		DateTimeCtrl* atimeCtrl;
		FileObjectsGroup* Custom5;
		wxNotebook* Notebook1;
		ProgramPanel* injectorPanel;
		FileObjectsGroup* Custom4;
		CommandExecutor* Custom11;
		wxStaticText* StaticText2;
		FileSizeCtrl* Custom10;
		FileObjectsGroup* Custom19;
		ProgramPanel* crypterPanel;
		CommandExecutor* integratedCE;
		FileObjectsGroup* Custom2;
		CommandExecutor* Custom12;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		OptionCheckBox* Custom9;
		FileObjectsGroup* Custom1;
		ParamSpinCtrl* Custom23;
		ProgramPanel* nukePanel;
		DateTimeCtrl* Custom16;
		FileObjectsGroup* Custom6;
		FileObjectsGroup* Custom3;
		OptionCheckBox* Custom20;
		ProgramPanel* redatePanel;
		OptionCheckBox* Custom7;
		OptionCheckBox* Custom8;
		CommandExecutor* Custom17;
		OptionCheckBox* Custom21;
		OptionCheckBox* Custom22;
		wxStaticText* StaticText4;
		CommandExecutor* Custom13;
		ProgramPanel* keyGenPanel;
		DateTimeCtrl* Custom15;
		FileObjectsGroup* Custom18;
		//*)

	protected:

		//(*Identifiers(GUI)
		static const long ID_CUSTOM7;
		static const long ID_STATICTEXT4;
		static const long ID_CUSTOM24;
		static const long ID_CUSTOM2;
		static const long ID_CUSTOM8;
		static const long ID_CUSTOM9;
		static const long ID_CUSTOM4;
		static const long ID_CUSTOM1;
		static const long CRYPTER_PANEL;
		static const long ID_STATICTEXT1;
		static const long ID_CUSTOM10;
		static const long ID_CUSTOM3;
		static const long ID_CUSTOM11;
		static const long KEY_GEN_PANEL;
		static const long ID_CUSTOM21;
		static const long ID_CUSTOM22;
		static const long ID_CUSTOM19;
		static const long ID_CUSTOM23;
		static const long ID_CUSTOM20;
		static const long ID_CUSTOM18;
		static const long INJECTOR_PANEL;
		static const long ID_CUSTOM17;
		static const long ID_CUSTOM16;
		static const long ID_CUSTOM15;
		static const long ID_CUSTOM6;
		static const long ID_CUSTOM13;
		static const long REDATE_PANEL;
		static const long ID_STATICTEXT2;
		static const long ID_CUSTOM14;
		static const long ID_STATICTEXT3;
		static const long ID_CUSTOM5;
		static const long ID_CUSTOM12;
		static const long NUKE_PANEL;
		static const long ID_NOTEBOOK1;
		//*)

	private:

		//(*Handlers(GUI)
		void OnClose(wxCloseEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};


#ifdef __MINGW32__
#include "lib/mingwSupp.h"
#endif
#include "GUIObjects.h"

#endif
