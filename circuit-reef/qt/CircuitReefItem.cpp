// SPDX-License-Identifier: GPL-3.0-or-later
#include "CircuitReefItem.hpp"
#include <QPainter>
#include <QRandomGenerator>
#include <QtQml/qqml.h>
#include <QDebug>
CircuitReefItem::CircuitReefItem(QQuickItem* parent):QQuickPaintedItem(parent),renderer_(std::make_unique<reef::Renderer>(seed_)){
 setOpaquePainting(true);setAntialiasing(true);timer_.setInterval(33);timer_.setTimerType(Qt::PreciseTimer);
 connect(&timer_,&QTimer::timeout,this,&CircuitReefItem::refresh);
}
void CircuitReefItem::paint(QPainter* painter){
 auto image=latest_.load();if(image)painter->drawImage(boundingRect(),*image);else painter->fillRect(boundingRect(),QColor(2,12,25));
}
void CircuitReefItem::updateMonitor(){
 if(active_&&!private_){if(!monitor_)monitor_=std::make_unique<reef::Monitor>();}else monitor_.reset();
}
void CircuitReefItem::setActive(bool v){if(v==active_)return;active_=v;updateMonitor();if(v){clock_.start();timer_.start();refresh();}else timer_.stop();emit activeChanged();}
void CircuitReefItem::setPrivateMetrics(bool v){if(v==private_)return;private_=v;updateMonitor();emit privateMetricsChanged();}
void CircuitReefItem::setReducedMotion(bool v){if(v==reduced_)return;reduced_=v;emit reducedMotionChanged();}
void CircuitReefItem::setSeed(quint32 v){if(v==seed_)return;seed_=v;renderer_=std::make_unique<reef::Renderer>(seed_,palette_.toStdString());time_=0;emit seedChanged();}
void CircuitReefItem::setPalette(const QString& v){if(v==palette_)return;try{auto renderer=std::make_unique<reef::Renderer>(seed_,v.toStdString());palette_=v;renderer_=std::move(renderer);emit paletteChanged();}catch(const std::exception&e){qWarning()<<e.what();}}
void CircuitReefItem::setBrightness(double v){if(!std::isfinite(v))return;v=std::clamp(v,.05,1.0);if(qFuzzyCompare(v,brightness_))return;brightness_=v;emit brightnessChanged();}
void CircuitReefItem::regenerate(){setSeed(QRandomGenerator::global()->generate());}
void CircuitReefItem::refresh(){
 if(!active_)return;const double dt=std::min(clock_.restart()/1000.,.1);
 if(!isVisible()||width()<32||height()<32)return;
 time_+=dt*(reduced_?.4:1.0);
 double h=std::min(720.0,height()),w=h*width()/height();if(w>4096){h*=4096/w;w=4096;}
 reef::Metrics metrics;if(monitor_)metrics=monitor_->snapshot();else metrics.privateMode=true;
 reef::RenderOptions options;options.brightness=brightness_;
 try{
  const auto& frame=renderer_->render(std::max(32,int(w)),std::max(32,int(h)),time_,metrics,options);
  // The immutable deep copy is safe across the Qt Quick render thread boundary.
  QImage view(frame.data(),frame.width,frame.height,frame.stride(),QImage::Format_ARGB32_Premultiplied);
  latest_.store(std::make_shared<const QImage>(view.copy()));update();
 }catch(const std::exception&e){qWarning()<<"Circuit Reef render:"<<e.what();setActive(false);}
}
void registerCircuitReefQmlType(){qmlRegisterType<CircuitReefItem>("QindaQt.CircuitReef",1,0,"CircuitReef");}
