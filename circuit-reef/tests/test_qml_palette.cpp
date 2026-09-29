// SPDX-License-Identifier: GPL-3.0-or-later
#include "CircuitReefItem.hpp"
#include <QQmlComponent>
#include <QQmlEngine>
#include <QtTest>
class ReefQmlPaletteTest : public QObject {
 Q_OBJECT
private slots:
 void initTestCase() { qmlRegisterType<CircuitReefItem>("QindaQt.PrivateReefFixture", 1, 0, "CircuitReef"); }
 void qmlPropertyDoesNotOverrideBasePalette() {
   QQmlEngine engine;
   QQmlComponent component(&engine);
   component.setData("import QindaQt.PrivateReefFixture 1.0\nCircuitReef { reefPalette: \"amethyst\"; active: false; privateMetrics: true }", QUrl("qrc:/reef-palette-test.qml"));
   QVERIFY2(component.isReady(), qPrintable(component.errorString()));
   std::unique_ptr<QObject> object(component.create());
   QVERIFY2(object != nullptr, qPrintable(component.errorString()));
   QCOMPARE(object->property("reefPalette").toString(), QStringLiteral("amethyst"));
   QVERIFY(object->property("palette").metaType().id() != QMetaType::QString);
 }
 void nativePaletteNotification() {
   CircuitReefItem item;
   QSignalSpy changed(&item, &CircuitReefItem::reefPaletteChanged);
   item.setReefPalette(QStringLiteral("amethyst"));
   QCOMPARE(changed.size(), 1);
   QCOMPARE(item.reefPalette(), QStringLiteral("amethyst"));
   item.setReefPalette(QStringLiteral("amethyst"));
   QCOMPARE(changed.size(), 1);
 }
};
QTEST_MAIN(ReefQmlPaletteTest)
#include "test_qml_palette.moc"
