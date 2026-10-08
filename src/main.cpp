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
// Hệ số f
std::vector<std::vector<double>>fpx = {{0.15,0.5,1},{0.509,1,0.194}};
struct px{
    int r;
    int g;
    int b;
};
struct bwimg{
    int stand; //0 = Orthochromatic, 1 = Rec. 601
    int w;
    int h;
    std::vector<std::vector<px>>pixels;
    wxString path;
    wxString name;
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
//thuật toán zoom 
unsigned char* zoomimg(bwimg img, int k){
    unsigned char *res = (unsigned char*)malloc(img.h*img.w*3*k*k);
    std::vector<std::vector<px>>respx(img.w*k,std::vector<px>(img.h*k));
    for(int x = 0;x<img.w;x++){
        for(int y = 0;y<img.h;y++){
            px clr = img.pixels[x][y];
            for(int i = 0;i<k;i++){
                for(int j = 0;j<k;j++){
                    respx[x*k+i][y*k + j] = clr;
                }
            }
        }
    }
    for(int x = 0;x<img.w*k;x++){
        for(int y = 0;y<img.h*k;y++){
            int ind = (y*img.w*k+ x)*3; 
            res[ind] = respx[x][y].r;
            res[ind+1] = respx[x][y].g;
            res[ind+2] = respx[x][y].b;
        }
    }
    return res;
}
//các hình ảnh tạo sẵn
// một hình vuông màu 
unsigned char* colorSqr(int w,px cl){
    unsigned char* res = (unsigned char*)malloc(w*w*3);
    for(int i = 0;i<w*w*3;i+=3){
        res[i] = cl.r;
        res[i+1] = cl.g;
        res[i+2] = cl.b;
    }
    return res;
}
//hình vuông hue rgb
unsigned char* huergbSqr(double h){
    double li = 1.0;
    double s = 0.0;
    unsigned char*res = (unsigned char*)malloc(200*200*3);
    int j = 0;
    for(int i = 0;i<200*200*3;i+=3){
        if(i/3/200 != j){
            li -= 0.005;
            s = 0;
            j++;
        }
        wxImage::HSVValue hsv(h/360,s,li);
        wxImage::RGBValue rgb = wxImage::HSVtoRGB(hsv);
        res[i] = rgb.red;
        res[i+1] = rgb.green;
        res[i+2] = rgb.blue;
        s += 0.005;
    }
    return res;
}
// dải màu hue 
unsigned char* huebarRec(){
    double h = 0;
    unsigned char*res = (unsigned char*)malloc(15*200*3);
    int j = 0;
    for(int i = 0;i<15*200*3;i+=3){
        if(i/3/200 != j){
            j++;
            h = 0;
        }
        wxImage::HSVValue hsv(h,1.0,1.0);
        wxImage::RGBValue rgb = wxImage::HSVtoRGB(hsv);
        res[i] = rgb.red;
        res[i+1] = rgb.green;
        res[i+2] = rgb.blue;
        h += 0.005;
    }
    return res;
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
        auto hovercsp = [csp,img,stan,alp,this](wxWindow* wid, std::vector<wxStaticText*> text)mutable{
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
            wid->Bind(wxEVT_LEFT_UP,[img,stan,alp,this](wxMouseEvent& evn)mutable{
                img.stand = stan;
                befCheckImg(img);
                notebook->DeletePage(notebook->GetPageIndex(alp));
            });
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
    //tab thông báo chọn chuẩn ảnh 
    void alertStanImg( wxString& fpath){
        wxPanel* alp = new wxPanel(notebook, wxID_ANY);
        alp->SetBackgroundColour(wxColor(255,255,255));
        wxBoxSizer* alpsz = new wxBoxSizer(wxHORIZONTAL);
        alp->SetSizer(alpsz);
        bwimg img;
        img.path = fpath;
        wxString upath;
        int cname = 0;
        for(int i = fpath.size()-1;i>=0;i--){
            if(fpath[i] == '\\' || fpath[i] == '/')break;
            if(cname == 15){upath += "...";break;}
            upath+=fpath[i];
            cname++;
        }
        std::reverse(upath.begin(),upath.end());
        img.name = upath;
        cardStanImg(alp,0,tou8("Phim Orthochromatic"),tou8("Loại hình ảnh trắng đen phổ biến trong lịch sử trong thế kỉ XIX-XX, được ưu tiên hơn về thuật toán."),
        "../assets/img/exO.jpg",img);
        cardStanImg(alp,1,tou8("Chuẩn Rec. 601"),tou8("Loại hình ảnh trắng đen theo chuẩn kĩ thuật, ra đời muộn hơn, độ chính xác cao hơn"),
        "../assets/img/exR6.tiff",img);
        
        notebook->AddPage(alp,tou8("Chọn chuẩn cho ")+tou8(upath),true);
    }
    // Kiểm tra ảnh, tiền xử lí và lấy dữ liệu màu ảnh  
    void befCheckImg(bwimg& img){
        int w,h,chn;
        unsigned char*data = stbi_load(img.path,&w,&h,&chn,3);
        std::vector<std::vector<px>>res(w,std::vector<px>(h));
        bool ask = false;
        for(int y = 0;y<h;y++){
            for(int x = 0;x<w;x++){
                int i = (y*w +x)*3;
                if(((data[i] != data[i+1]) || (data[i+1] != data[i+2])) && (!ask)){
                    if(wxMessageBox(tou8("Ảnh ") + img.name + tou8(" có những điểm ảnh không thuộc hệ trắng đen, bạn có muốn dùng tính năng sửa chữa nhanh?"),tou8("Thông báo"),wxYES_NO|wxICON_QUESTION,this)){
                        ask = true;
                    }else {stbi_image_free(data);return;}
                }
                unsigned char dt = std::max(fpx[img.stand][0]*data[i],std::max(fpx[img.stand][1]*data[i+1],fpx[img.stand][2]*data[i+2]));
                res[x][y].r = dt;
                res[x][y].g = dt;
                res[x][y].b = dt;
                data[i] = dt;
                data[i+1] = dt;
                data[i+2] = dt;
                }

        }
        img.pixels = res;
        img.h = h;
        img.w = w;
        workTabImg(img,data);
    }
    
    //tạo tab ảnh mới 
    void workTabImg(bwimg& img,unsigned char* imgpx){
        wxPanel* fwtp = new wxPanel(notebook,wxID_ANY);
        wxScrolledWindow* wtp = new wxScrolledWindow(fwtp,wxID_ANY);
        wxBoxSizer* fwtpsz = new wxBoxSizer(wxHORIZONTAL);
        fwtp->SetSizer(fwtpsz);
        wtp->SetCanFocus(true);
        wtp->SetFocus();
        //hiển thị ảnh 
        wxImage simg(wxSize(img.w,img.h),imgpx,nullptr);
        wxStaticBitmap* bmpsimg = new wxStaticBitmap(wtp,wxID_ANY,wxBitmap(simg));
        wxBoxSizer* wtpsz = new wxBoxSizer(wxVERTICAL);
        wtpsz->Add(bmpsimg,1,wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL,20);
        wtp->SetSizer(wtpsz);
        wtp->SetScrollRate(30,20);
        wtp->FitInside();
        //zoom và di chuyển ảnh
        int curzoom = 1;
        wtp->Bind(wxEVT_CHAR_HOOK,[simg,bmpsimg,curzoom,img,wtp](wxKeyEvent& evn)mutable{
            int key = evn.GetKeyCode();
            //zoom
            if(key == 'F' || key == 'f'){
                if(curzoom>1){
                    curzoom-=1;
                    wxImage simg2(wxSize(img.w*curzoom,img.h*curzoom),zoomimg(img,curzoom),nullptr,false);
                    bmpsimg->SetBitmap(simg2);
                    wtp->FitInside();
                    wtp->Layout();
                }
            }
            if(key == 'G' || key == 'g'){
                if(curzoom<8){
                    curzoom += 1;
                    wxImage simg2(wxSize(img.w*curzoom,img.h*curzoom),zoomimg(img,curzoom),nullptr,false);
                    bmpsimg->SetBitmap(simg2); 
                    wtp->FitInside();
                    wtp->Layout();
                }
            }
            evn.Skip();
        });
        //thanh công cụ 
        px usclr = {20,0,125};
        wxPanel* tlpn = new wxPanel(fwtp,wxID_ANY,wxPoint(0,0),wxSize(50,520));
        tlpn->SetBackgroundColour(wxColor(255,255,255));
        wxBoxSizer* tlpnsz = new wxBoxSizer(wxVERTICAL);
        tlpn->Raise();
        // hình vuông màu
        wxImage showsqr(wxSize(30,30),colorSqr(30,usclr));
        wxBitmapButton* btnsqr = new wxBitmapButton(tlpn,wxID_ANY,wxBitmap(showsqr));
        tlpnsz->Add(btnsqr,0,wxALIGN_CENTER_HORIZONTAL|wxALL,10);
        tlpn->SetSizer(tlpnsz);
        tlpn->Layout();
        //rgb picker 
        wxPanel* rpkp = new wxPanel(fwtp,wxID_ANY,wxPoint(70,0),wxSize(300,300));
        rpkp->SetBackgroundColour(wxColor(0,0,0));
        rpkp->Raise();
        rpkp->Hide();
        wxBoxSizer* rpkpsz = new wxBoxSizer(wxVERTICAL);
        rpkp->SetSizer(rpkpsz);
        //hình vuông chọn 
        wxImage picksqr(wxSize(200,200),huergbSqr(0));
        wxStaticBitmap* bmppick = new wxStaticBitmap(rpkp,wxID_ANY,wxBitmap(picksqr));
        
        rpkpsz->Add(bmppick,0,wxALL|wxFIXED_MINSIZE,5);
        //Hue Slider 
        wxSlider* huesl = new wxSlider(rpkp,wxID_ANY,0,0,360,wxPoint(0,230),wxSize(200,30),wxSL_HORIZONTAL);
        rpkpsz->Add(huesl);
        //dải Hue 
        wxImage huerec(wxSize(200,15),huebarRec());
        wxStaticBitmap* bmphrec = new wxStaticBitmap(rpkp,wxID_ANY,wxBitmap(huerec));
        bmphrec->Disable();
        rpkpsz->Add(bmphrec,0,wxALL,10);
        huesl->Bind(wxEVT_SLIDER,[bmppick,rpkp](wxCommandEvent& evn){
            int hue = evn.GetInt();
            wxImage picksqr2(wxSize(200,200),huergbSqr(hue));
            bmppick->SetBitmap(picksqr2);
            rpkp->Refresh(false);
        });
        // khi hiện hay ẩn picker 
        btnsqr->Bind(wxEVT_BUTTON,[rpkp](wxCommandEvent& evn)mutable{
            rpkp->Show(!rpkp->IsShown());
            if(rpkp->IsShown()){
                rpkp->Raise();
                rpkp->Layout();
                rpkp->Refresh();
            }
        });
        //click chọn màu 
        auto pckerfunc = [btnsqr,huesl,usclr](wxMouseEvent& evn)mutable{
            if(evn.LeftDown() || (evn.Dragging() && evn.LeftIsDown())){
                int clx = evn.GetPosition().x;
                int cly = evn.GetPosition().y;
                double s = clx*0.005;
                double v = 1.0-(cly*0.005);
                double h = huesl->GetValue();
                wxImage::HSVValue hsv(h/360,s,v);
                wxImage::RGBValue rgb = wxImage::HSVtoRGB(hsv);
                usclr.r = rgb.red;
                usclr.g = rgb.green;                
                usclr.b = rgb.blue;
                wxImage sqr2(wxSize(30,30),colorSqr(30,usclr));
                btnsqr->SetBitmap(sqr2);
                evn.Skip();
            }
        };
        bmppick->Bind(wxEVT_LEFT_DOWN,pckerfunc);
        bmppick->Bind(wxEVT_MOTION,pckerfunc);
        // chuột click lên ảnh
        bmpsimg->Bind(wxEVT_LEFT_UP,[bmpsimg](wxMouseEvent& evn){
            int clx = evn.GetPosition().x;
            int cly = evn.GetPosition().y;
        });
        // khi cuộn
        auto scroll = [rpkp](wxScrollWinEvent& evn) {
            if (rpkp->IsShown()) rpkp->Show(false);
            evn.Skip();
        };
        wtp->Bind(wxEVT_SCROLLWIN_THUMBTRACK, scroll);
        wtp->Bind(wxEVT_SCROLLWIN_LINEUP, scroll);
        wtp->Bind(wxEVT_SCROLLWIN_LINEDOWN, scroll);
        wtp->Bind(wxEVT_SCROLLWIN_PAGEUP, scroll);
        wtp->Bind(wxEVT_SCROLLWIN_PAGEDOWN, scroll);
        fwtpsz->Add(tlpn,0);
        fwtpsz->Add(wtp,1,wxEXPAND);
        notebook->AddPage(fwtp,tou8(img.name),true);
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