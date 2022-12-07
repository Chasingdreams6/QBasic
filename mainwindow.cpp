#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <exception>

extern std::map<std::string, int> intMap;
extern std::map<Token, int> prio;
extern std::set<int> lines;
extern std::map<int, WrappedStm> stms;
extern std::map<Op, std::string> opC;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    prio[PLUS_TK] = prio[MINUS_TK] = 1;
    prio[MUL_TK] = prio[DIV_TK] = 2;
    prio[POW_TK] = 3;
    prio[LPAREN_TK] = 4;

    opC[PLUS_OP] = "+"; opC[MINUS_OP] = "-";
    opC[MUL_OP] = "*"; opC[DIV_OP] = "/";
    opC[POW_OP] = "**"; opC[EQ_OP] = "=";
    opC[LT_OP] = "<"; opC[GT_OP] = ">";
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::arithToken(Token tk) {
    if (tk == PLUS_TK || tk == MINUS_TK || tk == MUL_TK || tk == DIV_TK) return true;
    if (tk == POW_TK || tk == LPAREN_TK || tk == RPAREN_TK || tk == INT_TK) return true;
    if (tk == STR_TK) return true;
    return false;
}

bool isLegalAlpha0(QChar x) {
   return (x >= 'a' && x <= 'z')
           || (x >= 'A' && x <= 'Z')
           || (x == '_');
}
bool isLegalAlpha1(QChar x) {
   return isLegalAlpha0(x) || (x >= '0' && x <= '9');
}
Token MainWindow::getToken() {
    while (pos < list.size() && list[pos] == " ") {pos++;}
    if (pos >= list.size()) return NULL_TK;
    if (list[pos] == "") {bios = 0; return NULL_TK;}
    if (list.mid(pos, 3) == "REM") {bios = 3; return REM_TK;}
    if (list.mid(pos, 3) == "LET") {bios = 3; return LET_TK;}
    if (list.mid(pos, 5) == "PRINT") {bios = 5; return PRINT_TK;}
    if (list.mid(pos, 5) == "INPUT") {bios = 5; return INPUT_TK;}
    if (list.mid(pos, 4) == "GOTO") {bios = 4; return GO_TK;}
    if (list.mid(pos, 2) == "IF") {bios = 2; return IF_TK;}
    if (list.mid(pos, 4) == "THEN") {bios = 4; return THEN_TK;}
    if (list.mid(pos, 3) == "END") {bios = 3; return END_TK;}
    if (list[pos] == '+') {bios = 1; return PLUS_TK;}
    if (list[pos] == '-') {bios = 1; return MINUS_TK;}
    if (list.mid(pos, 2) == "**") {bios = 2; return POW_TK;}
    if (list[pos] == '/') {bios = 1; return DIV_TK;}
    if (list[pos] == '*') {bios = 1; return MUL_TK;}
    if (list[pos] == '(') {bios = 1; return LPAREN_TK;}
    if (list[pos] == ')') {bios = 1; return RPAREN_TK;}
    if (list[pos] == '<') {bios = 1; return LT_TK;}
    if (list[pos] == '=') {bios = 1; return EQ_TK;}
    if (list[pos] == '>') {bios = 1; return GT_TK;}
    if (isLegalAlpha0(list[pos])) {
        bios = 0;
        while (pos + bios < list.size()
               && isLegalAlpha1(list[pos + bios])) bios++;
        return STR_TK;
    }
    if (list[pos] >= '0' && list[pos] <= '9') {
        bios = 0;
        while (pos + bios < list.size()
               && list[pos + bios] >= '0'
               && list[pos + bios] <= '9') bios++;
        return INT_TK;
    }
    errorMsg("Unknown token");
    return NULL_TK;
}

// must after getToken
void MainWindow::advance() {
    pos += bios;
}

void MainWindow::run() {
    if (lines.empty()) return ;
    int line_no = *lines.begin();
    while (line_no > 0) {
        WrappedStm cur = stms[line_no];
        line_no = cur.stm_->exec(ui->textBrowser, ui->treeDisplay, 0);
    }
}

