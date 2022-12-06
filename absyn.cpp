#include "absyn.h"

std::map<std::string, int> intMap;
std::map<Token, int> prio;
std::set<int> lines;
std::map<int, WrappedStm> stms;
std::map<Op, std::string> opC;

int quick_power(int a, int b) {
    if (!b) return 1;
    int res = quick_power(a, b / 2);
    res *= res;
    if (b % 2) res *= a;
    return res;
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
    auto it = lines.upper_bound(pos_);
    if (it == lines.end()) return -1;
    return *it;
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
    auto it = lines.upper_bound(pos_);
    if (it == lines.end()) return -1;
    return *it;
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
    auto it = lines.upper_bound(pos_);
    if (it == lines.end()) return -1;
    return *it;
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

}

void InputStm::print(QTextBrowser *tree, int d)
{
    QString res;
    for (int i = 0; i < d; ++i) res += '\t';
    res += QString::number(pos_) + " INPUT";
    tree->append(res);
}

int GotoStm::exec(QTextBrowser *res, QTextBrowser *tree, int d)
{

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
    // TODO
    tree->append(res);
}

int EndStm::exec(QTextBrowser *res, QTextBrowser *tree, int d)
{

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
