#ifndef GUI_OBJECTS_H
#define GUI_OBJECTS_H

#include <wx/button.h>
#include <wx/radiobut.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <wx/msgdlg.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/datetime.h>
#define wxUSE_DATEPICKCTRL 1
#include <wx/datectrl.h>
#include <boost/filesystem.hpp>
#include <vector>
#include <algorithm>
#include <iostream>

///TODO: remove these two lines after upgrading GCC.
#define override
#define final

class ParamPanel: public wxPanel{
	private:
		size_t priority;
	public:
		ParamPanel(wxWindow* parent, size_t priority):wxPanel(parent, -1){
			this->priority = priority;
		}
	virtual std::string getParam() = 0;
	size_t getPriority() const{ return this->priority; }
};

extern GUI* frame;
extern std::string workingDir;
#ifdef __WIN32__
    #include <direct.h>
    #define getWorkingDir _getcwd
#else
    #include <unistd.h>
    #define getWorkingDir getcwd
 #endif


enum{
	REGEX_MASTER_COMPONENT,
};

wxWindowList getObjects(wxWindow* parent, wxWindowID id);
template <class T> wxWindowList getObjects(wxWindow* parent);

class FileObjectsGroup: public ParamPanel{
	private:
		wxBoxSizer *boxSizer;
		wxFlexGridSizer *fileSizer, *regexSizer;
		wxStaticText *fileLabel, *regexLabel;
		wxTextCtrl *filePath, *regexExpr;
		wxString currentPath;
		class BrowseButton: public wxButton{
			private:
				FileObjectsGroup* fogPtr;
			public:
			BrowseButton(FileObjectsGroup* parent):wxButton(parent, wxID_ANY, _("Browse"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _("button")){
				this->fogPtr = parent;
				Connect(GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
					wxCommandEventHandler(BrowseButton::browse)
				);
			}
			void browse(wxCommandEvent&){
				if(fogPtr->isRegex()){ //select a directory
					wxDirDialog* browseDialog = new wxDirDialog(this, _("Select a directory as a regex base."));
					browseDialog->ShowModal();
					fogPtr->setPath(browseDialog->GetPath());
				}else{ //select a file
					wxFileDialog* browseDialog = new wxFileDialog(this, _("Select a file"), _(""),  _(""), _("*"), wxFD_SAVE|wxFD_CHANGE_DIR);
					browseDialog->ShowModal();
					fogPtr->setPath(browseDialog->GetPath());
				}
			}
		} *fileBrowseButton;

		class SubCheckBox: public wxCheckBox{
			private:
				wxWindow* panel;
			public:
			SubCheckBox(wxWindow* parent, wxWindowID id, const wxString& label, wxWindow* panel):wxCheckBox(parent, id, label){
				this->panel = panel;
				Connect(GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
					wxCommandEventHandler(SubCheckBox::en_DisableRegexMasterRadioButton)
				);
			}
			void en_DisableRegexMasterRadioButton(wxCommandEvent& event){
				wxWindowList objList = getObjects(panel, REGEX_MASTER_COMPONENT);
				for(auto& i:objList){
					if(event.IsChecked()) i->Enable();
					else i->Disable();
				}
			}

		} *regexEnableCheckBox;

