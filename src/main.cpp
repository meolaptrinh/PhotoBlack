#include <wx/wx.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/aui/auibook.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"
#include<vector>
#include<string>
#include<algorithm>
#include<unordered_map>


//fonts
std::vector<wxFont>fplaypen;


struct px{
    int r;
    int g;
    int b;
};
struct bwimg{
    int stand; //0 = Orthochromatic, 1 = Rec. 601
    std::vector<std::vector<px>>pixels;
    wxString path;
};
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
    // các tác vụ ảnh
    //Thẻ chọn chuẩn ảnh 
    void cardStanImg(wxPanel* alp,int stan,wxString name,wxString des,wxString eximgp,bwimg img){
        wxPanel* csp = new wxPanel(alp,wxID_ANY);
        csp->SetBackgroundColour(wxColor(255,255,255));
        auto hovercsp = [csp,img,stan,this](wxWindow* wid, std::vector<wxStaticText*> text)mutable{
            auto checkHover = [csp,text](wxMouseEvent& evn) {
                wxPoint mpos = wxGetMousePosition();
                bool in = csp->GetScreenRect().Contains(mpos);
                if (in) {
                    csp->SetBackgroundColour(wxColor(0, 0, 0));
                    for (wxStaticText* t : text) {
                        t->SetForegroundColour(wxColor(255, 255, 255));
                        t->Refresh();
                    }
                } else {
                    csp->SetBackgroundColour(wxColor(255, 255, 255));
                    for (wxStaticText* t : text) {
                        t->SetForegroundColour(wxColor(0, 0, 0));
                        t->Refresh();
                    }
                }
                csp->Refresh();
                evn.Skip();
            };
            wid->Bind(wxEVT_ENTER_WINDOW, checkHover);
            wid->Bind(wxEVT_LEAVE_WINDOW, checkHover);
            wid->Bind(wxEVT_LEFT_UP,[img,stan,this](wxMouseEvent evn)mutable{img.stand = stan;befCheckImg(img);evn.Skip();});
        };

        wxBoxSizer* cspsz = new wxBoxSizer(wxVERTICAL);  
        wxImage eximg;
        eximg.LoadFile(getpath(eximgp));
        if(eximg.HasAlpha()){
            eximg.ClearAlpha();
        }
        eximg.Rescale(300,300,wxIMAGE_QUALITY_HIGH);
        wxStaticBitmap* bmeximg = new wxStaticBitmap(csp,wxID_ANY,wxBitmap(eximg));
        
        wxStaticText* title = new wxStaticText(csp,wxID_ANY,name);
        title->SetFont(fplaypen[18]);
        title->Wrap(280);
        wxStaticText* desc = new wxStaticText(csp,wxID_ANY,des);
        desc->SetFont(fplaypen[15]);
        desc->Wrap(280);

        std::vector<wxStaticText*>text = {title,desc};
        hovercsp(csp,text);
        hovercsp(bmeximg,text);
        hovercsp(title,text);
        hovercsp(desc,text);

        cspsz->Add(bmeximg,0,wxALL|wxALIGN_CENTER_HORIZONTAL,20);
        cspsz->Add(title,0,wxALL|wxALIGN_CENTER_HORIZONTAL,20);
        cspsz->Add(desc,0,wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT,20);

        csp->SetSizer(cspsz);
        wxSizer* alpsz = alp->GetSizer();
        alpsz->Add(csp,1,wxALL,30);
        alp->Layout();
    }
    //tab thông báo chọn chuẩn ảnh và đưa ra struct ảnh 
    void alertStanImg( wxString& fpath){
        wxPanel* alp = new wxPanel(notebook, wxID_ANY);
        alp->SetBackgroundColour(wxColor(255,255,255));
        wxBoxSizer* alpsz = new wxBoxSizer(wxHORIZONTAL);
        alp->SetSizer(alpsz);
        bwimg img;
        img.path = fpath;
        wxString upath;
        for(int i = fpath.size()-1;i>=0;i--){
            if(fpath[i] == '\\' || fpath[i] == '/')break;
            upath+=fpath[i];
        }
        std::reverse(upath.begin(),upath.end());

        cardStanImg(alp,0,tou8("Phim Orthochromatic"),tou8("Loại hình ảnh trắng đen phổ biến trong lịch sử trong thế kỉ XIX-XX, được ưu tiên hơn về thuật toán."),
        "../assets/img/exO.jpg",img);
        cardStanImg(alp,1,tou8("Chuẩn Rec. 601"),tou8("Loại hình ảnh trắng đen theo chuẩn kĩ thuật, ra đời muộn hơn, độ chính xác cao hơn"),
        "../assets/img/exR6.tiff",img);
        
        notebook->AddPage(alp,tou8("Chọn chuẩn cho ")+tou8(upath),true);
    }
    // Kiểm tra ảnh, tiền xử lí và lấy dữ liệu màu ảnh  
    bwimg befCheckImg(bwimg& img){
        int w,h,chn;
        unsigned char*data = stbi_load(img.path,&w,&h,&chn,3);
        std::vector<std::vector<px>>res(w,std::vector<px>(h));
        for(int i = 0;i<w*h*3;i+=3){
            res[i/3/h][i%h].r = data[i];
            res[i/3/h][i%h].g = data[i+1];
            res[i/3/h][i%h].b = data[i+2];
        }
        img.pixels = res;
        return img;
    }
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
            alertStanImg(fpath);
        }
    }
};
class PbApp:public wxApp{
public:
    virtual bool OnInit() override{
        wxInitAllImageHandlers();
        //load các tài nguyên
        //fonts
        //Playpen
        fplaypen.resize(51); 
        wxFont::AddPrivateFont(getpath("../assets/fonts/PlaypenSans-Regular.ttf"));
        for(int i = 1;i<=50;i++){
            fplaypen[i] = wxFontInfo(i).FaceName("Playpen Sans");
        }
        PbFrame* frame = new PbFrame();
        frame->Show(true);
        return true;
    }
};
wxIMPLEMENT_APP(PbApp);