#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "absyn.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

friend class Stm;
friend class Exp;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_cmdLineEdit_editingFinished();

    void on_btnClearCode_clicked();
    Status parse(QString);
    //bool eat(enum Token);
    inline void advance();
    Stm* stm(int );
    Exp* exp();
    Exp* build_exp(std::queue<Poi>);
    Exp* construct_exp(Poi);
    Token getToken();
    Op token2Op(Token);
    Op cmpToken2Op(Token);
    bool arithToken(Token);
    void preExec(QString &);
    void errorMsg(std::string );
    void run();
    void shuffle();

    void on_btnLoadCode_clicked();

    void on_btnRunCode_clicked();

private:
    Ui::MainWindow *ui;
    QString list;
    int pos, value, bios;
    int lastLine;
};
#endif // MAINWINDOW_H
