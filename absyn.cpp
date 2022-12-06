#include "absyn.h"

std::map<std::string, int> intMap;
std::map<Op, int> prio;
std::set<int> lines;
std::map<int, WrappedStm> stms;

int quick_power(int a, int b) {
    if (!b) return 1;
    int res = quick_power(a, b / 2);
    res *= res;
    if (b % 2) res *= a;
    return res;
}

int CompoundExp::getVal()
{
    int res = 0;
    switch (op_) {
    case PLUS_OP:
        res = left_->getVal() + right_->getVal();
        break;
    case MINUS_OP:
        res = left_->getVal() - right_->getVal();
        break;
    case MUL_OP:
        res = left_->getVal() * right_->getVal();
        break;
    case DIV_OP:
        res = left_->getVal() / right_->getVal();
        break;
    case POW_OP:
        res = quick_power(left_->getVal(), right_->getVal());
        break;
    default:
        break;
    }
    return res;
}

void CompoundExp::free()
{
    left_->free();
    right_->free();
}

int RemStm::exec(QTextBrowser *res, QTextBrowser *tree)
{
    auto it = lines.upper_bound(pos_);
    if (it == lines.end()) return -1;
    return *it;
}

int LetStm::exec(QTextBrowser *res, QTextBrowser *tree)
{
    intMap[sym_] = exp_->getVal();
    auto it = lines.upper_bound(pos_);
    if (it == lines.end()) return -1;
    return *it;
}

void LetStm::free()
{
    exp_->free();
}

int PrintStm::exec(QTextBrowser *res, QTextBrowser *tree)
{
    int ans = exp_->getVal();
    res->append(QString::number(ans));
    auto it = lines.upper_bound(pos_);
    if (it == lines.end()) return -1;
    return *it;
}

void PrintStm::free()
{
    exp_->free();
}

int InputStm::exec(QTextBrowser *res, QTextBrowser *tree)
{

}

int GotoStm::exec(QTextBrowser *res, QTextBrowser *tree)
{

}

int IfStm::exec(QTextBrowser *res, QTextBrowser *tree)
{

}

void IfStm::free()
{
    exp1_->free();
    exp2_->free();
}

int EndStm::exec(QTextBrowser *res, QTextBrowser *tree)
{

}
