/*
 * Copyright (c) 2019 kewenyu

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <utility>
#include <vector>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <memory>
#include <limits>
#include <cmath>

#include "draw.h"

static bool isStrNumber(const std::string &str) {
    bool hasDecimal = false;
    bool hasMinus = false;
    std::string::const_iterator iter = str.cbegin();

    while(iter != str.cend()) {
        if (*iter == '.') {
            if (hasDecimal || str.size() == 1) {
                return false;
            } else {
                hasDecimal = true;
            }
        } else if (*iter == '-') {
            if (hasMinus || (iter - str.cbegin()) > 0 || str.size() == 1) {
                return false;
            } else {
                hasMinus = true;
            }
        } else if (*iter < 48 || *iter > 57) {
            return false;
        }

        iter++;
    }

    return true;
}

static void stripRedundantSpace(std::string &str) noexcept {
    bool lastIsSpace = false;
    std::string::reverse_iterator riter = str.rbegin();

    while (riter != str.rend()) {
        if (*riter == ' ') {
            if (lastIsSpace) {
                str.erase(std::next(riter).base());
            } else {
                lastIsSpace = true;
            }
        } else {
            lastIsSpace = false;
        }

        riter++;
    }

    if (*(str.end() - 1) != ' ') {
        str.push_back(' ');
    }
}

static int getOperandNum(const std::string &str) {
    if (isStrNumber(str)) {
        return 0;
    }

    static const std::unordered_map<std::string, int> strNumMap {
            // Boolean Operator
            std::make_pair("not", 1),
            std::make_pair("and", 2),
            std::make_pair("or", 2),
            std::make_pair(">", 2),
            std::make_pair("<", 2),
            std::make_pair(">=", 2),
            std::make_pair("<=", 2),
            std::make_pair("=", 2),

            // Numeric Operator
            std::make_pair("abs", 1),
            std::make_pair("+", 2),
            std::make_pair("-", 2),
            std::make_pair("*", 2),
            std::make_pair("/", 2),
            std::make_pair("pow", 2),
            std::make_pair("max", 2),
            std::make_pair("min", 2),

            // Ternary Operator
            std::make_pair("?", 3),

            // Placeholder
            std::make_pair("x", 0),
            std::make_pair("y", 0),
    };

    if (strNumMap.find(str) == strNumMap.end()) {
        throw std::runtime_error("getOperandNum: Invalid operator: " + str);
    }

    return strNumMap.at(str);
}

std::shared_ptr<ExpTreeNode> buildExpressionTree(std::string &expStr) {
    std::shared_ptr<ExpTreeNode> nodePtr;
    std::string tempStr;
    std::vector<std::shared_ptr<ExpTreeNode>> opStack;

    stripRedundantSpace(expStr);
    std::string::const_iterator iter = expStr.cbegin();

    while (iter != expStr.cend()) {
        if (*iter == ' ') {
            int opNum = getOperandNum(tempStr);

            nodePtr = std::make_shared<ExpTreeNode>();
            nodePtr->opNum = opNum;
            nodePtr->opStr = tempStr;

            for (int i = opNum; i > 0; --i) {
                std::shared_ptr<ExpTreeNode> opNode = opStack[opStack.size() - i];
                nodePtr->setChild(opNode);
            }

            opStack.resize(opStack.size() - opNum);
            opStack.push_back(nodePtr);

            tempStr.erase();
        } else {
            tempStr += (*iter);
        }

        iter++;
    }

    if (opStack.size() > 1) {
        throw std::runtime_error("buildExpressionTree: Imbalance operand stack !");
    } else if (opStack.empty()) {
        throw std::runtime_error("buildExpressionTree: Unexpected empty stack !");
    }

    return opStack.back();
}

union Op {
    bool b;
    float f;

    Op(): b() {}
    explicit Op(float value_f): f(value_f) {}
    explicit Op(bool value_b): b(value_b) {}
};

static void parseExpressionTree(const std::shared_ptr<ExpTreeNode> &node, std::vector<Op> &stack, int x, int y) {

    if (node->rightSibling) {
        parseExpressionTree(node->rightSibling, stack, x, y);
    }

    if (node->leftChild) {
        parseExpressionTree(node->leftChild, stack, x, y);
    }

    if (isStrNumber(node->opStr)) {
//        Op u{};
//        std::stringstream ss;
//        ss << node->opStr;
//        ss >> u.f;
        Op u(std::stof(node->opStr));
        stack.push_back(u);
    }  else {
        if (node->opStr == "not") {
            Op u(!stack[stack.size() - 1].b);
            stack.resize(stack.size() - 1);
            stack.push_back(u);
        } else if (node->opStr == "and") {
            Op u(stack[stack.size() - 1].b && stack[stack.size() - 2].b);
            stack.resize(stack.size() - 2);
            stack.push_back(u);
        } else if (node->opStr == "or") {
            Op u(stack[stack.size() - 1].b || stack[stack.size() - 2].b);
            stack.resize(stack.size() - 2);
            stack.push_back(u);
        } else if (node->opStr == ">") {
            Op u(stack[stack.size() - 1].f > stack[stack.size() - 2].f);
            stack.resize(stack.size() - 2);
            stack.push_back(u);
        } else if (node->opStr == "<") {
            Op u(stack[stack.size() - 1].f < stack[stack.size() - 2].f);
            stack.resize(stack.size() - 2);
            stack.push_back(u);
        } else if (node->opStr == "<=") {
            Op u(stack[stack.size() - 1].f <= stack[stack.size() - 2].f);
            stack.resize(stack.size() - 2);
            stack.push_back(u);
        } else if (node->opStr == ">=") {
            Op u(stack[stack.size() - 1].f >= stack[stack.size() - 2].f);
            stack.resize(stack.size() - 2);
            stack.push_back(u);
        } else if (node->opStr == "=") {
            Op u(std::fabs(stack[stack.size() - 1].f - stack[stack.size() - 2].f) <= std::numeric_limits<float>::epsilon());
            stack.resize(stack.size() - 2);
            stack.push_back(u);
        } else if (node->opStr == "+") {
            Op u{};
            u.f = stack[stack.size() - 1].f + stack[stack.size() - 2].f;
            stack.resize(stack.size() - 2);
            stack.push_back(u);
        } else if (node->opStr == "-") {
            Op u{};
            u.f = stack[stack.size() - 1].f - stack[stack.size() - 2].f;
            stack.resize(stack.size() - 2);
            stack.push_back(u);
        } else if (node->opStr == "*") {
            Op u{};
            u.f = stack[stack.size() - 1].f * stack[stack.size() - 2].f;
            stack.resize(stack.size() - 2);
            stack.push_back(u);
        } else if (node->opStr == "/") {
            Op u(stack[stack.size() - 1].f / stack[stack.size() - 2].f);
            stack.resize(stack.size() - 2);
            stack.push_back(u);
        } else if (node->opStr == "pow") {
            Op u(std::pow(stack[stack.size() - 1].f, stack[stack.size() - 2].f));
            stack.resize(stack.size() - 2);
            stack.push_back(u);
        } else if (node->opStr == "abs") {
            Op u(std::fabs(stack[stack.size() - 1].f));
            stack.resize(stack.size() - 1);
            stack.push_back(u);
        } else if (node->opStr == "max") {
            Op u(std::fmax(stack[stack.size() - 1].f, stack[stack.size() - 2].f));
            stack.resize(stack.size() - 2);
            stack.push_back(u);
        } else if (node->opStr == "min") {
            Op u(std::fmin(stack[stack.size() - 1].f, stack[stack.size() - 2].f));
            stack.resize(stack.size() - 2);
            stack.push_back(u);
        } else if (node->opStr == "?") {
            Op u{};
            u.f = stack[stack.size() - 1].b ? stack[stack.size() - 2].f : stack[stack.size() - 3].f;
            stack.resize(stack.size() - 3);
            stack.push_back(u);
        } else if (node->opStr == "x") {
            Op u(static_cast<float>(x));
            stack.push_back(u);
        } else if (node->opStr == "y") {
            Op u(static_cast<float>(y));
            stack.push_back(u);
        }
    }
}

float doCalcExpression(const std::shared_ptr<ExpTreeNode> &root, int x, int y) {
    std::vector<Op> stack;

    parseExpressionTree(root, stack, x, y);

    if (stack.size() > 1) {
        throw std::runtime_error("doCalcExpression: Imbalance operand stack !");
    } else if (stack.empty()) {
        throw std::runtime_error("doCalcExpression: Unexpected empty stack !");
    }

    return stack.back().f;
}
