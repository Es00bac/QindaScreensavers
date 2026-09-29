// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QQuickPaintedItem>
#include <QtQmlIntegration>
#include <QElapsedTimer>
#include <QImage>
#include <QTimer>
#include <atomic>
#include <memory>
#include "reef/renderer.hpp"
// Optional CPU-reference Qt Quick adapter. The standalone SDL application uses
// the accelerated DrawList backend. This adapter is for embedding in a host.
class CircuitReefItem : public QQuickPaintedItem {
 Q_OBJECT
 // Registered as QindaQt.CircuitReef/CircuitReef by the qt_add_qml_module
 // target, so a host that never links this library -- the screen locker's
 // greeter -- can import the scene from QML alone.
 QML_NAMED_ELEMENT(CircuitReef)
 Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
 Q_PROPERTY(bool privateMetrics READ privateMetrics WRITE setPrivateMetrics NOTIFY privateMetricsChanged)
 Q_PROPERTY(bool reducedMotion READ reducedMotion WRITE setReducedMotion NOTIFY reducedMotionChanged)
 Q_PROPERTY(quint32 seed READ seed WRITE setSeed NOTIFY seedChanged)
 // AGENT-GUARD: Qt 6.11 Item owns palette; a QString override aborts
 // strict native locker imports. See docs/ADR-0001-qt611-palette-property.md.
 Q_PROPERTY(QString reefPalette READ reefPalette WRITE setReefPalette NOTIFY reefPaletteChanged)
 Q_PROPERTY(double brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)
public:
 explicit CircuitReefItem(QQuickItem* parent=nullptr);
 void paint(QPainter*) override;
 bool active()const{return active_;}void setActive(bool);
 bool privateMetrics()const{return private_;}void setPrivateMetrics(bool);
 bool reducedMotion()const{return reduced_;}void setReducedMotion(bool);
 quint32 seed()const{return seed_;}void setSeed(quint32);
 QString reefPalette()const{return palette_;}void setReefPalette(const QString&);
 double brightness()const{return brightness_;}void setBrightness(double);
 Q_INVOKABLE void regenerate();
signals:
 void activeChanged();void privateMetricsChanged();void reducedMotionChanged();
 void seedChanged();void reefPaletteChanged();void brightnessChanged();
private:
 void refresh();void updateMonitor();
 bool active_=false,private_=true,reduced_=false;quint32 seed_=2026;
 QString palette_=QStringLiteral("lagoon");double brightness_=.86,time_=0;
 QTimer timer_;QElapsedTimer clock_;
 std::unique_ptr<reef::Renderer> renderer_;std::unique_ptr<reef::Monitor> monitor_;
 std::atomic<std::shared_ptr<const QImage>> latest_;
};
void registerCircuitReefQmlType();
