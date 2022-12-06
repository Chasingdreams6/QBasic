#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <exception>

extern std::map<std::string, int> intMap;
extern std::map<Op, int> prio;
extern std::set<int> lines;
extern std::map<int, WrappedStm> stms;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    prio[PLUS_OP] = prio[MINUS_OP] = 1;
    prio[MUL_OP] = prio[DIV_OP] = 2;
    prio[POW_OP] = 3;
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::arithToken(Token tk) {
    if (tk == PLUS_TK || tk == MINUS_TK || tk == POW_TK || tk == DIV_TK) return true;
    if (tk == POW_TK || tk == LPAREN_TK || tk == RPAREN_TK || tk == INT_TK) return true;
    if (tk == STR_TK) return true;
    return false;
}

Token MainWindow::getToken() {
    if (list[pos] == "") return NULL_TK;
    if (list[pos] == "REM") return REM_TK;
    if (list[pos] == "LET") return LET_TK;
    if (list[pos] == "PRINT") return PRINT_TK;
    if (list[pos] == "INPUT") return INPUT_TK;
    if (list[pos] == "GOTO") return GO_TK;
    if (list[pos] == "IF") return IF_TK;
    if (list[pos] == "END") return END_TK;
    if (list[pos] == "+") return PLUS_TK;
    if (list[pos] == "-") return MINUS_TK;
    if (list[pos] == "**") return POW_TK;
    if (list[pos] == "/") return DIV_TK;
    if (list[pos] == "*") return MUL_TK;
    if (list[pos] == "(") return LPAREN_TK;
    if (list[pos] == ")") return RPAREN_TK;
    if (list[pos] == "<") return LT_TK;
    if (list[pos] == "=") return EQ_TK;
    if (list[pos] == ">") return GT_TK;
    if (list[pos][0] < '0' || list[pos][0] > '9') return STR_TK;
    return INT_TK;
}

void MainWindow::advance() {
    pos++;
}

void MainWindow::run() {
    if (lines.empty()) return ;
    int line_no = *lines.begin();
    while (line_no > 0) {
        WrappedStm cur = stms[line_no];
        line_no = cur.stm_->exec(ui->textBrowser, ui->treeDisplay);
    }
}

bool MainWindow::parse(QString cmd) {
    bool ok = true;
    int line_no;
    Token curToken;
    pos = 0;
    cmd = cmd.trimmed();
    list = cmd.split(" ");
    if (list.empty()) {
        errorMsg("empty command");
        return false;
    }
    curToken = getToken();
    if (curToken != INT_TK) {
        errorMsg("Parse line number error");
        return false; // fail
    }
    line_no = list[pos].toInt(&ok, 10);
    advance();
    if (stms.count(line_no)) {
        errorMsg("The line number had existed");
        return false;
    }
    lines.insert(line_no);
    WrappedStm cur(line_no, stm(line_no), cmd.toStdString());
    stms[line_no] = cur;
    return true;
}

Stm* MainWindow::stm(int line_no) {
    Token curToken = getToken(); advance();
    Token op_tk;
    std::string sym;
    switch (curToken) {
    case NULL_TK:
        break;
    case LET_TK:
        sym = list[pos].toStdString();
        advance();
        op_tk = getToken(); advance();
        if (op_tk != EQ_TK) {
            errorMsg("Invalid let stm");
            return nullptr;
        }
        return new LetStm(line_no, sym, exp());
    case PRINT_TK:
        return new PrintStm(line_no, exp());
    default:
        errorMsg("Unsupported statement now");
        return nullptr;
    }
}


