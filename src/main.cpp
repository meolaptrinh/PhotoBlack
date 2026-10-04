#include <wx/wx.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/aui/auibook.h>
wxString getpath(const wxString& path){
    wxString epath = wxStandardPaths::Get().GetExecutablePath();
    wxFileName fn(epath);
    wxString ep = fn.GetPath();
    wxFileName res;
    res.Assign(fn.GetPath() + "/" + path);
    return res.GetFullPath();
}
wxString tou8(const char*s){
    return wxString::FromUTF8(s);
}
class PbFrame:public wxFrame{
private:
    //trình quản lí tab
    wxAuiNotebook* notebook;
    //ảnh bìa
    wxImage cover_img;
    wxStaticBitmap* bitmapCover;
    //thanh menu 
    void CreateMenuBar(){
        wxMenuBar* menuBar = new wxMenuBar();
        wxMenu* fileMenu = new wxMenu();
        fileMenu->Append(wxID_OPEN,tou8("Mở\tCtrl+O"),tou8("mở một tệp ảnh đen trắng từ máy tính"));
        fileMenu->AppendSeparator();
        fileMenu->Append(wxID_EXIT,tou8("Thoát"),tou8("Đóng ứng dụng"));
        wxMenu* helpMenu = new wxMenu();
        helpMenu->Append(wxID_ABOUT,tou8("Thông tin"),tou8("Thông tin của ứng dụng"));
        helpMenu->AppendSeparator();
        helpMenu->Append(wxID_HELP,tou8("Hướng dẫn"),tou8("Sản phẩm này không phải là thuốc"));
        menuBar->Append(fileMenu,tou8("Tệp"));
        menuBar->Append(helpMenu,tou8("Thông tin"));
        SetMenuBar(menuBar);
        Bind(wxEVT_MENU,[this](wxCommandEvent& evn){Close(true);},wxID_EXIT);
        Bind(wxEVT_MENU,&PbFrame::openFile,this,wxID_OPEN);
        Bind(wxEVT_MENU,[this](wxCommandEvent& evn){wxLaunchDefaultApplication(getpath("../doc/tutorial.docx"));},wxID_HELP);
    }
    //trang welcome
    void CreateWelcomeTab(){
        wxPanel* panel = new wxPanel(notebook,wxID_ANY);
        //ảnh bìa 
        cover_img.LoadFile(getpath("../assets/img/cover.png"));
        bitmapCover = new wxStaticBitmap(panel,wxID_ANY,wxBitmap(cover_img));
        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
        mainSizer->Add(bitmapCover,1,wxEXPAND|wxALL,5);
        panel->SetSizer(mainSizer);
        panel->Bind(wxEVT_SIZE,&PbFrame::coverResize,this);
        notebook->AddPage(panel,tou8("Chào mừng"),true);
    }
    // các tab ảnh
    
public:
    PbFrame():wxFrame(nullptr,wxID_ANY,"PhotoBlack",wxDefaultPosition,wxSize(1024,650)){
        notebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_CLOSE_ON_ALL_TABS | wxAUI_NB_MIDDLE_CLICK_CLOSE);
        //thanh menu
        CreateMenuBar();
        CreateWelcomeTab();
    }
private: //event
    void coverResize(wxSizeEvent& evn){
        evn.Skip();
        wxSize newsize = evn.GetSize();
        if(newsize.x<=0||newsize.y<=0||!cover_img.IsOk())return;
        wxImage scl = cover_img;
        scl.Rescale(newsize.x,newsize.y,wxIMAGE_QUALITY_HIGH);
        bitmapCover->SetBitmap(wxBitmap(scl));
    }
    void openFile(wxCommandEvent& evn){
        wxFileDialog openDlg(this,tou8("Chọn tệp ảnh"),"","",tou8("Tệp ảnh (*.png;*.jpg(Không hỗ trợ 12-bit);*.jpeg(Không hỗ trợ 12-bit);*.bmp;*.gif)|*.png;*.jpg;*.jpeg;*.bmp;*.gif"),wxFD_OPEN|wxFD_FILE_MUST_EXIST);
        if(openDlg.ShowModal() == wxID_OK){
            wxString fpath = openDlg.GetPath();
            std::cout<<fpath<<"\n";
        }
    }
};
class PbApp:public wxApp{
public:
    virtual bool OnInit() override{
        wxInitAllImageHandlers();
        PbFrame* frame = new PbFrame();
        frame->Show(true);
        return true;
    }
};
wxIMPLEMENT_APP(PbApp);