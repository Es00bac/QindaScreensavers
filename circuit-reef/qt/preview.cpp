// SPDX-License-Identifier: GPL-3.0-or-later
#include "CircuitReefItem.hpp"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
int main(int argc,char**argv){
 QGuiApplication app(argc,argv);registerCircuitReefQmlType();QQmlApplicationEngine engine;
 engine.loadData(R"QML(
import QtQuick
import QtQuick.Window
import QindaQt.CircuitReef 1.0
Window {
 width: 1280; height: 720; visible: true
 title: "Circuit Reef | QindaQt embedding preview"
 CircuitReef {
  anchors.fill: parent
  active: true
  privateMetrics: true
  focus: true
  Keys.onEscapePressed: Qt.quit()
 }
}
)QML");
 if(engine.rootObjects().isEmpty())return 1;return app.exec();
}
