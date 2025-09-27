#include "matlib.h"
#include <iostream>
#include <cctype>
#include <stdexcept>
#include <cmath>
#include <stack>
#include <sstream>
#include <optional>
#include <algorithm>

// ==================== MathExpressionParser ====================

MathExpressionParser::MathExpressionParser() {
    // Стандартные математические константы
    //variables.reserve(10);
    variables["pi"] = 3.14159265358979323846;
    variables["e"] = 2.71828182845904523536;
    variables["inf"] = std::numeric_limits<double>::infinity();
    variables["nan"] = std::numeric_limits<double>::quiet_NaN();

    // Стандартные математические функции
    //functions.reserve(15);
    functions["sin"] = [](double x) { return std::sin(x); };
    functions["cos"] = [](double x) { return std::cos(x); };
    functions["tan"] = [](double x) { return std::tan(x); };
    functions["asin"] = [](double x) { return std::asin(x); };
    functions["acos"] = [](double x) { return std::acos(x); };
    functions["atan"] = [](double x) { return std::atan(x); };

    functions["sqrt"] = [](double x) {
        if (x < 0) throw std::runtime_error("Square root of negative number");
        return std::sqrt(x);
        };
    functions["cbrt"] = [](double x) { return std::cbrt(x); };
    functions["exp"] = [](double x) { return std::exp(x); };
    functions["ln"] = [](double x) {
        if (x <= 0) throw std::runtime_error("Logarithm of non-positive number");
        return std::log(x);
        };
    functions["log10"] = [](double x) {
        if (x <= 0) throw std::runtime_error("Log10 of non-positive number");
        return std::log10(x);
        };

    functions["abs"] = [](double x) { return std::abs(x); };
    functions["floor"] = [](double x) { return std::floor(x); };
    functions["ceil"] = [](double x) { return std::ceil(x); };
    functions["round"] = [](double x) { return std::round(x); };
}

void MathExpressionParser::setVariable(const std::string& name, double value) {
    variables[name] = value;
}

double MathExpressionParser::getVariable(const std::string& name) const {
    auto it = variables.find(name);
    if (it != variables.end()) return it->second;
    throw std::runtime_error("Unknown variable: " + name);
}

bool MathExpressionParser::hasVariable(const std::string& name) const {
    return variables.find(name) != variables.end();
}

void MathExpressionParser::setFunction(const std::string& name, std::function<double(double)> func) {
    functions[name] = func;
}

bool MathExpressionParser::hasFunction(const std::string& name) const {
    return functions.find(name) != functions.end();
}

double MathExpressionParser::parse(const std::string& expression) {
    if (expression.empty()) throw std::runtime_error("Empty expression");

    size_t pos = 0;
    double result = parseExpression(expression, pos);

    skipWhitespace(expression, pos);
    if (pos < expression.length()) {
        throw std::runtime_error("Unexpected characters at end of expression");
    }

    return result;
}

double MathExpressionParser::parseShuntingYard(const std::string& expression) {
    return evaluateShuntingYard(expression);
}

std::function<double(double)> MathExpressionParser::createFunction(const std::string& expression) {
    return createFunction(expression, "x");
}

std::function<double(double)> MathExpressionParser::createFunction(const std::string& expression, const std::string& variable) {
    return [this, expression, variable](double x) {
        MathExpressionParser tempParser = *this;
        tempParser.setVariable(variable, x);
        return tempParser.parse(expression);
        };
}

std::function<double(double)> MathExpressionParser::compose(const std::vector<std::function<double(double)>>& funcs) {
    if (funcs.empty()) return [](double x) { return x; };

    return [funcs](double x) {
        double result = x;
        for (const auto& f : funcs) {
            result = f(result);
        }
        return result;
        };
}

bool MathExpressionParser::validate(const std::string& expression) {
    try {
        parse(expression);
        return true;
    }
    catch (...) {
        return false;
    }
}

void MathExpressionParser::listVariables() const {
    std::cout << "Variables:" << std::endl;
    for (const auto& var : variables) {
        std::cout << "  " << var.first << " = " << var.second << std::endl;
    }
}

void MathExpressionParser::listFunctions() const {
    std::cout << "Functions:" << std::endl;
    for (const auto& func : functions) {
        std::cout << "  " << func.first << "(x)" << std::endl;
    }
}

