#pragma once
#include "renderer.hpp"
#include <QElapsedTimer>
#include <QImage>
#include <QMutex>
#include <QQuickPaintedItem>
#include <QtQmlIntegration>
#include <QTimer>
#include <memory>
namespace patrol {
class PatrolItem : public QQuickPaintedItem {
    Q_OBJECT
    // Registered as Qinda.Patrol/PatrolScene by the qt_add_qml_module target so
    // a host that never links this library -- the screen locker's greeter, for
    // one -- can import the scene from QML alone.
    QML_NAMED_ELEMENT(PatrolScene)
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(bool metricsEnabled READ metricsEnabled WRITE setMetricsEnabled NOTIFY metricsEnabledChanged)
    Q_PROPERTY(bool demo READ demo WRITE setDemo NOTIFY demoChanged)
public:
    explicit PatrolItem(QQuickItem* parent=nullptr);
    void paint(QPainter* painter) override;
    bool running()const{return running_;} void setRunning(bool);
    bool metricsEnabled()const{return metricsEnabled_;} void setMetricsEnabled(bool);
    bool demo()const{return demo_;} void setDemo(bool);
signals:
    void runningChanged();void metricsEnabledChanged();void demoChanged();
private:
    bool running_=false,metricsEnabled_=false,demo_=false;
    World world_;
    Renderer renderer_;
    std::unique_ptr<MetricSampler> sampler_;
    QTimer timer_;
    QElapsedTimer clock_;
    double accumulator_=0;
    QMutex imageMutex_;
    QImage image_;
    void tick();void refreshImage();void refreshSampling();
};
// Call once, before constructing the host QQmlEngine.
void registerQindaPatrolQmlTypes();
}
