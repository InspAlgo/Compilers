import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

import LR0Parser 1.0
import TableModel 1.0

Window {
    visible: true
    width: 1080
    height: 640
    title: qsTr("LR(0) Parser")

    Image {
        id: background_image
        anchors.fill: parent
        fillMode: Image.Stretch
        smooth: true
        source: "qrc:/img/m.jpg"
    }

    LR0Parser {
        id: parser
    }

    // 数据模型
    ListModel {
        id: listmodel_grammar
    }

    ListModel {
        id: listmodel_grammar_expanding
    }

    ListModel {
        id: listmodel_states
    }

    ListModel {
        id: listmodel_table_colheader
    }

    Rectangle {
        // 显示输入的文法的区域
        id: grammar_rect

        width: parent.width/4*3 - 20
        height: parent.height - 20

        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10

        radius: 10
        color: "#3edcdcdc"

        ListView {
            id: grammar_listview
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.topMargin: 10
            anchors.bottomMargin: 10
            width: parent.width
            height: parent.height - 20
            clip: true
            model: listmodel_grammar
            delegate: Item {
                width: parent.width
                height: 35
                ScrollView {
                    anchors.fill: parent
                    TextArea {
                        text: model_data
                        font.pixelSize: 20
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

    Rectangle {
        // 控制区
        id: control_rect

        width: parent.width/4 - 20
        height: parent.height - 20

        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10

        radius: 10
        color: "#3edcdcdc"

        Column {
            spacing: 10
            anchors.fill: parent

            Rectangle {
                width: parent.width
                height: 60
                radius: 10
                color: "#6e66cdaa"

                ScrollView {
                    anchors.fill: parent
                    TextArea {
                        id: start_token_text
                        placeholderText:qsTr("请输入起始 Token")
                        horizontalAlignment: TextArea.AlignHCenter
                        verticalAlignment: TextArea.AlignVCenter
                    }
                }
            }  // Rectangle

            Rectangle {
                width: parent.width
                height: 60
                radius: 10
                color: "#6e66cdaa"

                MouseArea {
                    id: control_clear
                    anchors.fill: parent
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("清空")
                    }
                    onClicked: {
                        start_token_text.clear()
                        start_token_text.focus = false
                        clearListModel()
                        parser.clear()
                    }
                }
            }  // Rectangle

            Rectangle {
                width: parent.width
                height: 60
                radius: 10
                color: "#6e66cdaa"

                MouseArea {
                    id: control_add_prodction
                    anchors.fill: parent
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("添加产生式")
                    }
                    onClicked: {
                        start_token_text.focus = false
                        grammar_listview.model = listmodel_grammar
                        dialog_add_production.showDialog()
                    }
                }
            }  // Rectangle

            Rectangle {
                width: parent.width
                height: 60
                radius: 10
                color: "#6e66cdaa"

                MouseArea {
                    id: control_run_parsing
                    anchors.fill: parent
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("生成LR(0)分析表")
                    }
                    onClicked: {
                        start_token_text.focus = false
                        if(start_token_text.length < 1){
                            showCriticalMessage(qsTr("起始 Token 为空！"))
                            return
                        }

                        if(listmodel_grammar.count <= 0){
                            showCriticalMessage(qsTr("未添加产生式！"))
                            return
                        }

                        parser.set_start_token(start_token_text.text.toString())
                        parser.running()

                        getParsingTableData()

                        showNormalMessage(qsTr("已生成LR(0)分析表"))
                    }
                }
            }  // Rectangle

            Rectangle {
                width: parent.width
                height: 60
                radius: 10
                color: "#6e66cdaa"

                MouseArea {
                    id: control_show_grammar_expanding
                    anchors.fill: parent
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("显示拓展文法")
                    }
                    onClicked: {
                        start_token_text.focus = false
                        if(grammar_listview.model === listmodel_grammar_expanding)
                            grammar_listview.model = listmodel_grammar
                        else
                            grammar_listview.model = listmodel_grammar_expanding
                    }
                }
            }  // Rectangle

            Rectangle {
                width: parent.width
                height: 60
                radius: 10
                color: "#6e66cdaa"

                MouseArea {
                    id: control_show_states
                    anchors.fill: parent
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("显示项目集")
                    }
                    onClicked: {
                        start_token_text.focus = false
                        if(grammar_listview.model === listmodel_states)
                            grammar_listview.model = listmodel_grammar
                        else
                            grammar_listview.model = listmodel_states
                    }
                }
            }  // Rectangle

            Rectangle {
                width: parent.width
                height: 60
                radius: 10
                color: "#6e66cdaa"

                MouseArea {
                    id: control_show_parsing_table
                    anchors.fill: parent
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("显示分析表")
                    }
                    onClicked: {
                        start_token_text.focus = false
                        dialog_show_parsingtable.showTable()
                    }
                }
            }  // Rectangle

            Rectangle {
                width: parent.width
                height: 60
                radius: 10
                color: "#6e66cdaa"

                MouseArea {
                    id: control_show_tokens_parsing
                    anchors.fill: parent
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("符号分析")
                    }
                    onClicked: {
                        start_token_text.focus = false
                        dialog_tokens_parsing.showTokensParsing()
                    }
                }
            }  // Rectangle
        }
    }

    // 添加产生式窗口
    Dialog {
        id: dialog_add_production
        title: qsTr("添加产生式")
        visible: false
        width: 500
        height: 100

        standardButtons: StandardButton.Save | StandardButton.Cancel

        function showDialog() {
            dialog_add_production.inputClear()
            dialog_add_production.open()
        }

        onAccepted: {
            // 产生式左部和右部不能为空
            if (input_left.length <= 0 || input_listview.count <= 0)
            {
                inputClear()
                return
            }

            var p_right = input_listview.getAllText()
            var production = input_left.text.toString() + " -> ";

            if(p_right.length > 1 && p_right[p_right.length - 1].length < 1)
                p_right.pop()

            for(var i = 0; i< p_right.length; i++)
                production += (p_right[i].length < 1? "ε" : p_right[i] + " ")

            if(parser.add_production(input_left.text.toString(), p_right))
                listmodel_grammar.append({"model_data": production})

            grammar_listview.model = listmodel_grammar
            dialog_add_production.close()
            inputClear()
        }
        onRejected: {
            dialog_add_production.close()
            inputClear()
        }

        TextField {
            id: input_left
            width: 60
            height: 40
            placeholderText: qsTr("")
            horizontalAlignment: TextField.AlignHCenter
            verticalAlignment: TextField.AlignVCenter
        }

        Label {
            id: input_arrow
            width: 20
            height: 40
            anchors.left: input_left.right
            anchors.leftMargin: 5
            text: qsTr("->")
            horizontalAlignment: Label.AlignHCenter
            verticalAlignment: Label.AlignVCenter
        }

        ListModel {
            id: input_right_model
        }

        ListView {
            id: input_listview
            spacing: 5
            width: 150
            height: 40
            anchors.left: input_arrow.right
            anchors.leftMargin: 20
            orientation: ListView.Horizontal
            model: input_right_model
            delegate: TextField {
                width: 60
                text: token
                horizontalAlignment: TextField.AlignHCenter
                verticalAlignment: TextField.AlignVCenter

                function getText(){
                    return text
                }
            }

            function getAllText(){
                var all_text = []
                var children = input_listview.contentItem.children
                for(var i=0;i !== children.length;i++){
                    if (typeof children[i].getText !== "function") {
                        // 或者 === "undefined"
                        continue
                    }
                    all_text.push(children[i].getText())
                }

                return all_text
            }
        }

        Button {
            id: input_button_del
            text: qsTr("Del")
            width: 80
            height: 40
            anchors.right: input_button_add.left
            anchors.rightMargin: 5
            onClicked: {
                if(input_right_model.count > 0)
                    input_right_model.remove(input_right_model.count-1)
            }
        }

        Button {
            id: input_button_add
            text: qsTr("Add")
            width: 80
            height: 40
            anchors.right: parent.right
            onClicked: {
                if(input_right_model.count <= 0)
                    input_right_model.append({"token":""})
                else {
                    var str_list = input_listview.getAllText()
                    if(str_list[str_list.length-1].length > 0)
                        input_right_model.append({"token":""})
                }

            }
        }


        function inputClear(){
            input_left.clear()
            input_right_model.clear()
            dialog_add_production.width = 500
            dialog_add_production.height = 100
        }
    }

    // 显示分析表窗口
    Dialog {
        id: dialog_show_parsingtable
        title: qsTr("LR(0)分析表")
        visible: false
        width: 640
        height: 480

        TableModel {
            id: tablemodel_parsingtable
        }

        Rectangle {
            anchors.fill: parent
            color: "#3edcdcdc"
            radius: 10

            TableView {
                id: tableView

                columnWidthProvider: function (column) { return 100; }
                rowHeightProvider: function (column) { return 60; }
                anchors.fill: parent
                leftMargin: rowsHeader.implicitWidth
                topMargin: columnsHeader.implicitHeight

                ScrollBar.horizontal: ScrollBar{
                    id: table_view_scrollbar_horizontal
                    height: 10
                    policy: ScrollBar.AlwaysOn
                }
                ScrollBar.vertical: ScrollBar{
                    id: table_view_scrollbar_vertical
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
                    y: tableView.contentY
                    x: tableView.contentX
                    width: tableView.leftMargin
                    height: tableView.topMargin
                }

                Row {
                    id: columnsHeader
                    y: tableView.contentY
                    z: 2
                    Repeater {
                        id: columns_header_model
                        model: []
                        Label {
                            width: tableView.columnWidthProvider(modelData)
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
                    id: rowsHeader
                    x: tableView.contentX
                    z: 2
                    Repeater {
                        id: rows_header_model
                        model: []
                        Label {
                            width: 60
                            height: tableView.rowHeightProvider(modelData)
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

        function showTable() {
            tableView.visible = true
            tableView.model = tablemodel_parsingtable
            dialog_show_parsingtable.open()
        }
    }

    Dialog {
        id: dialog_tokens_parsing
        title: qsTr("符号分析")
        visible: false
        width: 640
        height: 480

        Rectangle {
            anchors.fill: parent
            color: "#00123456"
            radius: 10

            // 输入待分析的符号串以及开始分析按钮
            Rectangle {
                width: parent.width
                height: 60
                anchors.top: parent.top
                color: "#00123456"
                radius: 10

                TextField {
                    id: input_tokens
                    width: parent.width/4*2 - 30
                    height: parent.height
                    anchors.left: parent.left
                    verticalAlignment: TextField.AlignVCenter
                    horizontalAlignment: TextField.AlignHCenter
                    placeholderText:qsTr("请输入待分析的符号串")
                    font.pixelSize: 20

                }
                Rectangle {
                    width: parent.width/4-20
                    height: parent.height
                    radius: 10
                    color: "#6e66cdaa"
                    anchors.right: parent.right

                    MouseArea {
                        id: run_input_tokens_parsing
                        anchors.fill: parent
                        Text {
                            anchors.centerIn: parent
                            text: qsTr("开始分析")
                        }
                        onClicked: {
                            dialog_tokens_parsing.startRunParsing()
                        }
                    }
                }  // Rectangle
            }

            Rectangle {
                width: parent.width
                height: 60
                anchors.top: parent.top
                anchors.topMargin: 70
                color: "#00123456"

                Label {
                    text: qsTr("每步间隔时间(s)")
                    width: parent.width/4-20
                    height: parent.height
                    anchors.left: parent.left
                    verticalAlignment: Label.AlignVCenter
                    horizontalAlignment: Label.AlignHCenter
                }

                TextField {
                    id: show_time
                    width: parent.width/4-20
                    height: parent.height
                    verticalAlignment: TextField.AlignVCenter
                    horizontalAlignment: TextField.AlignHCenter
                    anchors.left: parent.left
                    anchors.leftMargin: parent.width/4-10
                    text: qsTr("3")
                    font.pixelSize: 20
                }

                Rectangle {
                    width: parent.width/4-20
                    height: parent.height
                    radius: 10
                    color: "#6e66cdaa"
                    anchors.right: parent.right
                    anchors.rightMargin: parent.width/4-10

                    MouseArea {
                        anchors.fill: parent
                        Text {
                            anchors.centerIn: parent
                            text: qsTr("设置时间")
                        }
                        onClicked: {
                            dialog_tokens_parsing.setTime(show_time.text)
                        }
                    }
                }  // Rectangle

                Rectangle {
                    width: parent.width/4-20
                    height: parent.height
                    radius: 10
                    color: "#6e66cdaa"
                    anchors.right: parent.right

                    MouseArea {
                        anchors.fill: parent
                        Text {
                            anchors.centerIn: parent
                            text: qsTr("暂停/继续")
                        }
                        onClicked: {
                            if(auto_show_timer.running) {
                                auto_show_timer.stop()
                                showNormalMessage("已暂停")
                            }

                            else {
                                showNormalMessage("演示继续")
                                auto_show_timer.start()
                            }
                        }
                    }
                }  // Rectangle
            }

            Rectangle {
                width: parent.width
                height: parent.height - 160
                anchors.bottom: parent.bottom

                ScrollView {
                    anchors.fill: parent
                    TextArea {
                        id: textarea_parsing_result
                        font.pixelSize: 20
                        readOnly: true
                    }
                }
            }
        }

        function showTokensParsing() {
            input_tokens.clear()
            show_time.clear()
            show_time.text = qsTr("3")
            textarea_parsing_result.clear()

            dialog_tokens_parsing.open()
        }

        function closeTokensParsing() {
            auto_show_timer.stop()
        }

        onDiscard: {
            closeTokensParsing()
        }
        onAccepted: {
            closeTokensParsing()
        }

        Timer {
            id: auto_show_timer
            interval: 3000; running: false; repeat: true;
            onTriggered: {
                dialog_tokens_parsing.runCurStep()
            }
        }

        function setTime(time) {
            auto_show_timer.interval = 1000*parseFloat(time)
            showNormalMessage("演示间隔："+time+"s")
        }

        function runCurStep() {
            var re = parser.get_step_data()

            if(re.length <= 0) {
                auto_show_timer.stop()
                showNormalMessage("分析结束")
                return
            }

            textarea_parsing_result.text = re;
        }

        function startRunParsing() {
            parser.set_input_tokens(input_tokens.text.split(" "))
            parser.run_tokens_parsing()
            auto_show_timer.start()
        }
    }

    MessageDialog {
        id: message_critical
        visible: false
        title: qsTr("错误")
        icon: StandardIcon.Critical
    }

    MessageDialog {
        id: message_normal
        visible: false
        title: ("消息")
        icon: StandardIcon.NoIcon
    }

    function showCriticalMessage(args) {
        message_critical.text = qsTr(args)
        message_critical.open()
    }

    function showNormalMessage(args) {
        message_normal.text = qsTr(args)
        message_normal.open()
    }

    // 获取分析表数据 1.分析表 2.拓展文法 3.状态表 4.分析表列表头
    // 加载到相关数据模型中
    function getParsingTableData() {
        clearReDataModel()

        var grammar_expanding = parser.get_grammar_expanding()
        var table_colheader = parser.get_table_colheader()
        var table_rowheader = parser.get_table_rowheader()
        var states_list = parser.get_states()
        var parsing_table = parser.get_parsing_table()

        for(var i = 0, ilen = grammar_expanding.length; i < ilen; i++) {
            listmodel_grammar_expanding.append({"model_data": grammar_expanding[i]})
        }

        for(var j = 0, jlen = table_colheader.length; j < jlen; j++) {
            listmodel_table_colheader.append({"model_data": table_colheader[j]})
        }

        for(var k = 0, klen = states_list.length; k < klen; k++) {
            listmodel_states.append({"model_data": states_list[k]})
        }

        rows_header_model.model = table_rowheader
        columns_header_model.model = table_colheader
        tablemodel_parsingtable.set_table_data(table_rowheader, table_colheader, parsing_table)
    }

    // 删除所有数据模型的数据
    function clearListModel()
    {
        listmodel_grammar.clear()
        clearReDataModel()
    }

    // 删除返回的数据
    function clearReDataModel()
    {
        listmodel_grammar_expanding.clear()
        listmodel_states.clear()
        listmodel_table_colheader.clear()

        tableView.visible = false  // 关闭分析表视图
        tableView.model = null  // 断开分析表的数据模型
        tablemodel_parsingtable.clear()  // 分析表数据模型清空
    }
}
