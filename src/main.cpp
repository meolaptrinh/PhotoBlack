#include <wx/wx.h>
class PbFrame:public wxFrame{
public:
    PbFrame():wxFrame(
        nullptr,
        wxID_ANY,
        "PhotoBlack",
        wxDefaultPosition,
        wxSize(1024,650)
    ){}
};
class PbApp:public wxApp{
public:
    virtual bool OnInit() override{
        PbFrame* frame = new PbFrame();
        frame->Show(true);
        return true;
    }
};
wxIMPLEMENT_APP(PbApp);