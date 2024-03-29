#include "client.h"


//Client::Client(QString host, int port, QString name, QWidget* pwgt)
Client::Client(QWidget* pwgt)
    :QWidget (pwgt)
{
    //QString name2 = name;
    socket = new QTcpSocket(this);
    msgField = new QLineEdit;
    //sendToServer();

    //когда появились новые данные читаем их от сервера
    connect(socket, &QTcpSocket::readyRead,
            this, &Client::readFromServer);
    dialog = new QTextEdit;
    dialog->setReadOnly(true);//поле только для чтения

    QPushButton* sendBtn = new QPushButton("&Send");
    connect(sendBtn, &QPushButton::clicked,
            this, &Client::sendMsgToServer);//по нажатию на кнопку отправляем сообщение на сервер
    //usersNamesList = new QListWidget();

    //model = new QStringListModel(usersNamesList, this);
    /*model = new UsersNamesListModel(this);
    usersNamesListView = new QListView(this);
    usersNamesListView->setModel(model);*/

    usersNamesListWidget = new QListWidget(this);
    //Layout
    QHBoxLayout* hBoxLayout = new QHBoxLayout;
    hBoxLayout->addWidget(dialog, 2);
    //hBoxLayout->addWidget(usersNamesListView);
    hBoxLayout->addWidget(usersNamesListWidget, 1);
    QVBoxLayout* vBoxLayout = new QVBoxLayout;
    vBoxLayout->addLayout(hBoxLayout);
    vBoxLayout->addWidget(msgField);
    vBoxLayout->addWidget(sendBtn);
    setLayout(vBoxLayout);

}

void Client::setErrorMsg(QAbstractSocket::SocketError er)
{
    /*errorMsg = "Не удалось подключиться к серверу";
    isConnectedSuccessfully = false;*/
    emit connectionFailed();
}

bool Client::connectToServer(QString host, int port, QString _name, QString &error)
{
    name = _name;
    isConnectedSuccessfully = true;
    /*connect(socket, &QAbstractSocket::error,
            [this] () {

    });*/
    //c новым синтаксисом не работает
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
    this, SLOT(setErrorMsg(QAbstractSocket::SocketError)));
    connect( socket, SIGNAL(connected()), this, SLOT(sendNameToServer()) );
    socket->connectToHost(host, port);
    return true;
}

void Client::sendNameToServer()
{
    QByteArray nameToSend = name.toUtf8();

    QByteArray block;
    //Запаковываем сюда байт, который определяет, имя это или сообщение
    QDataStream out(&block, QIODevice::ReadWrite);
    out << quint16(0) << quint8(0) << nameToSend;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    socket->write(block); //отправляем серверу свое имя

    emit connectionSucceeded();
}

void Client::sendMsgToServer()
{

    QByteArray msgToSend = msgField->text().toUtf8();//Получаем введенное сообщение
    QByteArray block;
    //Запаковываем сюда байт, который определяет, имя это или сообщение
    QDataStream out(&block, QIODevice::ReadWrite);
    out << quint16(0) << quint8(1) << msgToSend;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));

    socket->write(block);//отправляем его серверу
    //socket->write(msgToSend);//отправляем его серверу
    socket->flush();
    msgField->setText("");//очищаем поле ввода
}

void Client::readFromServer()
{
    QDataStream in(socket);
    QTime sendingTime;
    QString senderName;
    QByteArray msg;
    QByteArray newUserName;
    QString banMsg;
    nextBlockSize = 0;
    quint8 cntrlByte = 0;
    bool newUserConnected = false;
    forever {
        if (!nextBlockSize) {
            if (socket->bytesAvailable() < sizeof(quint16)) {
                break;
            }
            in >> nextBlockSize;
        }

        if (socket->bytesAvailable() < nextBlockSize) {
            break;
        }
        in >> cntrlByte;
        if (cntrlByte == 0) { //если получено просто имя участника чата
            in >> newUserName;
            int index = 0;
            if ((index = usersNamesList.indexOf(newUserName)) == -1) { //если такого в списке нет
                usersNamesList << newUserName;
                newUserConnected = true;
                /*
                int rowCount = model->rowCount();
                //model->insertRows(rowCount, 1);
                rowCount = model->rowCount();
                //model->setData(createIndex(rowCount, 0), newUserName);
                model->setData(model->index(model->rowCount()), newUserName);
                rowCount = model->rowCount();
                //model->setData(model->index(model->rowCount()), newUserName);
                */
                QListWidgetItem *item = new QListWidgetItem(newUserName, usersNamesListWidget);
                //usersNamesList.push_back(newUserName);
                //usersNamesListView->setModel(model);
            } else {
                /*QList<QListWidgetItem *> items = usersNamesListWidget->findItems(newUserName, Qt::MatchExactly);
                for (int j = 0; j < items.size(); ++j) {
                    usersNamesListWidget->removeItemWidget(items.at(j));
                }*/
                //model->removeRows(index, 1);
                usersNamesListWidget->takeItem(index);
                usersNamesList.removeAt(index);
            }
        } else if (cntrlByte == 1) { //если получено сообщение
            in >> msg >> sendingTime >> senderName;
        } else if (cntrlByte == 2) { //если получены имена всех текущих клиентов
            quint8 numberOfCLients;//узнаем количество клиентов
            in >> numberOfCLients;
            for (int i = 0; i < numberOfCLients; ++i) {
                QString userName;
                in >> userName;
                QListWidgetItem *item = new QListWidgetItem(userName, usersNamesListWidget);
                //model->insertRows(model->rowCount(), 1);
                //model->setData(model->index(model->rowCount()), userName);
                usersNamesList.push_back(userName);
            }
        } else if (cntrlByte == 3) { //Если получено сообщение о бане
            in >> banMsg;
            dialog->setText(banMsg);
            socket->disconnectFromHost();//сразу отсоединяем
        }
        nextBlockSize = 0;
    }


    //QByteArray msg(socket->readAll());//читаем
    //dialog->setText(msg);//отображаем в поле диалога
    if (cntrlByte == 1) {
        dialog->append(msg);
        dialog->append(senderName + " " + sendingTime.toString("hh:mm"));
    } else if (cntrlByte == 0) {
        QString userConnectionStatus;
        if (newUserConnected) {
            userConnectionStatus = "Connected";
        } else {
            userConnectionStatus = "Disconnected";
        }
        dialog->append(userConnectionStatus + " " + newUserName);
    } /*else if (cntrlByte == 3) {
        dialog->setText(banMsg);//просто печатаем сообщение бана
    }*/
    dialog->append("");
}
