import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

import QLRParser 1.0
import QTableModel 1.0

Window {
    visible: true
    width: 720
    minimumWidth: 720
    height: 480
    minimumHeight: 480
    title: qsTr("LR 文法识别及输入串分析过程显示程序")

    QLRParser {
        id: parser
    }

    // 背景图片
    Image {
        anchors.fill: parent
        fillMode: Image.Stretch
        smooth: true
        source: "qrc:/img/m.jpg"
    }

    // 文法输入区域
    Rectangle {
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 10
        width: parent.width/4*3-20
        height: parent.height-20
        color: "#3edcdcdc"
        radius: 10

        ScrollView {
            anchors.fill: parent

            TextArea {
                id: textarea_input_grammar
                anchors.top: parent.top
                anchors.topMargin: 5
                anchors.left: parent.left
                anchors.leftMargin: 5
                width: parent.width-10
                height: parent.height-10

                font.pixelSize: 15
                color: "black"

                placeholderText: qsTr("在这里输入文法的产生式，一个产生式为一行\n产生式的各个符号请用空格隔开，-> 号左右也要用空格隔开，如：\nA -> a A b\nε符号可以直接使用，也可以用 epsilon 表示，如：\nA -> ε\nA -> epsilon")
            }
        }
    }

    // 文法起始符号与拓广文法起始符号输入区域
    Rectangle {
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 10
        width: parent.width/4-20
        height: 150
        color: "#3edcdcdc"
        radius: 10

        TextArea {
            id: textfield_start_token
            anchors.top: parent.top
            anchors.topMargin: 5
            anchors.horizontalCenter: parent.horizontalCenter  // 水平居中
            width: parent.width-10
            height: parent.height/2-7.5
            horizontalAlignment: TextArea.AlignHCenter
            verticalAlignment: TextArea.AlignVCenter
            placeholderText: qsTr("文法起始符号")
        }

        TextArea {
            id: textfield_new_start_token
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 5
            anchors.horizontalCenter: parent.horizontalCenter  // 水平居中
            width: parent.width-10
            height: parent.height/2-7.5
            horizontalAlignment: TextArea.AlignHCenter
            verticalAlignment: TextArea.AlignVCenter
            placeholderText: qsTr("拓广文法起始符号")
        }
    }

    Rectangle {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 15
        width: parent.width/4-25
        height: 50
        color: "#3edcdcdc"
        radius: 10

        MouseArea {
            anchors.fill: parent
            Text {
                anchors.centerIn: parent
                text: qsTr("构建 LR 分析表")
            }
            onClicked: {
                var start_token = textfield_start_token.text
                var new_start_token = textfield_new_start_token.text

                if (!start_token.length || !new_start_token.length) {
                    sendCriticalMessage(qsTr("请设置起始符号"))
                    return
                }

                parser.build_parsing_table(start_token, new_start_token, textarea_input_grammar.text)

                var grammar_type = parser.get_grammar_type();

                if (grammar_type === qsTr("Not")) {
                    grammar_type = qsTr("非 LR ")
                }

                subwindow_tools.title = grammar_type + qsTr(" 文法")
                subwindow_tools.open()
            }
        }
    }

    MessageDialog {
        id: message_critical
        visible: false
        title: qsTr("错误")
        icon: StandardIcon.Critical
    }

    function sendCriticalMessage(args) {
        message_critical.text = qsTr(args)
        message_critical.open()
    }

    MessageDialog {
        id: message_normal
        visible: false
        title: qsTr("消息")
        icon: StandardIcon.NoIcon
    }

    function sendNormalMessage(args) {
        message_normal.text = qsTr(args)
        message_normal.open()
    }

    Window {
        id: subwindow_tools
        title: qsTr("LR 文法信息")
        modality: Qt.NonModal

        width: 640
        minimumWidth: 640
        height: 480
        minimumHeight: 480

        function open() {
            stack_subpage.clear()
            sub_parsing_table_page.loadData()
            stack_subpage.push(sub_parsing_table_page)

            show()
        }

        Image {
            anchors.fill: parent
            fillMode: Image.Stretch
            smooth: true
            source: "qrc:/img/m.jpg"
        }

        Column {
            clip: true
            spacing: 5

            anchors.right: parent.right
            width: parent.width/5

            // 分析表 button
            Rectangle {
                width: parent.width
                height: 50
                color: "#3edcdcdc"
                radius: 10

                MouseArea {
                    anchors.fill: parent
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("分析表")
                    }
                    onClicked: {
                        stack_subpage.clear()
                        sub_parsing_table_page.loadData()
                        stack_subpage.push(sub_parsing_table_page)
                    }
                }
            }

            // 拓广文法 button
            Rectangle {
                width: parent.width
                height: 50
                color: "#3edcdcdc"
                radius: 10

                MouseArea {
                    anchors.fill: parent
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("拓广文法")
                    }
                    onClicked: {
                        stack_subpage.clear()
                        sub_expanding_grammar_page.loadData()
                        stack_subpage.push(sub_expanding_grammar_page)
                    }
                }
            }

            // 项目集族 button
            Rectangle {
                width: parent.width
                height: 50
                color: "#3edcdcdc"
                radius: 10

                MouseArea {
                    anchors.fill: parent
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("项目集族")
                    }
                    onClicked: {
                        stack_subpage.clear()
                        sub_items_sets_page.loadData()
                        stack_subpage.push(sub_items_sets_page)
                    }
                }
            }

            // 输入符号串分析 button
            Rectangle {
                width: parent.width
                height: 50
                color: "#3edcdcdc"
                radius: 10

                MouseArea {
                    anchors.fill: parent
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("输入符号串分析")
                    }
                    onClicked: {
                        textfield_input_tokens.clear()

                        var type = parser.get_grammar_type()
                        if (type === qsTr("Not"))
                            return

                        parsing_tokens_text.visible = true
                        tableView_pp.model = null
                        rows_header_model_pp.model = [" "]
                        stack_subpage.clear()
                        stack_subpage.push(sub_parsing_tokens_page)
                    }
                }
            }

            // DFA button
            Rectangle {
                width: parent.width
                height: 50
                color: "#3edcdcdc"
                radius: 10

                MouseArea {
                    anchors.fill: parent
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("DFA")
                    }
                    onClicked: {
                        stack_subpage.clear()
                        sub_dfa_page.loadData()
                        stack_subpage.push(sub_dfa_page)
                    }
                }
            }
        }

        StackView {
            id: stack_subpage
            initialItem: sub_parsing_table_page
            anchors.left: parent.left
            width: parent.width/5*4
            height: parent.height
        }

        ListModel {
            id: listmodel_line_data  // 逐行数据
        }

        QTableModel {
            id: qtablemodel
        }

        Page {
            id: sub_parsing_table_page  // 分析表页面
            Image {
                anchors.fill: parent
                fillMode: Image.Stretch
                smooth: true
                source: "qrc:/img/m.jpg"
            }

            Rectangle {
                width: parent.width
                height: parent.height
                anchors.fill: parent
                color: "#3edcdcdc"

                TableView {
                    id: tableView_pt
                    columnWidthProvider: function (column) { return 100; }
                    rowHeightProvider: function (column) { return 60; }
                    anchors.fill: parent
                    leftMargin: rowsHeader_pt.implicitWidth
                    topMargin: columnsHeader_pt.implicitHeight

                    ScrollBar.horizontal: ScrollBar{
                        height: 10
                        policy: ScrollBar.AlwaysOn
                    }

                    ScrollBar.vertical: ScrollBar{
                        width: 10
                        policy: ScrollBar.AlwaysOn
                    }

                    clip: true

                    delegate: Item {
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: display
                            color: '#333333'
                            font.pixelSize: 15
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }

                    Rectangle { // mask the headers
                        z: 3
                        color: "#87cefa"
                        y: tableView_pt.contentY
                        x: tableView_pt.contentX
                        width: tableView_pt.leftMargin
                        height: tableView_pt.topMargin
                    }

                    Row {
                        id: columnsHeader_pt
                        y: tableView_pt.contentY
                        z: 2
                        Repeater {
                            id: columns_header_model_pt
                            model: []
                            Label {
                                width: tableView_pt.columnWidthProvider(modelData)
                                height: 35
                                text: modelData
                                color: '#333333'
                                font.pixelSize: 15
                                padding: 10
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter

                                background: Rectangle { color: "#48d1cc" }
                            }
                        }
                    }
                    Column {
                        id: rowsHeader_pt
                        x: tableView_pt.contentX
                        z: 2
                        Repeater {
                            id: rows_header_model_pt
                            model: []
                            Label {
                                width: 60
                                height: tableView_pt.rowHeightProvider(modelData)
                                text: modelData
                                color: '#333333'
                                font.pixelSize: 15
                                padding: 10
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter

                                background: Rectangle { color: "#48d1cc" }
                            }
                        }
                    }

                    ScrollIndicator.horizontal: ScrollIndicator { }
                    ScrollIndicator.vertical: ScrollIndicator { }
                }
            }

            function loadData() {
                var col_header = parser.get_col_header()
                var row_header = parser.get_row_header()
                var parsing_table = parser.get_parsing_table()

                columns_header_model_pt.model = col_header
                rows_header_model_pt.model = row_header

                qtablemodel.set_table_data(row_header, col_header, parsing_table)
                tableView_pt.model = null
                tableView_pt.model = qtablemodel
            }
        }

        Page {
            id: sub_expanding_grammar_page  // 拓广文法页面

            Image {
                anchors.fill: parent
                fillMode: Image.Stretch
                smooth: true
                source: "qrc:/img/m.jpg"
            }

            Rectangle {
                width: parent.width
                height: parent.height
                anchors.fill: parent
                color: "#3edcdcdc"

                ListView {
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.topMargin: 10
                    anchors.bottomMargin: 10

                    width: parent.width
                    height: parent.height - 20

                    clip: true

                    model: listmodel_line_data

                    delegate: Item {
                        width: parent.width
                        height: 35

                        ScrollView {
                            anchors.fill: parent

                            TextArea {
                                text: model_data
                                font.pixelSize: 15
                                verticalAlignment: TextArea.AlignVCenter
                                readOnly: true
                            }
                        }
                    }

                    ScrollBar.vertical: ScrollBar {
                        width: 10
                        policy: ScrollBar.AlwaysOn
                    }
                }
            }

            function loadData() {
                listmodel_line_data.clear()

                var data = parser.get_expanding_grammar()

                for (var i in data) {
                    listmodel_line_data.append({"model_data": data[i]})
                }

            }
        }

        Page {
            id: sub_items_sets_page  // 项目集页面
            Image {
                anchors.fill: parent
                fillMode: Image.Stretch
                smooth: true
                source: "qrc:/img/m.jpg"
            }

            Rectangle {
                width: parent.width
                height: parent.height
                anchors.fill: parent
                color: "#3edcdcdc"

                ListView {
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.topMargin: 10
                    anchors.bottomMargin: 10

                    width: parent.width
                    height: parent.height - 20

                    clip: true

                    model: listmodel_line_data

                    delegate: Item {
                        width: parent.width
                        height: 35

                        ScrollView {
                            anchors.fill: parent

                            TextArea {
                                text: model_data
                                font.pixelSize: 15
                                verticalAlignment: TextArea.AlignVCenter
                                readOnly: true
                            }
                        }
                    }

                    ScrollBar.vertical: ScrollBar {
                        width: 10
                        policy: ScrollBar.AlwaysOn
                    }
                }
            }

            function loadData() {
                listmodel_line_data.clear()

                var data = parser.get_items_sets()

                for (var i in data)
                    listmodel_line_data.append({"model_data": data[i]})
            }
        }

        Page {
            id: sub_parsing_tokens_page  // 输入符号串分析页面
            Image {
                anchors.fill: parent
                fillMode: Image.Stretch
                smooth: true
                source: "qrc:/img/m.jpg"
            }

            Rectangle {
                width: parent.width
                height: parent.height
                anchors.fill: parent
                color: "#3edcdcdc"

                Rectangle {
                    anchors.top: parent.top
                    anchors.topMargin: 10
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    width: parent.width/4*3-20
                    height: 50
                    color: "#3edcdcdc"
                    radius: 10

                    TextArea {
                        id: textfield_input_tokens
                        anchors.top: parent.top
                        anchors.topMargin: 5
                        anchors.horizontalCenter: parent.horizontalCenter  // 水平居中
                        width: parent.width-10
                        height: parent.height
                        horizontalAlignment: TextArea.AlignHCenter
                        verticalAlignment: TextArea.AlignVCenter
                        placeholderText: qsTr("输入符号串(空格隔开)")
                    }
                }

                Rectangle {
                    anchors.top: parent.top
                    anchors.topMargin: 10
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    width: parent.width/4-25
                    height: 50
                    color: "#3edcdcdc"
                    radius: 10

                    MouseArea {
                        anchors.fill: parent

                        Text {
                            id: parsing_tokens_text
                            anchors.centerIn: parent
                            text: qsTr("分析")
                            visible: false  // 玄学 bug 如果是 false 它会出现在这个窗口的首页，如果切换页面它有就又没有这个 bug 。。。
                        }

                        onClicked: {
                            sub_parsing_tokens_page.parsing_tokens()
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: parent.height - 80
                    anchors.bottom: parent.bottom
                    color: "#00dcdcdc"

                    TableView {
                        id: tableView_pp
                        columnWidthProvider: function (column) { return 100; }
                        rowHeightProvider: function (column) { return 60; }
                        anchors.fill: parent
                        leftMargin: rowsHeader_pp.implicitWidth
                        topMargin: columnsHeader_pp.implicitHeight

                        ScrollBar.horizontal: ScrollBar{
                            height: 10
                            policy: ScrollBar.AlwaysOn
                        }

                        ScrollBar.vertical: ScrollBar{
                            width: 10
                            policy: ScrollBar.AlwaysOn
                        }

                        clip: true

                        delegate: Item {
                            ScrollView {
                                anchors.fill: parent
                                TextArea {
                                    text: display
                                    color: '#333333'
                                    font.pixelSize: 15
                                    readOnly: true
                                    horizontalAlignment: TextArea.AlignHCenter
                                    verticalAlignment: TextArea.AlignVCenter
                                }
                            }
                        }

                        Rectangle { // mask the headers
                            z: 3
                            color: "#87cefa"
                            y: tableView_pp.contentY
                            x: tableView_pp.contentX
                            width: tableView_pp.leftMargin
                            height: tableView_pp.topMargin

                            Label {
                                text: qsTr("步骤")
                                color: '#333333'
                                font.pixelSize: 15
                                padding: 10
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        Row {
                            id: columnsHeader_pp
                            y: tableView_pp.contentY
                            z: 2
                            Repeater {
                                id: columns_header_model_pp
                                model: [qsTr("状态栈"), qsTr("符号栈"), qsTr("输入符号串"), qsTr("分析动作")]
                                Label {
                                    width: tableView_pp.columnWidthProvider(modelData)
                                    height: 35
                                    text: modelData
                                    color: '#333333'
                                    font.pixelSize: 15
                                    padding: 10
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter

                                    background: Rectangle { color: "#48d1cc" }
                                }
                            }
                        }
                        Column {
                            id: rowsHeader_pp
                            x: tableView_pp.contentX
                            z: 2
                            Repeater {
                                id: rows_header_model_pp
                                model: []
                                Label {
                                    width: 60
                                    height: tableView_pp.rowHeightProvider(modelData)
                                    text: modelData
                                    color: '#333333'
                                    font.pixelSize: 15
                                    padding: 10
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter

                                    background: Rectangle { color: "#48d1cc" }
                                }
                            }
                        }

                        ScrollIndicator.horizontal: ScrollIndicator { }
                        ScrollIndicator.vertical: ScrollIndicator { }
                    }
                }
            }

            function parsing_tokens() {
                var col_header = [qsTr("state_stack"), qsTr("tokens_stack"), qsTr("input_tokens"), qsTr("parsing_action")]

                var parsing_process_table = parser.parsing_tokens(textfield_input_tokens.text)
                var row_header = parser.get_parsing_process_step_count()

                rows_header_model_pp.model = row_header
                qtablemodel.set_table_data(row_header, col_header, parsing_process_table)
                tableView_pp.model = null
                tableView_pp.model = qtablemodel
            }
        }

        Page {
            id: sub_dfa_page  // DFA 页面
            Image {
                anchors.fill: parent
                fillMode: Image.Stretch
                smooth: true
                source: "qrc:/img/m.jpg"
            }
            Rectangle {
                width: parent.width
                height: parent.height
                anchors.fill: parent
                color: "#3edcdcdc"

                ListView {
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.topMargin: 10
                    anchors.bottomMargin: 10

                    width: parent.width
                    height: parent.height - 20

                    clip: true

                    model: listmodel_line_data

                    delegate: Item {
                        width: parent.width
                        height: 35

                        ScrollView {
                            anchors.fill: parent

                            TextArea {
                                text: model_data
                                font.pixelSize: 15
                                verticalAlignment: TextArea.AlignVCenter
                                readOnly: true
                            }
                        }
                    }

                    ScrollBar.vertical: ScrollBar {
                        width: 10
                        policy: ScrollBar.AlwaysOn
                    }
                }
            }

            function loadData() {
                listmodel_line_data.clear()

                var data = parser.get_dfa()

                for (var i in data)
                    listmodel_line_data.append({"model_data": data[i]})
            }
        }
    }
}

