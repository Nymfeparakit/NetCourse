#include "logindialog.h"

LoginDialog::LoginDialog(Client *_client, QWidget *parent)
{
    client = _client;

    nameFld = new QLineEdit(this);
    QLabel *nameLbl = new QLabel("&Имя:", this);
    nameLbl->setBuddy(nameFld);
    ipFld = new QLineEdit(this);
    ipFld->setText("localhost");
    QLabel *ipLbl = new QLabel("&ip:", this);
    ipLbl->setBuddy(ipFld);
    portFld = new QLineEdit(this);
    portFld->setText("2424");
    //допускаем ввод только чисел в диапазоне
    //portFld->setValidator( new QIntValidator(1024, 40000, this) );
    QLabel *portLbl = new QLabel("&Порт:", this);
    portLbl->setBuddy(portFld);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    //QPushButton *okButton = new QPushButton("Войти");

    //connect(okButton, &QPushButton::clicked,
            //this, &QDialog::accept);
            //this, &LoginDialog::acceptButtonClicked);

    connect(buttonBox, &QDialogButtonBox::accepted,
            this, &LoginDialog::acceptButtonClicked);

    //Layout
    QVBoxLayout* vBoxLayout = new QVBoxLayout;
    vBoxLayout->addWidget(nameLbl);
    vBoxLayout->addWidget(nameFld);
    vBoxLayout->addWidget(ipLbl);
    vBoxLayout->addWidget(ipFld);
    vBoxLayout->addWidget(portLbl);
    vBoxLayout->addWidget(portFld);
    errorLbl = new QLabel();
    vBoxLayout->addWidget(errorLbl);
    vBoxLayout->addWidget(buttonBox);
    //vBoxLayout->addWidget(okButton);
    setLayout(vBoxLayout);
    setFixedSize(500, 300); //устанавливаем фиксированный размер окна

}

void LoginDialog::acceptButtonClicked()
{
    name = nameFld->text();
    ip = ipFld->text();
    QIntValidator v(1024, 40000, this);
    int pos = 0;
    QString portStr = portFld->text();
    if (name.isEmpty() || ip.isEmpty() || portStr.isEmpty()) {
        errorLbl->setText("Должны быть заполнены все поля!");
        return;
    }
    if (v.validate(portStr, pos) != QIntValidator::Acceptable) {
        errorLbl->setText("В поле для ввода номера порта допускаются только цифры!");
        return;
    }
    port = portStr.toInt();
    QString errorMsg = "";

    connect(client, &Client::connectionFailed,
            [this] () {
        this->errorLbl->setText("Не удалось установить соединение");
        this->nameFld->setReadOnly(false);
        this->ipFld->setReadOnly(false);
        this->portFld->setReadOnly(false);
    });
    connect(client, &Client::connectionSucceeded,
            [this] () {
       emit accept();
    });
    client->connectToServer(ip, port, name, errorMsg);
    errorLbl->setText("Ожидание подключения...");
    nameFld->setReadOnly(true);
    ipFld->setReadOnly(true);
    portFld->setReadOnly(true);
    /*if (!client->connectToServer(ip, port, name, errorMsg)) {
        errorLbl->setText(errorMsg);
        return;
    }*/
}

QString LoginDialog::getName()
{
    return name;
}

QString LoginDialog::getIP()
{
    return ip;
}
int LoginDialog::getPort()
{
    return port;
}
