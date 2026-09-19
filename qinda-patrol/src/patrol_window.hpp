#pragma once
#include "renderer.hpp"
#include <QBackingStore>
#include <QElapsedTimer>
#include <QPointF>
#include <QTimer>
#include <QWindow>
#include <functional>
#include <optional>

namespace patrol {
struct WindowOptions {
    bool screensaver=false,overlay=true,verbose=false;
    int fps=30,index=0;
    std::uint64_t seed=551767;
    MetricMode metricMode=MetricMode::Live;
};
class PatrolWindow final : public QWindow {
public:
    PatrolWindow(WindowOptions options,MetricSampler* sampler,QScreen* screen);
    std::function<void()> exposureChanged;
protected:
    bool event(QEvent* event) override;
    void exposeEvent(QExposeEvent*) override;
    void resizeEvent(QResizeEvent*) override;
private:
    WindowOptions options_;
    MetricSampler* sampler_=nullptr; // owned by the application, outlives windows
    World world_;
    Renderer renderer_;
    QBackingStore backing_;
    QTimer timer_;
    QElapsedTimer frameClock_,startupClock_;
    double accumulator_=0;
    std::optional<QPointF> mouseOrigin_;
    void tick();
    void present();
    void log(const char* what) const;
};
}
