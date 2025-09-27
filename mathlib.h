#ifndef MATLIB_H
#define MATLIB_H

#include <string>
#include <functional>
#include <map>
#include <vector>
#include <memory>
#include <stack>

class MathExpressionParser {
private:
    std::map<std::string, double> variables;
    std::map<std::string, std::function<double(double)>> functions;

public:
    // Конструктор
    MathExpressionParser();

    // Управление переменными
    void setVariable(const std::string& name, double value);
    double getVariable(const std::string& name) const;
    bool hasVariable(const std::string& name) const;

    // Управление функциями
    void setFunction(const std::string& name, std::function<double(double)> func);
    bool hasFunction(const std::string& name) const;

    // Основные методы парсинга
    double parse(const std::string& expression);
    double parseShuntingYard(const std::string& expression);

    // Создание функциональных объектов
    std::function<double(double)> createFunction(const std::string& expression);
    std::function<double(double)> createFunction(const std::string& expression, const std::string& variable);

    // Композиция функций
    std::function<double(double)> compose(const std::vector<std::function<double(double)>>& functions);

    // Валидация выражения
    bool validate(const std::string& expression);

    // Информация о парсере
    void listVariables() const;
    void listFunctions() const;

private:
    // Рекурсивный спуск парсер
    double parseExpression(const std::string& expr, size_t& pos);
    double parseTerm(const std::string& expr, size_t& pos);
    double parseFactor(const std::string& expr, size_t& pos);
    double parsePrimary(const std::string& expr, size_t& pos);
    double parseNumber(const std::string& expr, size_t& pos);
    double parseIdentifier(const std::string& expr, size_t& pos);
    void skipWhitespace(const std::string& expr, size_t& pos);

    // Shunting Yard парсер
    double evaluateShuntingYard(const std::string& expr);

    // Вспомогательные функции
    bool isOperator(char c) const;
    void applyOperator(std::stack<double>& values, char op);
};

// Класс для композиции функций
class FunctionComposer {
private:
    std::vector<std::function<double(double)>> functions;

public:
    FunctionComposer();

    // Добавление функций
    FunctionComposer& add(const std::function<double(double)>& func);
    FunctionComposer& addMathFunction(const std::string& expression, MathExpressionParser& parser);
    FunctionComposer& addMathFunction(const std::string& expression, MathExpressionParser& parser, const std::string& variable);

    // Выполнение композиции
    double operator()(double x) const;

    // Информация
    size_t size() const;
    void clear();
};

// Вспомогательные функции
namespace matlib {
    double evaluate(const std::string& expression);
    std::function<double(double)> makeFunction(const std::string& expression);
    std::function<double(double)> makeFunction(const std::string& expression, const std::string& variable);



}

#endif // MATLIB_H