// ==================== Рекурсивный спуск ====================

void MathExpressionParser::skipWhitespace(const std::string& expr, size_t& pos) {
    const size_t len = expr.length();
    while (pos < len && std::isspace(expr[pos])) {
        pos++;
    }
}

double MathExpressionParser::parseExpression(const std::string& expr, size_t& pos) {
    double result = parseTerm(expr, pos);

    const size_t len = expr.length();
    while (pos < len) {
        skipWhitespace(expr, pos);
        if (pos >= len) break;

        char op = expr[pos];
        if (op == '+' || op == '-') {
            pos++;
            double term = parseTerm(expr, pos);
            result = (op == '+') ? result + term : result - term;
        }
        else {
            break;
        }
    }
    return result;
}

double MathExpressionParser::parseTerm(const std::string& expr, size_t& pos) {
    double result = parseFactor(expr, pos);

    const size_t len = expr.length();
    while (pos < len) {
        skipWhitespace(expr, pos);
        if (pos >= len) break;

        char op = expr[pos];
        if (op == '*' || op == '/') {
            pos++;
            double factor = parseFactor(expr, pos);
            if (op == '*') result *= factor;
            else {
                if (factor == 0.0) throw std::runtime_error("Division by zero");
                result /= factor;
            }
        }
        else {
            break;
        }
    }
    return result;
}

double MathExpressionParser::parseFactor(const std::string& expr, size_t& pos) {
    double result = parsePrimary(expr, pos);

    const size_t len = expr.length();
    while (pos < len) {
        skipWhitespace(expr, pos);
        if (pos >= len) break;

        if (expr[pos] == '^') {
            pos++;
            result = std::pow(result, parsePrimary(expr, pos));
        }
        else {
            break;
        }
    }
    return result;
}

double MathExpressionParser::parsePrimary(const std::string& expr, size_t& pos) {
    skipWhitespace(expr, pos);
    const size_t len = expr.length();
    if (pos >= len) throw std::runtime_error("Unexpected end of expression");

    char ch = expr[pos];

    if (ch == '(') {
        pos++;
        double result = parseExpression(expr, pos);
        skipWhitespace(expr, pos);
        if (pos >= len || expr[pos] != ')') {
            throw std::runtime_error("Expected ')'");
        }
        pos++;
        return result;
    }
    else if (std::isdigit(ch) || ch == '.') {
        return parseNumber(expr, pos);
    }
    else if (std::isalpha(ch) || ch == '_') {
        return parseIdentifier(expr, pos);
    }
    else if (ch == '-') {
        pos++;
        return -parsePrimary(expr, pos);
    }
    else if (ch == '+') {
        pos++;
        return parsePrimary(expr, pos);
    }
    else {
        throw std::runtime_error("Unexpected character: '" + std::string(1, ch) + "'");
    }
}

double MathExpressionParser::parseNumber(const std::string& expr, size_t& pos) {
    const size_t start = pos;
    const size_t len = expr.length();
    bool hasDecimal = false;

    while (pos < len && (std::isdigit(expr[pos]) || expr[pos] == '.')) {
        if (expr[pos] == '.') {
            if (hasDecimal) throw std::runtime_error("Invalid number format");
            hasDecimal = true;
        }
        pos++;
    }

    if (pos == start) throw std::runtime_error("Expected number");

    // Используем быстрый парсинг чисел
    const char* start_ptr = expr.c_str() + start;
    char* end_ptr;
    double result = std::strtod(start_ptr, &end_ptr);

    if (end_ptr == start_ptr) throw std::runtime_error("Invalid number format");

    return result;
}

double MathExpressionParser::parseIdentifier(const std::string& expr, size_t& pos) {
    const size_t start = pos;
    const size_t len = expr.length();

    while (pos < len && (std::isalnum(expr[pos]) || expr[pos] == '_')) {
        pos++;
    }

    std::string name(expr.substr(start, pos - start));
    skipWhitespace(expr, pos);

    if (pos < len && expr[pos] == '(') {
        // Это функция
        pos++;
        double arg = parseExpression(expr, pos);
        skipWhitespace(expr, pos);
        if (pos >= len || expr[pos] != ')') {
            throw std::runtime_error("Expected ')' after function argument");
        }
        pos++;

        auto it = functions.find(name);
        if (it != functions.end()) {
            return it->second(arg);
        }
        throw std::runtime_error("Unknown function: " + name);
    }
    else {
        // Это переменная
        auto it = variables.find(name);
        if (it != variables.end()) {
            return it->second;
        }
        throw std::runtime_error("Unknown variable: " + name);
    }
}