bool MainWindow::parse(QString cmd) {
    bool ok = true;
    int line_no;
    Token curToken;
    pos = 0;
    cmd = cmd.trimmed();
    list = cmd;
    if (!list.size()) {
        errorMsg("empty command");
        return false;
    }
    curToken = getToken();
    if (curToken != INT_TK) {
        errorMsg("Parse line number error");
        return false; // fail
    }
    QString number = list.mid(pos, bios);
    line_no = number.toInt(&ok, 10);
    if (ok == false) {
        errorMsg("Bad Int");
        return false;
    }
    advance();
    if (stms.count(line_no)) {
        errorMsg("The line number had existed");
        return false;
    }
    Stm* tmp = stm(line_no);
    if (tmp == nullptr) { // illegal stm, ignore it
        return false;
    }
    lines.insert(line_no);
    WrappedStm cur(line_no, tmp, cmd.toStdString());
    stms[line_no] = cur;
    return true;
}

Stm* MainWindow::stm(int line_no) {
    Token curToken = getToken(); advance();
    Token op_tk;
    Op cmp_op;
    std::string sym;
    int tmp;
    switch (curToken) {
    case NULL_TK:
        break;
    case REM_TK: {
        std::string str = list.mid(pos).toStdString();
        pos += str.size(); // to last
        return new RemStm(line_no, str);
    }
    case LET_TK: {
        op_tk = getToken();
        if (op_tk != STR_TK) {
            errorMsg("Bad Let Stm");
            return nullptr;
        }
        sym = list.mid(pos, bios).toStdString();
        advance();
        op_tk = getToken(); advance();
        if (op_tk != EQ_TK) {
            errorMsg("Invalid let stm");
            return nullptr;
        }
        return new LetStm(line_no, sym, exp());
    }
    case PRINT_TK:
        return new PrintStm(line_no, exp());
    case GO_TK: {
        op_tk = getToken();
        if (op_tk != INT_TK) {
            errorMsg("Bad Goto Stm");
            return nullptr;
        }
        tmp = list.mid(pos, bios).toInt(nullptr, 10);
        return new GotoStm(line_no, tmp);
    }
    case IF_TK: {
        Exp* left = exp();
        op_tk = getToken();
        if (op_tk != EQ_TK && op_tk != LT_TK && op_tk != GT_TK) {
            errorMsg("Unsupported compare in If Stm");
            return nullptr;
        }
        cmp_op = cmpToken2Op(op_tk); advance();
        Exp* right = exp();
        op_tk = getToken();
        if (op_tk != THEN_TK) {
            errorMsg("expected THEN");
            return nullptr;
        }
        advance();
        op_tk = getToken();
        if (op_tk != INT_TK) {
            errorMsg("expected INT");
            return nullptr;
        }
        tmp = list.mid(pos, bios).toInt(nullptr, 10);
        return new IfStm(line_no, left, right, cmp_op, tmp);
    }
    case END_TK:
        return new EndStm(line_no);
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
        errorMsg("unsupport arithmetic token");
        return PLUS_OP;
    }
}

Op MainWindow::cmpToken2Op(Token tk) {
    switch (tk) {
    case EQ_TK:
        return EQ_OP;
    case LT_TK:
        return LT_OP;
    case GT_TK:
        return GT_OP;
    default:
        errorMsg("unsupport com token");
        return EQ_OP;
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
    QString tmp;
    Token curToken;
    std::stack<Token> opStack;
    std::queue<Poi> outQueue;

    while(pos < list.size() && arithToken(getToken())) {
        curToken = getToken();
        if (curToken == INT_TK) {
            tmp = list.mid(pos, bios);
            outQueue.push(Poi(2, PLUS_OP, tmp.toInt(nullptr, 10), ""));
        }
        if (curToken == STR_TK) { // treat as int
            tmp = list.mid(pos, bios);
            outQueue.push(Poi(3, PLUS_OP, 0, tmp.toStdString()));
        }
        if (curToken == LPAREN_TK) {
            opStack.push(LPAREN_TK);
        }
        if (curToken == RPAREN_TK) {
            while (!opStack.empty() && opStack.top() != LPAREN_TK) {
                outQueue.push(Poi(1, token2Op(opStack.top()), 0, ""));
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
                   opStack.top() != LPAREN_TK &&
                   prio[curToken] <= prio[opStack.top()]) { // left-asso
                outQueue.push(Poi(1, token2Op(opStack.top()), 0, ""));
                opStack.pop();
            }
            opStack.push(curToken);
        }
        if (curToken == POW_TK) {
            while (!opStack.empty() &&
                   opStack.top() != LPAREN_TK &&
                   prio[curToken] < prio[opStack.top()]) { // right-asso
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
    ui->treeDisplay->clear();

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
    ui->treeDisplay->clear();
    run();
}