		class RegexMasterCheckBox: public wxCheckBox{
			private:
				wxWindow* panel;
			public:
			RegexMasterCheckBox(wxWindow* parent, wxWindowID id, const wxString& label, wxWindow* panel):wxCheckBox(parent, id, label, wxDefaultPosition, wxDefaultSize,  0, wxDefaultValidator, _("checkBox")){
				this->SetValue(false);
				this->panel = panel;
				Connect(GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
					wxCommandEventHandler(RegexMasterCheckBox::reselect)
				);
			}
			void reselect(wxCommandEvent&){
				wxWindowList objList = getObjects(panel, REGEX_MASTER_COMPONENT);
				this->SetValue(true);
				for(auto& i:objList){
					RegexMasterCheckBox* button = dynamic_cast<RegexMasterCheckBox*>(i);
					if(button==nullptr||this==button) continue;
					button->SetValue(false);
				}
			}
		} *regexMasterRadioButton;
		FileObjectsGroup* firstPanel;
		static FileObjectsGroup* currentFirstPanel;
	public:
	FileObjectsGroup(wxWindow* parent, size_t priotiry, bool firstInPanel=false):ParamPanel(parent, priotiry){
		boxSizer = new wxBoxSizer(wxVERTICAL);

		fileSizer = new wxFlexGridSizer(0, 10, 0, 0);
		fileLabel = new wxStaticText(this, -1, _("File:"));
		filePath = new wxTextCtrl(this, -1, wxGetCwd(), wxDefaultPosition, wxSize(380, 25), wxTE_READONLY);
		fileBrowseButton = new BrowseButton(this); ///TODO: This button browse the files.
		/*fileSizer->Add(fileSizer, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);*/
		fileSizer->Add(fileLabel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
		fileSizer->Add(filePath, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
		fileSizer->Add(fileBrowseButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
		boxSizer->Add(fileSizer, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxEXPAND);

		regexSizer = new wxFlexGridSizer(0, 10, 0, 0);
		regexLabel = new wxStaticText(this, -1, _("Regex expression:"));
		regexExpr = new wxTextCtrl(this, REGEX_MASTER_COMPONENT, _(".*"), wxDefaultPosition, wxSize(300, 25));
		regexExpr->Disable();
		regexMasterRadioButton = new RegexMasterCheckBox(this, REGEX_MASTER_COMPONENT, _("Regex master"), parent);
		regexMasterRadioButton->SetValue(firstInPanel);
		regexMasterRadioButton->Disable();
		regexSizer->Add(regexLabel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
		regexSizer->Add(regexExpr, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
		regexSizer->Add(regexMasterRadioButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);

		if(firstInPanel){ //one this button per panel.
			regexEnableCheckBox = new SubCheckBox(this, wxID_ANY, _("Use regex"),parent ); ///TODO: This button enables and disables regexMasterRadioButton on the panel.
			regexEnableCheckBox->SetValue(false);
			fileSizer->Add(regexEnableCheckBox, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
		}

		boxSizer->Add(regexSizer, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxEXPAND);
		this->SetSizer(boxSizer);
		if(firstInPanel)
			currentFirstPanel = this;
		this->firstPanel = currentFirstPanel;
	}
	bool isRegex(){
		return firstPanel->regexEnableCheckBox->IsChecked();
	}
	void setPath(wxString path){
		filePath->SetValue(path);
		currentPath = path;
	}
	virtual std::string getParam() override{
		std::string ret;
		if(!isRegex())
			ret = std::string(filePath->GetValue().mb_str());
		else if(regexMasterRadioButton->GetValue())
			ret = std::string(filePath->GetValue().mb_str())+"///"+std::string(regexExpr->GetValue().mb_str());
		else
			ret = std::string(filePath->GetValue().mb_str())+"//"+std::string(regexExpr->GetValue().mb_str());
		return std::string("\"")+ret+"\" ";
	}
};

class OptionCheckBox: public ParamPanel{
	private:
		bool flagSet;
		char flag;
	public:
	OptionCheckBox(wxWindow* parent, size_t priotiry, const char* label, char flag):ParamPanel(parent, priotiry){
		new wxCheckBox(this, wxID_ANY, wxString(label, wxConvLocal));
		this->flagSet = false;
		this->flag = flag;
	}
	virtual bool ProcessEvent(wxEvent& event) override{
		if(event.GetEventType()==wxEVT_COMMAND_CHECKBOX_CLICKED)
			flagSet = static_cast<wxCommandEvent&>(event).IsChecked();
		return ParamPanel::ProcessEvent(event);
	}
	virtual std::string getParam() override{
		if(flagSet)
			return std::string("-")+flag+' ';
		return std::string();
	}
};

class FileSizeCtrl: public ParamPanel{
	private:
		wxSpinCtrl* fileSizeBaseCtrl;
		char* fileSizeMult;
	public:
	FileSizeCtrl(wxWindow* parent, size_t priotiry):ParamPanel(parent, priotiry){
		fileSizeBaseCtrl = new wxSpinCtrl(this, -1, _T("100"), wxDefaultPosition, wxSize(135,22), 0, 0, 1000000, 100);
		fileSizeBaseCtrl->SetValue(_T("100"));
		wxChoice* fileSizeMultCtrl = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
		fileSizeMultCtrl->Append(_("Bytes"), (char*)"");
		fileSizeMultCtrl->Append(_("Kilobytes(1000 Bytes)"), (char*)"kB");
		fileSizeMultCtrl->Append(_("Megabytes(1000*1000 Bytes)"), (char*)"MB");
		fileSizeMultCtrl->Append(_("Gigabytes(1000*1000*1000 Bytes)"), (char*)"GB");
		fileSizeMultCtrl->Append(_("Kibibytes(1024 Bytes)"), (char*)"K");
		fileSizeMultCtrl->SetSelection( fileSizeMultCtrl->Append(_("Mebibytes(1024*1024 Bytes)"), (char*)"M") );
		fileSizeMultCtrl->Append(_("Gibibytes(1024*1024*1024 Bytes)"), (char*)"G");
		wxBoxSizer *boxSizer = new wxBoxSizer(wxHORIZONTAL);
		boxSizer->Add(fileSizeBaseCtrl, 1, wxALL|wxALIGN_LEFT|wxEXPAND|wxALIGN_CENTER_VERTICAL);
		boxSizer->Add(fileSizeMultCtrl, 1, wxALL|wxALIGN_LEFT|wxEXPAND|wxALIGN_CENTER_VERTICAL);
		this->SetSizer(boxSizer);
		fileSizeMult = (char*)"M";
	}
	virtual bool ProcessEvent(wxEvent& event) override{
		try{
			if(event.GetEventType()==wxEVT_COMMAND_CHOICE_SELECTED)
				fileSizeMult = reinterpret_cast<char*>(static_cast<wxCommandEvent&>(event).GetClientData());
		}catch(...){}
		return ParamPanel::ProcessEvent(event);
	}
	virtual std::string getParam() override{
		return std::to_string(fileSizeBaseCtrl->GetValue())+fileSizeMult+' ';
	}
};

class DateTimeCtrl: public ParamPanel{
	private:
		wxDatePickerCtrl* date;
		wxDateTime* dateTime;
		wxSpinCtrl *hour, *min, *sec, *msec;
		#ifdef __WIN32__
		bool unixOnly;
		#elif defined(__unix__)||defined(__APPLE__)
		wxSpinCtrl *usec, *nsec;
		#endif
		char flag;
		bool initialized;
	public:
		DateTimeCtrl(wxWindow* parent, size_t priotiry, const char* label, char flag, bool unixOnly=false):ParamPanel(parent, priotiry){
			#ifdef __WIN32__
			this->unixOnly = unixOnly;
			if(unixOnly) return;
			#endif
			wxFlexGridSizer* sizer = new wxFlexGridSizer(0, 100, 0, 0);
			wxStaticText* labelObj = new wxStaticText(this, wxID_ANY, wxString(label, wxConvLocal));
			sizer->Add(labelObj, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);

			date = new wxDatePickerCtrl(this, wxID_ANY);
			//date->SetFormat(wxT("%d/%m/%Y"));
			date->SetToolTip(_("date"));
			sizer->Add(date, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);

			std::list<wxSpinCtrl**> objectPtrList {&hour, &min, &sec};
			size_t count = 0;
			for(auto& i: objectPtrList){
				count++;
				*i = new wxSpinCtrl(this, wxID_ANY, _T("0"), wxDefaultPosition, wxSize(40,20), 0, 0, 59);
				(*i)->SetToolTip(_("hour : min : sec"));
				sizer->Add(*i, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
				if(count<objectPtrList.size()){
					wxStaticText* colon = new wxStaticText(this, wxID_ANY, _(":"));
					sizer->Add(colon, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
				}
			}
			wxStaticText* dot = new wxStaticText(this, wxID_ANY, _("."));
			sizer->Add(dot, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);

			#ifdef __WIN32__
			std::list<wxSpinCtrl**> subSecondObjectPtrList = {&msec};
			#elif defined(__unix__)||defined(__APPLE__)
			std::list<wxSpinCtrl**> subSecondObjectPtrList = {&msec, &usec, &nsec};
			#endif

			for(auto& i: subSecondObjectPtrList){
				*i = new wxSpinCtrl(this, wxID_ANY, _T("0"), wxDefaultPosition, wxSize(50,20), 0, 0, 999);
				#ifdef __WIN32__
					(*i)->SetToolTip(_(".milliseconds"));
				#elif defined(__unix__)||defined(__APPLE__)
					(*i)->SetToolTip(_(".milliseconds.microsecond.nanosecond"));
				#endif
				sizer->Add(*i, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
			}
			this->SetSizer(sizer);
			this->flag = flag;
		}
		virtual std::string getParam() override{
			#ifdef __WIN32__
				if(this->unixOnly) return std::string();
			#endif
			std::function<std::string(int, size_t)> zeroPad = [](int value, size_t pad){
				assert(value>=0&&value<std::pow(10, pad));
				std::string ret = std::to_string(value);
				return std::string(pad-ret.length(), '0')+ret;
			};
			wxDateTime dateVal = date->GetValue();
			return std::string("-")+flag+std::string("=")
				+zeroPad(dateVal.GetYear(), 4)
				+zeroPad(dateVal.GetMonth()+1, 2)
				+zeroPad(dateVal.GetDay(), 2)
				+zeroPad(hour->GetValue(), 2)
				+zeroPad(min->GetValue(), 2)
				+'.'
				+zeroPad(sec->GetValue(), 2)
				+'.'
				+zeroPad(msec->GetValue(), 3)
				#if defined(__unix__)||defined(__APPLE__)
				+'.'
				+zeroPad(usec->GetValue(), 3)
				+'.'
				+zeroPad(nsec->GetValue(), 3)
				#endif
				+' ';
		}
};

class ParamSpinCtrl: public ParamPanel{
	private:
		wxSpinCtrl* spinCtrl;
		char flag;
	public:
	ParamSpinCtrl(wxWindow* parent, size_t priotiry, char flag, int min, int max, int defaultValue):ParamPanel(parent, priotiry){
		spinCtrl = new wxSpinCtrl(this, wxID_ANY, wxString::Format(wxT("%i"),defaultValue), wxDefaultPosition, wxDefaultSize, 0, min, max, defaultValue);
		this->flag = flag;
	}
	virtual std::string getParam() override{
 		return std::string("-")+flag+'='+std::to_string(spinCtrl->GetValue())+' ';
	}
};

extern std::string praseCommand(std::string command);
class CommandExecutor: public wxBoxSizer{
	public:
	class CommandTextCtrl: public wxTextCtrl{
		private:
			wxWindow* panel;
			std::string programName;
		public:
			CommandTextCtrl(wxWindow* parent, wxWindowID id, const std::string value = ""):wxTextCtrl(parent, id, wxString(value.c_str(), wxConvLocal), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY){
				panel = parent;
				this->programName = praseCommand(value);
			}
		void updateCommand(){
			wxWindowList paramList = getObjects<ParamPanel>(panel);
			if(paramList.empty()) return;
			/*Somehow, this does not work. :( The code seems fine. I've no idea why doesn't it work. I suspect it is a bug of wxWidget.*/
			/*paramList.Sort([](const void* a, const void* b){
					const ParamPanel* a2 = reinterpret_cast<const ParamPanel*>(a);
					const ParamPanel* b2 = reinterpret_cast<const ParamPanel*>(b);
					if(a2->getPriority() < b2->getPriority()) return -1;
					else if(a2->getPriority() == b2->getPriority()) return 0;
					return 1;
				});
				for(const auto &i: paramList){
					ParamPanel* param = dynamic_cast<ParamPanel*>(i);
					assert(param!=nullptr);
					command += param->getParam()+" ";
					break;
				}
			*/
			std::string command = programName+" ";
			for(size_t i=1;i<paramList.size()+1;i++){
				for(const auto &j: paramList){
					ParamPanel* param = dynamic_cast<ParamPanel*>(j);
					if(param->getPriority()==i){
						assert(param!=nullptr);
						command += param->getParam();
						break;
					}
				}
			}
			SetValue(wxString(command.c_str(), wxConvLocal));
		}
		std::string getCommand(){
			updateCommand();
			return std::string(GetValue().mb_str());
		}
		virtual bool ProcessEvent(wxEvent& event){
			return (event.GetEventType()==wxEVT_COMMAND_TEXT_UPDATED); //Since SetValue() triggers wxEVT_COMMAND_TEXT_UPDATED, and wxEVT_COMMAND_TEXT_UPDATED triggers updateCommand(), which triggers SetValue(), not preventing the event from being processed will lead to an infinitie recursion. By returning true, the further process of the event is prevented.
		}
	} *commandTextCtrl;

	class ExecuteButton: public wxButton{
		private:
			CommandTextCtrl* commandTextCtrl;
			wxWindow* parent;
		public:
		ExecuteButton(wxWindow* parent, wxWindowID, const wxString& value = _(""), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, CommandTextCtrl* commandTextCtrl=nullptr):wxButton(parent, wxID_ANY, wxString(value.c_str(), wxConvLocal), pos, size){
			Connect(this->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
				wxCommandEventHandler(ExecuteButton::execute));
			this->commandTextCtrl = commandTextCtrl;
			this->parent = parent;
		}
		void execute(wxCommandEvent&){
			std::string command = std::string("cd \"")+workingDir+std::string("\" && ");
			command += commandTextCtrl->getCommand();
			commandTextCtrl->getCommand();
			parent->Disable();
			this->SetLabel(_("Executing"));
			char logName[L_tmpnam] = {'\0'};
			tmpnam(logName);
			command += " 2> ";
			command += logName;
std::cerr<<command<<std::endl;
			if(system(command.c_str())==0){
				(new wxMessageDialog(parent, _("Command completed successfully."), _("Completed"), wxICON_INFORMATION|wxOK))->ShowModal();
			}else{
				#ifdef __WIN32__
				if((new wxMessageDialog(parent, _("Error occured. Show log file?"), _("Error"), wxICON_ERROR|wxYES_NO))->ShowModal()==wxID_YES){
					std::string logCommand = std::string("notepad ")+logName;
					system(logCommand.c_str());
					remove(logName);
				}
				#elif defined(__unix__)||defined(__APPLE__)
				std::string errorMessage = "Error occured. The error message is logged into ";
				errorMessage += logName;
				if((new wxMessageDialog(parent, wxString(errorMessage.c_str(), wxConvLocal), _("Error"), wxICON_ERROR|wxOK))->ShowModal()==wxID_YES){
				}
				#endif
			}
			parent->Enable();
			this->SetLabel(_("Execute"));
		}
	} *executeButton;

	CommandExecutor(wxWindow* parent, std::string programName):wxBoxSizer(wxHORIZONTAL){
		commandTextCtrl = new CommandTextCtrl(parent, -1, programName);
		executeButton = new ExecuteButton(parent, wxID_ANY, _("Execute"), wxDefaultPosition, wxSize(64,74), commandTextCtrl);
		this->Add(commandTextCtrl, 7, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
		this->Add(executeButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	}
};

class ProgramPanel: public wxPanel{
	public:
	ProgramPanel(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = _("panel")):wxPanel(parent, id, pos, size, style, name){
		workingDir.resize(65536);
		getWorkingDir(&workingDir[0], 65536);
		workingDir = workingDir.substr(0, workingDir.find('\0'));
	}
	virtual bool ProcessEvent(wxEvent& event){
		try{
			do{ //use do...while() to make break; works.
				if(!this->IsShownOnScreen()) break;
				dynamic_cast<wxCommandEvent&>(event);
				wxWindowList objectList = getObjects<CommandExecutor::CommandTextCtrl>(this);
				if(objectList.empty()) break;
				static_cast<CommandExecutor::CommandTextCtrl*>(objectList.front())->updateCommand();
			}while(false);
		}catch(...){}
		return wxPanel::ProcessEvent(event);
	}
};

#endif
