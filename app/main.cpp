#include <utility.hpp>
#include <Wt/WApplication.h>
#include <Wt/WTemplate.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WPushButton.h>
#include <Wt/WComboBox.h>
#include <Wt/WLineEdit.h>
#include <Wt/WAnchor.h>
#include <Wt/WLink.h>
#include <Wt/WImage.h>
#include <cstdlib>
#include <thread>
#include <atomic>
#include <unistd.h>
#include <chrono>

#define COMMAND_SUCCESS 0
#define SUCCESS(a) ((a) == 0 && &(a) != nullptr)
std::atomic<bool> download_ready;

using namespace Wt;

class App: public WApplication {
    public:
    App(const WEnvironment & env);
    
};
App::App(const WEnvironment & env): WApplication(env) {
    setTitle("C++ Witty");
}

class AppTemplate : public WTemplate{
    public: 
    AppTemplate();
};


/*
TODO
Support: Audio -> OGG, AAC, FLAC, WAV,
Support: Video ->  AVI, MKV, WEBM

*/
enum{
    MP3, MP4, MOV
};

const string media_types_array [] = {
    [MP3] = "mp3"s,
    [MP4] = "mp4"s,
    [MOV] = "mov"s
};
const vector<string> media_types = {
  "mp3"s,
  "mp4"s,
  "mov"s
};


void sanitize_user_input(const string & input) {
    
}
/*
TODO: Ensure system input is sanitized
*/
void youtube_dl_download(const string & url, const int & type, const string & filename) {
    //❯ youtube-dl -x --audio-format mp3 https://youtu.be/yvsCeJ8p6pY --output "New song.%(ext)s"
    system("ls .");
    string pre = "cd ../app/docroot/Movies/ && "; //go to download folder -> Movies/
    string post = "cd ~-;"; //return to working folder
    switch(type) {
        case MP3:
            {
                
                string command = pre + fmt::format("youtube-dl -x --audio-format mp3  {} --output \"{}.%(ext)s\" && ", url, filename) + post;
                int res = system(command.c_str());
                if(SUCCESS(res)) {
                    download_ready = true;
                }
            }
            break;
        /*
            DONE: first request preffered format from youtube-dl, if not available download always available webm format then 
                  convert using ffmpeg
                
        */
        case MP4 ... MOV:
            {
                string command = pre + fmt::format("youtube-dl -f {} {} --output \"{}.%(ext)s\" &&", media_types[type], url, filename) + post;
                
                int res = system(command.c_str()); //download webm video
                if(SUCCESS(res)) {
                    download_ready = true;
                    break;
                }
                command = pre + fmt::format("youtube-dl -f webm {} --output \"{}.%(ext)s\" &&", url, filename, media_types[type]) + post;
                cout << fmt::format("Trying to convert video to {}", media_types[type]) << endl;
                res = system(command.c_str());
                if(SUCCESS(res)) {
                    command = pre + fmt::format("ffmpeg -i \"{0}.webm\" \"{0}.{1}\" && rm -rf {0}.webm &&", filename, media_types[type]) + post;
                    res = system(command.c_str());
                }
                if(SUCCESS(res)) {
                    download_ready = true;
                }else {
                    download_ready = false;
                }
            }
            break;
        default:
            {
                download_ready = false;
                cout << "Unsupported file format";
            }
            
    }
    
   
}

AppTemplate::AppTemplate() : WTemplate{tr("app-template")} {
    bindString("download_link", WString(""));
    bindString("error_result", WString(""));

    auto url = bindWidget("url_input", std::make_unique<WLineEdit>());
    auto filename = bindWidget("filename_input", std::make_unique<WLineEdit>());
    auto media = bindWidget("media_select", std::make_unique<WComboBox>());
    auto download = bindWidget("download_button", std::make_unique<WPushButton>("Download"));
    
    bindWidget("result", std::make_unique<WText>(""));
    //populate combo box list here
    for(auto item : media_types) {
        media->addItem(item);
    }
    media->setCurrentIndex(0); //default to first item in list
    //dowload url link here
    
    download->clicked().connect([=] {
        if(!(url->text().toUTF8() != ""s && (filename->text().toUTF8() != ""))) {
            bindWidget("result", std::make_unique<WText>(WString("<p>url is required!</p><p>filename is required !</p>")));
            return;
        }
        string filename_str = filename->text().toUTF8();
        string url_link = url->text().toUTF8();
        int type = std::find(media_types.begin(), media_types.end(), media->currentText().toUTF8()) - media_types.begin();
        bindWidget("result", std::make_unique<WText>("downloading..."));
        std::thread youtube(youtube_dl_download,url_link, type, filename_str);
        youtube.join(); //wait for the download to finish
        
        if(download_ready) {
            bindWidget("result", std::make_unique<WText>("download ready"));
            cout << "download is ready" << endl;
            string a = fmt::format("<a href=\"Movies/{0}.{1}\" alt=\"{0} {1}\" download=\"{0}{1}\">{0}.{1}</a>", filename_str, media_types[type]);
            auto anchor = WString::fromUTF8(a);
            auto download_link = bindWidget("result", std::make_unique<WText>(anchor));
            download_link->clicked().connect([=] {
                bindWidget("result", std::make_unique<WText>(""));
                url->setText("");
                filename->setText("");
                download_ready = false;
                std::this_thread::sleep_for(20s);
                //system("rm -rf ../app/docroot/Movies/*"); //clean folder after download
                return;
            });

        }else {
            bindWidget("result", std::make_unique<WText>("download failed"));
            std::this_thread::sleep_for(10s);
            return;
        }


    });


}

int main(int argc, char** argv) {
    WRun(argc, argv, [](const WEnvironment & env) {
        auto app = std::make_unique<App>(env);
        app->useStyleSheet("style.css");
        
        app->messageResourceBundle().use(app->appRoot() + "templates");
        // app->messageResourceBundle().use(app->appRoot() + "main");
        app->root()->addNew<AppTemplate>();
        
        return app;
    });
}