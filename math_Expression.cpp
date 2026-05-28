#include <iostream>
#include <string>
#include <cassert>
#include <cmath>

struct Transformer;
struct Number;
struct BinaryOperation;
struct FunctionCall;
struct Variable;

using namespace std;


struct Expression //базовая абстрактная структура математического выражения
{ 
    virtual ~Expression() { } //виртуальный деструктор
    virtual double evaluate() const = 0; //абстрактный метод «вычислить»
    virtual Expression *transform(Transformer *tr) const = 0;
};




struct Transformer //// класс трансформера (посетителя) pattern Visitor
{
    virtual ~Transformer() {} //деструктор
    virtual Expression *transformNumber(Number const *) = 0;
    virtual Expression *transformBinaryOperation(BinaryOperation const *) = 0;
    virtual Expression *transformFunctionCall(FunctionCall const *) = 0;
    virtual Expression *transformVariable(Variable const *) = 0;
};



struct Number : Expression // класс числа
{
    Number(double value) : value_(value) {}  //конструктор
    double value() const { return value_; }  //геттер

    //выполнение вычисления числа (число и есть само число)
    double evaluate() const override { return value_; } 
    
    Expression *transform(Transformer *tr) const override {
        return tr->transformNumber(this);
    }
private:
    double value_;
};




struct BinaryOperation : Expression //класс бинарная операция
{
    //всевозможные бинарные операции
    enum {
        PLUS = '+',
        MINUS = '-',
        DIV = '/',
        MUL = '*'
    };

    //конструктор
    BinaryOperation(Expression const *left, int op, Expression const *right)  
        : left_(left), op_(op), right_(right) { 
        assert(left_ && right_); 
    }

    //переопределнный деструктор
    ~BinaryOperation() override { 
        delete left_;
        delete right_;
    }
    
    
    //выполнение бинарной операции
    double evaluate() const override {
        double left = left_->evaluate();
        double right = right_->evaluate();
        switch (op_) {
            case PLUS:  return left + right;
            case MINUS: return left - right;
            case DIV:   return left / right;
            case MUL:   return left * right;
            default:    return 0.0;
        }
    }

    Expression *transform(Transformer *tr) const override {
        return tr->transformBinaryOperation(this);
    }

    //геттеры
    Expression const *left() const { return left_; }
    Expression const *right() const { return right_; }
    int operation() const { return op_; }

private:
    Expression const *left_;
    Expression const *right_;
    int op_;
};



//класс функций корня и модуля
struct FunctionCall : Expression 
{
    //конструктор
    FunctionCall(std::string const &name, Expression const *arg) : name_(name), arg_(arg)
    {
        assert(arg_);
        assert(name_ == "sqrt" || name_ == "abs"); // разрешены только вызов sqrt и abs
    } 
    
    //переопределенный деструктор
    ~FunctionCall() override { delete arg_; } 
    
    //выполнение операции корня и модуля
    double evaluate() const override {
        if (name_ == "sqrt") return sqrt(arg_->evaluate());
        return fabs(arg_->evaluate());
    }

    Expression *transform(Transformer *tr) const override {
        return tr->transformFunctionCall(this);
    }

    //гетерры
    std::string const & name() const { return name_; }
    Expression const *arg() const { return arg_; }
    
private:
    std::string const name_;
    Expression const *arg_;
};




//класс переменной
struct Variable : Expression
{
    //конструктор
    Variable(std::string const &name) : name_(name) { } 

    //геттер
    std::string const &name() const { return name_; } 

    //выполнение вычисления переменной
    double evaluate() const override { return 0.0; } 
    
    Expression *transform(Transformer *tr) const override
    {
        return tr->transformVariable(this);
    }
private:
    std::string const name_;
};




/*Задание 1. (5 баллов) Реализуйте класс CopySyntaxTree, который, используя шаблон Visitor,
выполняет копирование AST. Интерфейсы всех используемых классов приведены для
удобства, вам надо будет привести их полную реализацию. Заготовка класса CopySyntaxTree
находится в самом низу*/

struct CopySyntaxTree : Transformer //класс копирования дерева
{
    //конструктор
    Expression *transformNumber(Number const *number) override
    { 
        return new Number(number->value());
    }

    Expression *transformBinaryOperation(BinaryOperation const *binop) override
    { 
        Expression* leftCopy = binop->left()->transform(this);
        Expression* rightCopy = binop->right()->transform(this);
        return new BinaryOperation(leftCopy, binop->operation(), rightCopy);
    }

    Expression *transformFunctionCall(FunctionCall const *fcall) override
    {
        Expression* argCopy = fcall->arg()->transform(this);
        return new FunctionCall(fcall->name(), argCopy);
    }

    Expression *transformVariable(Variable const *var) override
    { 
        return new Variable(var->name());
    }
};



/*Задание 2. (5 баллов) В этой задаче вам необходимо реализовать сворачивание констант в
дереве (constant folding). Например, у нас есть выражение (точнее, дерево, описывающее это
выражение) abs(var * sqrt(32.0 - 16.0)), на выходе мы должны получить дерево для
следующего выражения abs(var * 4.0), т. е. подвыражение sqrt(32.0 - 16.0) было вычислено.
Для того, чтобы определить, что выражение (Expression) на самом деле является числом
(Number), используйте оператор dynamic_cast.
Все промежуточные узлы дерева, которые вы создали, нужно освободить.*/

