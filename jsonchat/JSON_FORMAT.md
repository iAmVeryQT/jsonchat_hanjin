| Field | |
|---|---|
| type | chat |
| room | 123 |
| name | 아무개 |
| text | 안녕하세요 |
| subtype(optional) | 'fileshare' |
| file path(optional) | /aaa/a.txt |
| file size(optional) | 123 |


~~~
QString roomName = ui->roomName->text();
QString nickName = ui->nickName->toPlainText();
...
QString msg = "#json begin {";
msg += "'type': " + MSG_TYPE["CHAT"] + ",";
msg += "'room': '" + roomName + "',";
msg += "'name': '" + nickName + "',";
msg += "'text': '" + nickName + "님이";
msg += "(" + shortPath + "," + QString::number(fileSize) + "byte) 파일을 공유합니다." + "',";
msg += "'subtype': " + MSG_SUB_TYPE["FILE_SHARE"] + ",";
msg += "'filepath': '" + filePath + "',";
msg += "'filesize': '" + QString::number(fileSize) + "',";
msg += "} #json end";
~~~
