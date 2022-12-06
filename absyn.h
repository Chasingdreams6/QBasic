#ifndef ABSYN_H
#define ABSYN_H
#pragma once

#include <string>
#include <map>
#include <stack>
#include <queue>
#include <set>
#include <QTextBrowser>
#include "QFileDialog"
#include "QFile"

class Exp;
class Stm;
struct WrappedStm;
extern std::map<std::string, int> intMap;

enum Op {
    PLUS_OP,
    MINUS_OP,
    MUL_OP,
    DIV_OP,
    POW_OP,
    EQ_OP,
    LT_OP,
    GT_OP,
};
struct Poi {
    int kind_; // 1 is op, 2 is val, 3 is sym
    Op op_;
    int val_;
    std::string sym_;
    Poi(int kind, Op op, int val, std::string sym) :
        kind_(kind), op_(op), val_(val), sym_(std::move(sym)){}
};
enum Token {
    NULL_TK,
    INT_TK,
    STR_TK,
    REM_TK,
    LET_TK,
    PRINT_TK,
    INPUT_TK,
    GO_TK,
    IF_TK,
    END_TK,
    PLUS_TK,
    MINUS_TK,
    MUL_TK,
    POW_TK,
    DIV_TK,
    LPAREN_TK,
    RPAREN_TK,
    EQ_TK,
    LT_TK,
    GT_TK
};

class Stm {
public:
    int pos_;
    Stm() {}
    Stm(int pos) : pos_(pos) {}
    virtual int exec(QTextBrowser *res, QTextBrowser *tree, int d) = 0;
    virtual void free() = 0;
    virtual void print(QTextBrowser *tree, int d) = 0;
};

struct WrappedStm {
    int pos_;
    std::string ori_;
    Stm *stm_;
    WrappedStm () {}
    WrappedStm(int pos, Stm *stm, const std::string ori)
        : pos_(pos), ori_(ori), stm_(stm){}
};

class RemStm : public Stm {
private:
    std::string str_;
public:
    RemStm(int pos, std::string str) : Stm(pos), str_(std::move(str)){}
    int exec(QTextBrowser *res, QTextBrowser *tree, int d) override;
    void free() override {}
    void print(QTextBrowser *tree, int d) override;
};

class LetStm : public Stm {
private:
    std::string sym_;
    Exp* exp_;
public:
    LetStm(int pos, std::string sym, Exp *exp) :
        Stm(pos), sym_(sym), exp_(exp) {}
    int exec(QTextBrowser *res, QTextBrowser *tree, int d) override;
    void free() override;
    void print(QTextBrowser *tree, int d) override;
};

class PrintStm : public Stm {
private:
    Exp* exp_;
public:
    PrintStm(int pos, Exp *exp) :
        Stm(pos), exp_(exp) {}
    int exec(QTextBrowser *res, QTextBrowser *tree, int d) override;
    void free() override;
    void print(QTextBrowser *tree, int d) override;
};

class InputStm : public Stm {
private:
    std::string sym_;
public:
    InputStm(int pos, std::string sym) :
        Stm(pos), sym_(std::move(sym)) {}
    int exec(QTextBrowser *res, QTextBrowser *tree, int d) override;
    void free() override {}
    void print(QTextBrowser *tree, int d) override;
};

class GotoStm : public Stm {
private:
    int to_;
public:
    GotoStm(int pos, int to) :
        Stm(pos), to_(to) {}
    int exec(QTextBrowser *res, QTextBrowser *tree, int d) override;
    void free() override {}
    void print(QTextBrowser *tree, int d) override;
};

class IfStm : public Stm {
private:
    Exp *exp1_, *exp2_;
    enum Op op_;
    int to_;
public:
    IfStm(int pos, Exp *exp1, Exp *exp2, enum Op op, int to) :
        Stm(pos), exp1_(exp1), exp2_(exp2), op_(op), to_(to) {}
    int exec(QTextBrowser *res, QTextBrowser *tree, int d) override;
    void free() override;
    void print(QTextBrowser *tree, int d) override;
};

class EndStm : public Stm {
private:
public:
    EndStm(int pos) : Stm(pos) {}
    int exec(QTextBrowser *res, QTextBrowser *tree, int d) override;
    void free() override {}
    void print(QTextBrowser *tree, int d) override;
};

class Exp {
public:
    Exp () {}
    virtual void free() = 0;
    virtual int exec(QTextBrowser *res, QTextBrowser *tree, int d) = 0;
    virtual void print(QTextBrowser *tree, int d) = 0;
};

class ConstExp : public Exp {
private:
    int val_;
public:
    ConstExp(int val) : val_(val) {}
    void free() override {}
    int exec(QTextBrowser *res, QTextBrowser *tree, int d) override;
    void print(QTextBrowser *tree, int d) override;
};

class IdExp : public Exp {
private:
    std::string sym_;
public:
    IdExp(std::string sym) : sym_(std::move(sym)) {}
    void free() override {}
    int exec(QTextBrowser *res, QTextBrowser *tree, int d) override;
    void print(QTextBrowser *tree, int d) override;
};

class CompoundExp : public Exp {
private:
    Exp *left_, *right_;
    enum Op op_;
public:
    CompoundExp(Exp *left, Exp *right, Op op) :
        left_(left), right_(right), op_(op) {}
    void free() override;
    int exec(QTextBrowser *res, QTextBrowser *tree, int d) override;
    void print(QTextBrowser *tree, int d) override;
};

#endif // ABSYN_H
