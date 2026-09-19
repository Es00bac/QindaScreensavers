// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Shared native presentation: one output-bound surface per monitor, independent
// GL contexts, output hotplug, logical input coordinates and physical framebuffers.
#include <QGuiApplication>
#include <QScreen>
#include <QWindow>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPointer>
#include <QCursor>
#include <QEventLoop>
#include <QImage>
#include <QDir>
#include <QOpenGLFunctions>
#include <LayerShellQt/Window>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

namespace saver {
struct Presentation { int screen=-1,fps=60; double duration=0; bool list=false; std::string title,captureDir; };
class Window:public QWindow {
    std::chrono::steady_clock::time_point born_=std::chrono::steady_clock::now();
    QPointF baseline_;bool havePointer_=false;
protected:
    bool event(QEvent* e)override {
        const double age=std::chrono::duration<double>(std::chrono::steady_clock::now()-born_).count();
        if(e->type()==QEvent::Close){dismissed=true;return true;}
        if(e->type()==QEvent::KeyPress && (static_cast<QKeyEvent*>(e)->key()==Qt::Key_Escape||age>1.2))dismissed=true;
        if(age>1.2&&(e->type()==QEvent::MouseButtonPress||e->type()==QEvent::Wheel||e->type()==QEvent::TouchBegin))dismissed=true;
        if(e->type()==QEvent::MouseMove){auto p=static_cast<QMouseEvent*>(e)->position();if(havePointer_&&age>1.2&&std::hypot(p.x()-baseline_.x(),p.y()-baseline_.y())>6)dismissed=true;if(!havePointer_||age<=1.2)baseline_=p;havePointer_=true;}
        return QWindow::event(e);
    }
public:
    QScreen* assigned=nullptr;bool dismissed=false;QString capturePath;
    explicit Window(QScreen* output):QWindow(output),assigned(output){setScreen(output);setGeometry(output->geometry());setCursor(QCursor(Qt::BlankCursor));}
    virtual void present()=0;
};
template<class Renderer> class GLWindow final:public Window {
    std::unique_ptr<QOpenGLContext> context_;
    std::unique_ptr<Renderer> renderer_;
    std::function<std::unique_ptr<Renderer>(int,int)> create_;
    std::function<void(Renderer&)> draw_;unsigned frames_=0;
public:
    GLWindow(QScreen* output,std::function<std::unique_ptr<Renderer>(int,int)> create,std::function<void(Renderer&)> draw):Window(output),create_(std::move(create)),draw_(std::move(draw)){
        setSurfaceType(QSurface::OpenGLSurface);QSurfaceFormat format;format.setRenderableType(QSurfaceFormat::OpenGL);format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);format.setDepthBufferSize(24);format.setSwapInterval(0);setFormat(format);
    }
    ~GLWindow()override {if(context_&&context_->makeCurrent(this)){renderer_.reset();context_->doneCurrent();}}
    void present()override {
        if(!isExposed()||width()<1||height()<1)return;
        if(!context_){context_=std::make_unique<QOpenGLContext>();context_->setFormat(format());context_->setScreen(screen());if(!context_->create())throw std::runtime_error("Could not create per-output OpenGL context");}
        if(!context_->makeCurrent(this))throw std::runtime_error("Could not bind per-output OpenGL surface");
        int w=std::clamp(int(std::lround(width()*devicePixelRatio())),1,8192),h=std::clamp(int(std::lround(height()*devicePixelRatio())),1,8192);
        if(!renderer_)renderer_=create_(w,h);
        else if(renderer_->width()!=w||renderer_->height()!=h)renderer_->resize(w,h);
        draw_(*renderer_);
        if(++frames_==3&&!capturePath.isEmpty()){
            QImage image(w,h,QImage::Format_RGBA8888);auto* gl=context_->functions();gl->glPixelStorei(GL_PACK_ALIGNMENT,4);gl->glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,image.bits());
            if(!image.flipped(Qt::Vertical).save(capturePath))throw std::runtime_error("Could not save native monitor capture");
            std::cerr<<"OUTPUT_CAPTURED "<<assigned->name().toStdString()<<" "<<w<<"x"<<h<<" at "<<x()<<","<<y()<<"\n";
        }
        context_->swapBuffers(this);context_->doneCurrent();
    }
};
inline int run(int& argc,char** argv,const Presentation& config,
               const std::function<std::unique_ptr<Window>(QScreen*,int)>& factory,
               const std::function<void(double)>& advance,const std::function<bool()>& stopping){
    QGuiApplication app(argc,argv);auto screens=app.screens();
    if(config.list){for(int i=0;i<screens.size();++i){auto* s=screens[i];auto g=s->geometry();std::cout<<i<<" "<<s->name().toStdString()<<" "<<g.width()<<"x"<<g.height()<<" at "<<g.x()<<","<<g.y()<<" scale="<<s->devicePixelRatio()<<"\n";}return 0;}
    if(screens.empty())throw std::runtime_error("No monitors are connected");
    if(config.screen>=screens.size())throw std::runtime_error("Selected monitor is not connected");
    const bool layer=app.platformName()==QLatin1String("wayland");
    std::vector<std::unique_ptr<Window>> views;std::vector<QScreen*> removed;std::vector<QPointer<QScreen>> added;int sequence=0;
    auto add=[&](QScreen* screen){
        auto w=factory(screen,sequence++);w->setTitle(QString::fromStdString(config.title));
        if(!config.captureDir.empty()){QDir dir(QString::fromStdString(config.captureDir));if(!dir.mkpath("."))throw std::runtime_error("Could not create monitor capture directory");QString name=screen->name();name.replace('/', '_');w->capturePath=dir.filePath(name+".png");}
        if(layer){auto* surface=LayerShellQt::Window::get(w.get());surface->setScope(QStringLiteral("screensaver"));surface->setLayer(LayerShellQt::Window::LayerOverlay);
            surface->setAnchors({LayerShellQt::Window::AnchorTop,LayerShellQt::Window::AnchorBottom,LayerShellQt::Window::AnchorLeft,LayerShellQt::Window::AnchorRight});
            surface->setExclusiveZone(-1);surface->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityExclusive);surface->setScreen(screen);surface->setWantsToBeOnActiveScreen(false);surface->setCloseOnDismissed(true);w->show();
        }else w->showFullScreen();
        QObject::connect(screen,&QScreen::geometryChanged,w.get(),[ptr=w.get(),layer](const QRect& r){if(!layer)ptr->setGeometry(r);});
        std::cerr<<"OUTPUT_ADDED "<<screen->name().toStdString()<<" "<<screen->geometry().width()<<"x"<<screen->geometry().height()<<" scale="<<screen->devicePixelRatio()<<" shell="<<(layer?"layer-shell":"fullscreen")<<"\n";
        views.push_back(std::move(w));
    };
    if(config.screen<0)for(auto* s:screens)add(s);else add(screens[config.screen]);
    QObject::connect(&app,&QGuiApplication::screenAdded,&app,[&](QScreen* s){if(config.screen<0)added.push_back(s);});
    QObject::connect(&app,&QGuiApplication::screenRemoved,&app,[&](QScreen* s){removed.push_back(s);});
    using Clock=std::chrono::steady_clock;auto born=Clock::now(),previous=born;std::uint64_t frames=0;
    while(!stopping()){
        auto begin=Clock::now();app.processEvents(QEventLoop::AllEvents);
        for(auto* screen:removed){std::erase_if(views,[&](const auto& v){return v->assigned==screen;});std::cerr<<"OUTPUT_REMOVED\n";}removed.clear();
        for(const auto& s:added)if(s)add(s);
        added.clear();
        if(std::any_of(views.begin(),views.end(),[](const auto& v){return v->dismissed;}))break;
        if(config.screen>=0&&views.empty())break;
        double elapsed=std::chrono::duration<double>(begin-born).count();if(config.duration>0&&elapsed>=config.duration)break;
        bool visible=std::any_of(views.begin(),views.end(),[](const auto& v){return v->isExposed();});
        double dt=std::clamp(std::chrono::duration<double>(begin-previous).count(),0.,.1);previous=begin;
        if(visible){advance(dt);for(auto& v:views)v->present();++frames;}
        std::this_thread::sleep_until(begin+std::chrono::microseconds(visible?1000000/config.fps:100000));
    }
    std::cerr<<"Presented "<<frames<<" native frames across "<<views.size()<<" outputs\n";
    views.clear();return 0;
}
}
