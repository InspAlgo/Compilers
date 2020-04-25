import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

import QToDFA 1.0

Window {
    visible: true
    width: 1080
    height: 480
    title: qsTr("NFA确定化为DFA")

    // 背景图片
    Image {
        anchors.fill: parent
        fillMode: Image.Stretch
        smooth: true
        source: "qrc:/img/m.jpg"
    }

    QToDFA {
        id: dfa
    }

    // NFA 输入区域
    Rectangle {
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 10
        width: parent.width/2-100
        height: parent.height-20
        color: "#3edcdcdc"
        radius: 10

        ScrollView {
            anchors.fill: parent

            TextArea {
                id: textarea_input_nfa
                anchors.top: parent.top
                anchors.topMargin: 5
                anchors.left: parent.left
                anchors.leftMargin: 5
                width: parent.width-10
                height: parent.height-10

                font.pixelSize: 15
                color: "black"

                placeholderText: qsTr("输入 NFA")
            }
        }
    }

    Rectangle {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: parent.width/2-70
        width: 120
        height: 50
        color: "#3edcdcdc"
        radius: 10

        MouseArea {
            anchors.fill: parent
            Text {
                anchors.centerIn: parent
                text: qsTr("To DFA")
            }
            onClicked: {
                image.source = qsTr("")
                image.source = qsTr("file:///") + dfa.get_dir() + dfa.to_dfa(textarea_input_nfa.text)
                dfa.remove_temp_files()
            }
        }
    }

    Rectangle {
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 10
        width: parent.width/2-100
        height: parent.height-20
        color: "#3edcdcdc"
        radius: 10

        Image {
            id: image
            anchors.fill: parent
            fillMode: Image.Stretch
            smooth: true
            cache: false
            sourceSize.height: 500
            sourceSize.width: 400
        }
    }
}