// ==================== Shunting Yard ====================

bool MathExpressionParser::isOperator(char c) const {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
}

void MathExpressionParser::applyOperator(std::stack<double>& values, char op) {
    if (values.size() < 2) {
        throw std::runtime_error("Not enough operands for operator " + std::string(1, op));
    }

    double b = values.top(); values.pop();
    double a = values.top(); values.pop();

    switch (op) {
    case '+': values.push(a + b); break;
    case '-': values.push(a - b); break;
    case '*': values.push(a * b); break;
    case '/':
        if (b == 0.0) throw std::runtime_error("Division by zero");
        values.push(a / b);
        break;
    case '^': values.push(std::pow(a, b)); break;
    default: throw std::runtime_error("Unknown operator: " + std::string(1, op));
    }
}

double MathExpressionParser::evaluateShuntingYard(const std::string& expr) {
    std::stack<double> values;
    std::stack<char> ops;

    static const std::map<char, int> precedence{
        {'+', 1}, {'-', 1},
        {'*', 2}, {'/', 2},
        {'^', 3}
    };

    const size_t len = expr.length();
    for (size_t i = 0; i < len; i++) {
        char current = expr[i];
        if (std::isspace(current)) continue;

        if (std::isdigit(current) || current == '.') {
            size_t start = i;
            while (i < len && (std::isdigit(expr[i]) || expr[i] == '.')) {
                i++;
            }
            i--;

            const char* start_ptr = expr.c_str() + start;
            char* end_ptr;
            double value = std::strtod(start_ptr, &end_ptr);
            if (end_ptr == start_ptr) throw std::runtime_error("Invalid number format");
            values.push(value);
        }
        else if (current == '(') {
            ops.push('(');
        }
        else if (current == ')') {
            while (!ops.empty() && ops.top() != '(') {
                applyOperator(values, ops.top());
                ops.pop();
            }
            if (ops.empty()) throw std::runtime_error("Mismatched parentheses");
            ops.pop(); // Remove '('
        }
        else if (isOperator(current)) {
            while (!ops.empty() && ops.top() != '(' &&
                precedence.at(ops.top()) >= precedence.at(current)) {
                applyOperator(values, ops.top());
                ops.pop();
            }
            ops.push(current);
        }
        else {
            throw std::runtime_error("Unknown character: " + std::string(1, current));
        }
    }

    while (!ops.empty()) {
        if (ops.top() == '(') throw std::runtime_error("Mismatched parentheses");
        applyOperator(values, ops.top());
        ops.pop();
    }

    if (values.size() != 1) throw std::runtime_error("Invalid expression");
    return values.top();
}

// ==================== FunctionComposer ====================

FunctionComposer::FunctionComposer() {}

FunctionComposer& FunctionComposer::add(const std::function<double(double)>& func) {
    functions.push_back(func);
    return *this;
}

FunctionComposer& FunctionComposer::addMathFunction(const std::string& expression, MathExpressionParser& parser) {
    functions.push_back(parser.createFunction(expression));
    return *this;
}

FunctionComposer& FunctionComposer::addMathFunction(const std::string& expression, MathExpressionParser& parser, const std::string& variable) {
    functions.push_back(parser.createFunction(expression, variable));
    return *this;
}

double FunctionComposer::operator()(double x) const {
    double result = x;
    for (const auto& f : functions) {
        result = f(result);
    }
    return result;
}

size_t FunctionComposer::size() const {
    return functions.size();
}

void FunctionComposer::clear() {
    functions.clear();
}

// ==================== Вспомогательные функции ====================

namespace matlib {
    double evaluate(const std::string& expression) {
        MathExpressionParser parser;
        return parser.parse(expression);
    }

    std::function<double(double)> makeFunction(const std::string& expression) {
        MathExpressionParser parser;
        return parser.createFunction(expression);
    }

    std::function<double(double)> makeFunction(const std::string& expression, const std::string& variable) {
        MathExpressionParser parser;
        return parser.createFunction(expression, variable);
    }
}
