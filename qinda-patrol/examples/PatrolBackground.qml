// After registerQindaPatrolQmlTypes() in the C++ host; this is an embedding
// example, not a claimed existing QindaQt screensaver/lock-screen interface.
import QtQuick
import Qinda.Patrol 1.0

PatrolScene {
    id: patrol
    anchors.fill: parent
    // The host must control running according to idle/lock/output power state.
    running: visible
    // Default for a public/locked screen: no usage information exposed.
    metricsEnabled: false
    demo: false
}
