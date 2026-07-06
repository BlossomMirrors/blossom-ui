import QtQuick
import org.kde.kirigami as Kirigami

Rectangle {
    id: root
    anchors.fill: parent
    color: "#000000"

    Column {
        id: content
        anchors.centerIn: parent
        spacing: 24

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: i18nd("blossomui_ksplash", "Welcome")
            color: "white"
            font.family: "Lora"
            font.italic: true
            font.pointSize: 46

            SequentialAnimation on opacity {
                loops: Animation.Infinite
                NumberAnimation {
                    from: 1.0
                    to: 0.3
                    duration: 1500
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    from: 0.3
                    to: 1.0
                    duration: 1500
                    easing.type: Easing.InOutQuad
                }
            }
        }

        Kirigami.Icon {
            id: spinner
            anchors.horizontalCenter: parent.horizontalCenter
            source: "process-working-symbolic"
            color: "white"
            width: Kirigami.Units.gridUnit * 2
            height: width
            opacity: 0.8

            RotationAnimator on rotation {
                from: 0
                to: 360
                duration: 2000
                loops: Animation.Infinite
                running: Kirigami.Units.longDuration > 1
            }
        }
    }
}
