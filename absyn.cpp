#include "absyn.h"

std::map<std::string, int> intMap;
std::map<Token, int> prio;
std::set<int> lines;
std::map<int, WrappedStm> stms;
std::map<Op, std::string> opC;
bool inputFlag = false;
std::string inputVar;

int quick_power(int a, int b) {
    if (!b) return 1;
    int res = quick_power(a, b / 2);
    res *= res;
    if (b % 2) res *= a;
    return res;
}

int nextLine(int line) {
    auto it = lines.upper_bound(line);
    if (it == lines.end()) return -1;
    return *it;
}


void CompoundExp::free()
{
    left_->free();
    right_->free();
}



int CompoundExp::exec(QTextBrowser *res, QTextBrowser *tree, int d)
{
    print(tree, d);
    int left = left_->exec(res, tree, d + 1);
    int right = right_->exec(res, tree, d + 1);
    int result = 0;
    switch (op_) {
    case PLUS_OP:
        result = left + right;
        break;
    case MINUS_OP:
        result = left - right;
        break;
    case MUL_OP:
        result = left * right;
        break;
    case DIV_OP:
        result = left / right;
        break;
    case POW_OP:
        result = quick_power(left, right);
        break;
    default:
        res->append("Unsuporrted arith");
        break;
    }
    return result;
}

void CompoundExp::print(QTextBrowser *tree, int d)
{
    QString res;
    for (int i = 0; i < d ; ++i) res += '\t';
    res += QString(opC[op_].c_str());
    tree->append(res);
}

int RemStm::exec(QTextBrowser *res, QTextBrowser *tree, int d)
{
    print(tree, d);
    return nextLine(pos_);
}

void RemStm::print(QTextBrowser *tree, int d)
{
    QString res;
    for (int i = 0; i < 4 * d; ++i) res += " ";
    res += QString::number(pos_) + " REM\n";
    res += '\t' + QString(str_.c_str());
    tree->append(res);
}


int LetStm::exec(QTextBrowser *res, QTextBrowser *tree, int d)
{
    print(tree, d);
    intMap[sym_] = exp_->exec(res, tree, d + 1);
    return nextLine(pos_);
}

void LetStm::free()
{
    exp_->free();
}

void LetStm::print(QTextBrowser *tree, int d)
{
    QString res;
    res += QString::number(pos_) + " LET =\n";
    res += '\t' + QString(sym_.c_str());
    tree->append(res);
}

int PrintStm::exec(QTextBrowser *res, QTextBrowser *tree, int d)
{
    print(tree, d);
    int ans = exp_->exec(res, tree, d + 1);
    res->append(QString::number(ans));
    return nextLine(pos_);
}

void PrintStm::free()
{
    exp_->free();
}

void PrintStm::print(QTextBrowser *tree, int d)
{
    QString res;
    for (int i = 0; i < d; ++i) res += '\t';
    res += QString::number(pos_) + " PRINT";
    tree->append(res);
}

int InputStm::exec(QTextBrowser *res, QTextBrowser *tree, int d)
{
    print(tree, d);
    inputFlag = true;
    inputVar = sym_;
    return nextLine(pos_);
}

void InputStm::print(QTextBrowser *tree, int d)
{
    QString res;
    for (int i = 0; i < d; ++i) res += '\t';
    res += QString::number(pos_) + " INPUT\n";
    for (int i = 0; i < d + 1; ++i) res += '\t';
    res += QString(sym_.c_str());
    tree->append(res);
}

int GotoStm::exec(QTextBrowser *res, QTextBrowser *tree, int d)
{
    print(tree, d);
    return to_;
}

void GotoStm::print(QTextBrowser *tree, int d)
{
    QString res;
    for (int i = 0; i < d; ++i) res += '\t';
    res += QString::number(pos_) + " GOTO " +QString::number(to_);
    tree->append(res);
}

int IfStm::exec(QTextBrowser *res, QTextBrowser *tree, int d)
{
    print(tree, d);
    int left= exp1_->exec(res, tree, d + 1);
    int right = exp2_->exec(res, tree, d + 1);
    bool flag = false;
    switch (op_) {
    case EQ_OP:
        if (left == right) flag = true;
        break;
    case LT_OP:
        if (left < right) flag = true;
        break;
    case GT_OP:
        if (left > right) flag = true;
        break;
    default:
        res->append("Unsupported compare");
        break;
    }
    if (flag) {
        QString tmp;
        for (int i = 0; i < d; ++i) tmp += '\t';
        tmp += QString::number(to_);
        return to_;
    } else {
        QString tmp;
        for (int i = 0; i < d; ++i) tmp += '\t';
        tmp += QString::number(nextLine(pos_));
        return nextLine(pos_);
    }
}

void IfStm::free()
{
    exp1_->free();
    exp2_->free();
}

void IfStm::print(QTextBrowser *tree, int d)
{
    QString res;
    for (int i = 0; i < d; ++i) res += '\t';
    res += QString::number(pos_) + " IF THEN\n";
    for (int i = 0; i < d; ++i) res += '\t';
    res += QString(opC[op_].c_str());
    // TODO
    tree->append(res);
}

int EndStm::exec(QTextBrowser *res, QTextBrowser *tree, int d)
{
    print(tree, d);
    return -1; // end
}

void EndStm::print(QTextBrowser *tree, int d)
{
   QString res;
   for (int i = 0; i < d; ++i) res += '\t';
   res += QString::number(pos_) + " END";
   tree->append(res);
}

int ConstExp::exec(QTextBrowser *res, QTextBrowser *tree, int d)
{
    print(tree, d);
    return val_;
}

void ConstExp::print(QTextBrowser *tree, int d)
{
    QString res;
    for (int i = 0; i < d; ++i) res += '\t';
    res += QString::number(val_);
    tree->append(res);
}

int IdExp::exec(QTextBrowser *res, QTextBrowser *tree, int d)
{
    print(tree, d);
    if (!intMap.count(sym_)) { // not decleared
        QString opt = QString("Var ") + QString(sym_.c_str()) + QString(" is not decleared");
        res->append(opt);
        return 0;
    }
    return intMap[sym_];
}

void IdExp::print(QTextBrowser *tree, int d)
{
    QString res;
    for (int i = 0; i < d; ++i) res += '\t';
    res += QString(sym_.c_str());
    tree->append(res);
}