Op MainWindow::token2Op(Token tk) {
    switch (tk) {
    case PLUS_TK:
        return PLUS_OP;
    case MINUS_TK:
        return MINUS_OP;
    case MUL_TK:
        return MUL_OP;
    case DIV_TK:
        return DIV_OP;
    case POW_TK:
        return POW_OP;
    default:
        errorMsg("unsupport arithmetic symbol");
        return PLUS_OP;
    }
}
void debug(std::queue<Poi> outQueue) {
//    while (!outQueue.empty()) {
//        Poi cur = outQueue.front();
//        switch (cur.kind_) {
//            case 1:

//        }
//    }
}
Exp* MainWindow::exp() {
    // 首先把中缀转后缀
    Token curToken;
    std::stack<Token> opStack;
    std::queue<Poi> outQueue;

    while(pos < list.size() && arithToken(getToken())) {
        curToken = getToken();
        if (curToken == INT_TK) {
            outQueue.push(Poi(2, PLUS_OP, list[pos].toInt(nullptr, 10), ""));
        }
        if (curToken == STR_TK) { // treat as int
            outQueue.push(Poi(3, PLUS_OP, 0, list[pos].toStdString()));
        }
        if (curToken == LPAREN_TK) {
            opStack.push(LPAREN_TK);
        }
        if (curToken == RPAREN_TK) {
            while (!opStack.empty() && opStack.top() != LPAREN_TK) {
                outQueue.push(Poi(1, token2Op(curToken), 0, ""));
                opStack.pop();
            }
            if (opStack.empty()) {
                errorMsg("unMatched ()");
                return nullptr;
            }
            opStack.pop();
        }
        if (curToken == PLUS_TK || curToken == MINUS_TK
                || curToken == MUL_TK || curToken == DIV_TK) {
            while (!opStack.empty() &&
                   prio[token2Op(curToken)] <= prio[token2Op(opStack.top())]) { // left-asso
                outQueue.push(Poi(1, token2Op(opStack.top()), 0, ""));
                opStack.pop();
            }
            opStack.push(curToken);
        }
        if (curToken == POW_TK) {
            while (!opStack.empty() &&
                   prio[token2Op(curToken)] < prio[token2Op(opStack.top())]) { // right-asso
                outQueue.push(Poi(1, token2Op(opStack.top()), 0, ""));
                opStack.pop();
            }
            opStack.push(curToken);
        }
        advance();
    }
    while (!opStack.empty()) {
        outQueue.push(Poi(1, token2Op(opStack.top()), 0, ""));
        opStack.pop();
    }
    return build_exp(outQueue);
}

void MainWindow::errorMsg(std::string error) {
    ui->textBrowser->append(QString(error.c_str()));
}

Exp* MainWindow::construct_exp(Poi e) {
    if (e.kind_ == 2) { // int
        return new ConstExp(e.val_);
    }
    else {
        return new IdExp(e.sym_);
    }
}

Exp* MainWindow::build_exp(std::queue<Poi> outQueue) {
    std::stack<Exp*> stack;
    try {
        while (!outQueue.empty()) {
            Poi cur = outQueue.front(); outQueue.pop();
            if (cur.kind_ != 1) { // var or cons
                stack.push(construct_exp(cur));
                continue;
            }
            Exp* right = stack.top(); stack.pop();
            Exp* left = stack.top(); stack.pop();
            stack.push(new CompoundExp(left, right, cur.op_));
        }
    } catch(std::exception &e) {
        errorMsg("Parse Exp error");
    }
    if (stack.size() != 1) {
        errorMsg("Parse Exp error");
        return nullptr;
    }
    return stack.top();
}

void MainWindow::on_cmdLineEdit_editingFinished()
{
    QString cmd = ui->cmdLineEdit->text();
    ui->cmdLineEdit->setText("");

    parse(cmd);
    ui->CodeDisplay->append(cmd);

}

void MainWindow::on_btnClearCode_clicked()
{
    ui->CodeDisplay->clear();
    ui->textBrowser->clear();
    intMap.clear();
    if (lines.empty()) return ;
    while (!lines.empty()) {
        int cur = *lines.begin();
        lines.erase(cur);
        WrappedStm cur_stm = stms[cur];
        cur_stm.stm_->free();
    }
    lines.clear();
    stms.clear();
}

void MainWindow::on_btnLoadCode_clicked()
{
    on_btnClearCode_clicked();

    QString path = QFileDialog::getOpenFileName(this, "载入", "E:\\sep_new_2022\\QBasic-handout\\");
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return ;
    while (!file.atEnd()) {
        QByteArray array = file.readLine();
        QString str = QString(array);
        parse(str);
        ui->CodeDisplay->append(str);
    }
}

void MainWindow::on_btnRunCode_clicked()
{
    ui->textBrowser->clear();
    run();
}