struct FoldConstants : Transformer //класс сворачивания констнт
{ 
    //сворачивание числа. число невозможно свернуть, поэтому просто возвращаем число
    Expression *transformNumber(Number const *number) override
    { 
        return new Number(number->value());
    }

    //сворачивание переменной. переменая - есть переменная просто вазвращаем имя (ее не свернуть)
    Expression *transformVariable(Variable const *var) override
    { 
        return new Variable(var->name());
    }

    //своачивание функий. считаем ее аргумент. 
    Expression *transformFunctionCall(FunctionCall const *fcall) override
    {
        Expression* optimizedArg = fcall->arg()->transform(this);
        
        //если получили новым аргументом число, то считаем функию
        Number* isNumber = dynamic_cast<Number*>(optimizedArg);
        if (isNumber)
        {
            double val = isNumber->value();
            delete optimizedArg; //удаляем вспомогательный объект числа
            
            //выбор функции
            if (fcall->name() == "sqrt")
                return new Number(sqrt(val));
            else
                return new Number(fabs(val));
        }
        
        //если получили новым аргументом выражение, то оставляем так
        return new FunctionCall(fcall->name(), optimizedArg);
    }

    //сворачивание бинарных операций.считаем операции выражения
    Expression *transformBinaryOperation(BinaryOperation const *binop) override
    { 
        //использую рекурсию для сворачивания лвой и правых частей
        Expression* optimizedLeft = binop->left()->transform(this);
        Expression* optimizedRight = binop->right()->transform(this);

        Number* leftNum = dynamic_cast<Number*>(optimizedLeft);
        Number* rightNum = dynamic_cast<Number*>(optimizedRight);

        //если левая и правая часть стали числом, то считаем последнюю операцию и возвращаем число-результат
        if (leftNum && rightNum)
        {
            double leftVal = leftNum->value();
            double rightVal = rightNum->value();
            int op = binop->operation();
            
            //удаляю вспомогательные объекты
            delete optimizedLeft;
            delete optimizedRight;

            //вычисление операции
            if (op == BinaryOperation::PLUS) 
                return new Number(leftVal + rightVal);
            if (op == BinaryOperation::MINUS)
                return new Number(leftVal - rightVal);
            if (op == BinaryOperation::MUL)
                return new Number(leftVal * rightVal);
            if (op == BinaryOperation::DIV)
                return new Number(leftVal / rightVal);
        }

        //случай, когда в одной из частей левой или правой встречается переменная.
        return new BinaryOperation(optimizedLeft, binop->operation(), rightNum ? optimizedRight : optimizedRight);
    }
};




int main() {
    cout.precision(4);
    
    
    {
        cout << "Задание 0. Проверка иеархии" << endl;
        Expression * e1 = new Number(1.234);
        Expression * e2 = new Number(-1.234);
        Expression * e3 = new BinaryOperation(e1, BinaryOperation::DIV, e2);
        cout << "Выражение: 1.234 / -1.234     Ответ: " <<e3->evaluate() << endl;
        delete e3; 
    }
    
    {
        Expression* n32 = new Number(32.0);
        Expression* n16 = new Number(16.0);
        Expression* minus = new BinaryOperation(n32, BinaryOperation::MINUS, n16);
        Expression* callSqrt = new FunctionCall("sqrt", minus);
        Expression* n2 = new Number(2.0);
        Expression* mult = new BinaryOperation(n2, BinaryOperation::MUL, callSqrt);
        Expression* callAbs = new FunctionCall("abs", mult);
        cout << "Выражение: abs(2.0 * sqrt(32.0 - 16.0))     Ответ: " << callAbs->evaluate() << endl;
        delete callAbs; 
    }
    
    {

        cout << endl;
        cout << "Задание 1. Копирование дерева" << endl;
        Number* n32 = new Number(32.0);
        Number* n16 = new Number(16.0);
        BinaryOperation* minus = new BinaryOperation(n32, BinaryOperation::MINUS, n16);
        FunctionCall* callSqrt = new FunctionCall("sqrt", minus);
        Variable* var = new Variable("var");
        BinaryOperation* mult = new BinaryOperation(var, BinaryOperation::MUL, callSqrt);
        FunctionCall* callAbs = new FunctionCall("abs", mult);
        
        CopySyntaxTree CST;
        Expression* newExpr = callAbs->transform(&CST);

        cout << "Исходное дерево " << callAbs->evaluate() << endl;
        cout << "Скопированное дерево " << newExpr->evaluate() << endl;

        delete callAbs; 
        delete newExpr; 
    }
    

    {
        cout << endl;
        cout << "Задание 2. Сворачивание выражения констант" << endl;
        Number* n32 = new Number(32.0);
        Number* n16 = new Number(16.0);
        BinaryOperation* minus = new BinaryOperation(n32, BinaryOperation::MINUS, n16);
        FunctionCall* callSqrt = new FunctionCall("sqrt", minus);
        Variable* var = new Variable("var");
        BinaryOperation* mult = new BinaryOperation(var, BinaryOperation::MUL, callSqrt);
        FunctionCall* callAbs = new FunctionCall("abs", mult);
        
        FoldConstants FC;
        Expression* newExpr = callAbs->transform(&FC);

        cout << "Исходное дерево abs(var * sqrt(32.0 - 16.0)) вычисляется в: " << callAbs->evaluate() << endl;
        cout << "Оптимизированное дерево abs(var * 4.0) вычисляется в: " << newExpr->evaluate() << endl;


        delete callAbs;  
        delete newExpr;  
    }

    return 0;
}

