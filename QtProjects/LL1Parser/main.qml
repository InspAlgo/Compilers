import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2
import LL1ParsingToQML 1.0
import TableModel 1.0

Window {
    visible: true
    width: 720
    height: 480
    title: qsTr("LL(1) Parser")

    Image {
        id: background_image
        anchors.fill: parent
        fillMode: Image.Stretch
        smooth: true
        source: "qrc:/img/m.jpg"
    }

    LL1ParsingToQML {
        id: parser
    }

    Row {
        spacing: 0
        anchors.fill: parent

        Column {
            width: parent.width/2
            height: parent.height
            spacing: 0

            Rectangle {
                id: rect_listview
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.topMargin: 10
                anchors.leftMargin: 10

                width: parent.width - 20
                height: parent.height/3*2 -20

                color: "#3edcdcdc"
                radius: 10

                ListView {
                    clip: true
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: parent.height - 20
                    model: ListModel {
                        id:list_model
                    }
                    delegate: Item {
                        width: parent.width
                        height: 35
                        ScrollView {
                            anchors.fill: parent
                            TextArea {
                                text: production_left + " -> " + production_right_str
                                font.pixelSize: 20
                                readOnly: true
                            }
                        }
                    }
                    ScrollBar.vertical: ScrollBar{
                        width: 10
                        policy: ScrollBar.AlwaysOn
                    }
                }
            }

            Row {
                id:row_set_start_token
                anchors.bottom: select_row.top
                anchors.left: parent.left
                anchors.bottomMargin: 10
                anchors.leftMargin: 10

                width: parent.width - 20
                height: parent.height/6 - 20

                // 设置起始 Token 的输入框
                Rectangle {
                    anchors.left: parent.left
                    width: parent.width/3*2-5
                    height: parent.height
                    color: "#3effffe0"
                    radius: 10

                    ScrollView {
                        anchors.fill: parent
                        TextArea {
                            id:start_token_text
                            placeholderText:qsTr("请输入起始 Token")
                            font.pixelSize: 20
                            horizontalAlignment: TextArea.AlignHCenter
                            verticalAlignment: TextArea.AlignVCenter
                        }
                    }
                }
            }

            Row {
                id:select_row
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.bottomMargin: 20
                anchors.leftMargin: 10

                width: parent.width - 20
                height: parent.height/6 - 20

                spacing: 15

                Rectangle {
                    width: parent.width/3 - 10
                    height: parent.height
                    color: "#6e66cdaa"
                    radius: 10
                    MouseArea {
                        anchors.fill: parent
                        Text {
                            anchors.centerIn: parent
                            text: qsTr("清空")
                        }
                        onClicked: {
                            start_token_text.clear()
                            list_model.clear()
                            parser.clear()
                            clearResult()
                        }
                    }
                }

                Rectangle {
                    width: parent.width/3 - 10
                    height: parent.height
                    color: "#6e66cdaa"
                    radius: 10
                    MouseArea {
                        anchors.fill: parent
                        Text {
                            anchors.centerIn: parent
                            text: qsTr("添加产生式")
                        }
                        onClicked: {
                            add_production_win.inputClear()
                            add_production_win.open()
                        }
                    }
                }

                Rectangle {
                    width: parent.width/3 - 10
                    height: parent.height
                    color: "#6e66cdaa"
                    radius: 10

                    TableModel {
                        id: table_model
                    }

                    MouseArea {
                        anchors.fill: parent
                        Text {
                            anchors.centerIn: parent
                            text: qsTr("LL(1)分析")
                        }
                        onClicked: {
                            if(start_token_text.length < 1){
                                showCriticalMessage(qsTr("起始 Token 为空！"))
                                return
                            }
                            if(list_model.count <= 0){
                                showCriticalMessage(qsTr("未添加产生式！"))
                                return
                            }
                            clearResult()  // 清空已显示
                            parser.set_start_token(start_token_text.text.toString())
                            parser.test()
                            textarea_first.append(parser.get_first_s_set_str())
                            textarea_follow.append(parser.get_follow_set_str())

                            // 加载分析表的行头和列头
                            var nonterminal = parser.get_nonterminal()
                            var terminal = parser.get_terminal()
                            var columns_header = []
                            var rows_header = []
                            for(var i=0;i<nonterminal.length;i++)
                                rows_header.push(nonterminal[i])
                            for(var j=0; j<terminal.length;j++)
                                columns_header.push(terminal[j])
                            columns_header_model.model = columns_header
                            rows_header_model.model = rows_header

                            //                            console.log(parser.get_parsing_table_str())
                            var table = parser.get_table()
                            table_model.set_header(nonterminal, terminal)
                            table_model.set_table(table)
                            tableView.model = table_model
                            tableView.visible = true
                        }
                    }
                }
            }
        }

        Column {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 10
            anchors.rightMargin: 10
            anchors.bottomMargin: 10
            width: parent.width/2 -10
            height: parent.height - 20
            spacing: 10

            Rectangle {
                width: parent.width
                height: parent.height/5 - 10
                color:"#3edcdcdc"
                radius: 10
                Flickable {
                    anchors.fill: parent

                    TextArea.flickable: TextArea {
                        id: textarea_first
                        font.pixelSize: 20
                        readOnly: true
                    }

                    ScrollBar.vertical: ScrollBar{
                        width: 10
                        policy: ScrollBar.AlwaysOn
                    }
                }
                Text {
                    anchors.fill: parent
                    text: qsTr("FISRT SET")
                    color: "#6e778899"
                    font.pixelSize: 30
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Rectangle {
                width: parent.width
                height: parent.height/5 -10
                color:"#3edcdcdc"
                radius: 10
                Flickable {
                    anchors.fill: parent
                    TextArea.flickable: TextArea {
                        id: textarea_follow
                        font.pixelSize: 20
                        readOnly: true
                    }
                    ScrollBar.vertical: ScrollBar{
                        width: 10
                        policy: ScrollBar.AlwaysOn
                    }
                }
                Text {
                    anchors.fill: parent
                    text: qsTr("FOLLOW SET")
                    color: "#6e778899"
                    font.pixelSize: 30
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Rectangle {
                width: parent.width
                height: parent.height/5*3 -10
                color:"#3edcdcdc"
                radius: 10

                TableView {
                    id: tableView
                    visible: false

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


                Text {
                    anchors.fill: parent
                    text: qsTr("LL(1) PARSING TABLE")
                    color: "#aa778899"
                    font.pixelSize: 30
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }

    Dialog {
        id:add_production_win
        title: qsTr("添加产生式")
        width: 500
        height: 100
        visible: false
        standardButtons: StandardButton.Save | StandardButton.Cancel

        onAccepted: {
            // 产生式左部和右部不能为空
            if (input_left.length <= 0 || input_listview.count <= 0)
            {
                inputClear()
                return
            }

            var p_left = input_left.text.toString()
            var p_right = input_listview.getAllText()
            var p_right_str=""

            for(var i=0;i<p_right.length;i++){
                if(i!==0&&p_right[i].length===0&&i===p_right.length-1)
                    continue
                parser.add_token(p_right[i])
                p_right_str += (p_right[i].length < 1? "ε " : p_right[i] + " ")
            }

            parser.add_token_left(p_left)
            if(parser.add_production())
                list_model.append({"production_left":p_left,"production_right_str":p_right_str})

            inputClear()
        }
        onRejected: {
            inputClear()
        }

        Row {
            anchors.fill: parent

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
        }

        function inputClear(){
            input_left.clear()
            input_right_model.clear()
            add_production_win.width = 500
            add_production_win.height = 100
        }
    }

    MessageDialog {
        id: message_critical
        visible: false
        title: qsTr("错误")
        icon: StandardIcon.Critical
        standardButtons: StandardButton.OK
    }

    function showCriticalMessage(args) {
        message_critical.text = qsTr(args)
        message_critical.open()
    }

    function clearResult() {
        textarea_first.clear()
        textarea_follow.clear()
        tableView.model = null
        tableView.visible = false
        table_model.clear()
    }
}
