#include "patrol_window.hpp"
#include <QCursor>
#include <QEvent>
#include <QGuiApplication>
#include <QImage>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QScreen>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace patrol {
PatrolWindow::PatrolWindow(WindowOptions options,MetricSampler* sampler,QScreen* screen)
    : options_(options),sampler_(sampler),world_(options.seed),backing_(this) {
    setSurfaceType(QSurface::RasterSurface);
    setScreen(screen);
    setTitle(QStringLiteral("Qinda Patrol — Kind of Cute"));
    if(options.screensaver){setFlags(Qt::Window|Qt::FramelessWindowHint);setCursor(QCursor(Qt::BlankCursor));}
    else resize(1280,720);
    timer_.setTimerType(Qt::PreciseTimer);
    timer_.setInterval(std::max(1,int(std::ceil(1000.0/options.fps))));
    QObject::connect(&timer_,&QTimer::timeout,this,[this]{tick();});
    QObject::connect(this,&QWindow::screenChanged,this,[this](QScreen*){log("moved");});
    startupClock_.start();
    log("created");
}
void PatrolWindow::log(const char* what) const {
    if(!options_.verbose)return;
    const QScreen* s=screen();const QRect g=geometry();
    std::cerr<<"qinda-patrol: window "<<options_.index<<' '<<what<<" on "<<(s?s->name().toStdString():std::string("(no screen)"))
             <<" geometry "<<g.width()<<'x'<<g.height()<<" at "<<g.x()<<','<<g.y()<<(isExposed()?" exposed":" not exposed")<<'\n';
}
void PatrolWindow::exposeEvent(QExposeEvent*) {
    if(isExposed()){accumulator_=0;frameClock_.start();timer_.start();present();}
    else timer_.stop();
    log(isExposed()?"exposed":"hidden");
    if(exposureChanged)exposureChanged();
}
void PatrolWindow::resizeEvent(QResizeEvent* event){backing_.resize(event->size());if(isExposed())present();}
void PatrolWindow::tick(){
    if(!isExposed())return;
    double elapsed=frameClock_.isValid()?double(frameClock_.nsecsElapsed())/1e9:0;
    frameClock_.restart();accumulator_+=std::clamp(elapsed,0.0,0.2);
    int steps=0;while(accumulator_>=FixedDt && steps<24){world_.step();accumulator_-=FixedDt;++steps;}
    if(steps==24)accumulator_=0; // bounded catch-up: overload slows art, not the system
    present();
}
void PatrolWindow::present(){
    if(!isExposed()||width()<1||height()<1)return;
    Metrics m;
    if(options_.metricMode==MetricMode::Demo)m=demoMetrics(world_.time);
    else if(options_.metricMode==MetricMode::Hidden)m.mode=MetricMode::Hidden;
    else if(sampler_)m=sampler_->snapshot();
    const auto& frame=renderer_.render(world_,m,options_.overlay);
    QImage image(reinterpret_cast<const uchar*>(frame.pixels.data()),frame.width,frame.height,frame.width*int(sizeof(Color)),QImage::Format_ARGB32);
    const QRect rect(QPoint(0,0),size());backing_.beginPaint(rect);
    QPainter painter(backing_.paintDevice());painter.fillRect(rect,Qt::black);
    painter.setRenderHint(QPainter::SmoothPixmapTransform,false);
    double scale=std::min(double(width())/ViewW,double(height())/ViewH);
    QRectF target((width()-ViewW*scale)*0.5,(height()-ViewH*scale)*0.5,ViewW*scale,ViewH*scale);
    painter.drawImage(target,image);painter.end();backing_.endPaint();backing_.flush(rect);
}
bool PatrolWindow::event(QEvent* e){
    if(e->type()==QEvent::KeyPress){auto* key=static_cast<QKeyEvent*>(e);if(key->key()==Qt::Key_Escape || (options_.screensaver&&startupClock_.elapsed()>1200)){QGuiApplication::quit();return true;}}
    if(options_.screensaver&&startupClock_.elapsed()>1200){
        if(e->type()==QEvent::MouseMove){auto* mouse=static_cast<QMouseEvent*>(e);QPointF now=mouse->globalPosition();if(!mouseOrigin_)mouseOrigin_=now;else if(std::hypot(now.x()-mouseOrigin_->x(),now.y()-mouseOrigin_->y())>=4){QGuiApplication::quit();return true;}}
        else if(e->type()==QEvent::MouseButtonPress||e->type()==QEvent::Wheel||e->type()==QEvent::TouchBegin){QGuiApplication::quit();return true;}
    }
    return QWindow::event(e);
}
}
