#include "patrol_item.hpp"
#include <QMutexLocker>
#include <QPainter>
#include <QtQml/qqml.h>
#include <algorithm>
namespace patrol {
PatrolItem::PatrolItem(QQuickItem* parent):QQuickPaintedItem(parent){
    setOpaquePainting(true);setAntialiasing(false);setTextureSize(QSize(ViewW,ViewH));
    timer_.setInterval(33);timer_.setTimerType(Qt::PreciseTimer);
    connect(&timer_,&QTimer::timeout,this,[this]{tick();});refreshImage();
}
void PatrolItem::setRunning(bool v){if(running_==v)return;running_=v;clock_.start();accumulator_=0;if(v)timer_.start();else timer_.stop();refreshSampling();emit runningChanged();}
void PatrolItem::setMetricsEnabled(bool v){if(metricsEnabled_==v)return;metricsEnabled_=v;refreshSampling();refreshImage();emit metricsEnabledChanged();}
void PatrolItem::setDemo(bool v){if(demo_==v)return;demo_=v;refreshSampling();refreshImage();emit demoChanged();}
void PatrolItem::refreshSampling(){if(metricsEnabled_&&!demo_){if(!sampler_)sampler_=std::make_unique<MetricSampler>();sampler_->setActive(running_);}else sampler_.reset();}
void PatrolItem::tick(){double elapsed=double(clock_.nsecsElapsed())/1e9;clock_.restart();accumulator_+=std::clamp(elapsed,0.0,0.2);int n=0;while(accumulator_>=FixedDt&&n++<24){world_.step();accumulator_-=FixedDt;}if(n>=24)accumulator_=0;refreshImage();}
void PatrolItem::refreshImage(){
    Metrics m;if(demo_)m=demoMetrics(world_.time);else if(sampler_)m=sampler_->snapshot();else m.mode=MetricMode::Hidden;
    const auto& frame=renderer_.render(world_,m,false);
    QImage copied=QImage(reinterpret_cast<const uchar*>(frame.pixels.data()),ViewW,ViewH,ViewW*int(sizeof(Color)),QImage::Format_ARGB32).copy();
    {QMutexLocker guard(&imageMutex_);image_=std::move(copied);}update();
}
void PatrolItem::paint(QPainter* painter){
    QImage image;{QMutexLocker guard(&imageMutex_);image=image_;}
    painter->fillRect(boundingRect(),Qt::black);painter->setRenderHint(QPainter::SmoothPixmapTransform,false);
    double s=std::min(width()/ViewW,height()/ViewH);painter->drawImage(QRectF((width()-ViewW*s)*0.5,(height()-ViewH*s)*0.5,ViewW*s,ViewH*s),image);
}
void registerQindaPatrolQmlTypes(){qmlRegisterType<PatrolItem>("Qinda.Patrol",1,0,"PatrolScene");}
}
