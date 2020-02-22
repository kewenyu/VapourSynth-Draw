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

static float strToFloat(const std::string &str) {
    std::string::const_iterator iter = str.begin();
    float res = 0.0f;
    bool isNeg = false;

    if (*iter == '-') {
        isNeg = true;
        iter++;
    }

    while (*iter >= '0' && *iter <= '9') {
        res = res * 10.0f + static_cast<float>(*iter - '0');
        iter++;
    }

    if (*iter == '.') {
        float decimal = 0.0f;
        int n = 0;
        iter++;

        while(*iter >= '0' && *iter <= '9') {
            decimal = (decimal * 10.0f) + static_cast<float>(*iter - '0');
            iter++;
            ++n;
        }

        res += decimal / static_cast<float>(std::pow(10.0f, n));
    }

    if (isNeg) {
        res = -res;
    }

    return res;
}

void stripRedundantSpace(std::string &str) noexcept {
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
            std::make_pair("%", 2),

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

static OpEnum getOperandId(const std::string &str) {
    if (isStrNumber(str)) {
        return NUM;
    }

    static const std::unordered_map<std::string, OpEnum> strNumMap {
            // Boolean Operator
            std::make_pair("not", NOT),
            std::make_pair("and", AND),
            std::make_pair("or", OR),
            std::make_pair(">", GT),
            std::make_pair("<", LT),
            std::make_pair(">=", GE),
            std::make_pair("<=", LE),
            std::make_pair("=", EQ),

            // Numeric Operator
            std::make_pair("abs", ABS),
            std::make_pair("+", ADD),
            std::make_pair("-", SUB),
            std::make_pair("*", MUL),
            std::make_pair("/", DIV),
            std::make_pair("pow", POW),
            std::make_pair("max", MAX),
            std::make_pair("min", MIN),
            std::make_pair("%", MOD),

            // Ternary Operator
            std::make_pair("?", TER),

            // Placeholder
            std::make_pair("x", X),
            std::make_pair("y", Y),
    };

    if (strNumMap.find(str) == strNumMap.end()) {
        throw std::runtime_error("getOperandId: Invalid operator: " + str);
    }

    return strNumMap.at(str);
}

void tokenize(std::string &expStr, std::vector<Operator> &tokens) {
    std::string tempStr;

    stripRedundantSpace(expStr);

    for(char iter : expStr) {
        if (iter == ' ') {
            Operator token;
            token.opNum = getOperandNum(tempStr);
            token.opStr = tempStr;
            token.opId = getOperandId(tempStr);

            tokens.push_back(token);
            tempStr.erase();
        } else {
            tempStr += iter;
        }
    }
}

float parseExpression(const std::vector<Operator>& tokens, int x, int y) {
    std::unique_ptr<Operand[]> stack(new Operand[tokens.size()]);
    int stackSize = 0;

    for(const Operator &node : tokens) {
        switch (node.opId) {
            case NOT:
                stack[stackSize - 1].b = !stack[stackSize - 1].b;
                break;
            case AND:
                stack[stackSize - 2].b = stack[stackSize - 2].b && stack[stackSize - 1].b;
                --stackSize;
                break;
            case OR:
                stack[stackSize - 2].b = stack[stackSize - 2].b || stack[stackSize - 1].b;
                --stackSize;
                break;
            case GT:
                stack[stackSize - 2].b = stack[stackSize - 2].f > stack[stackSize - 1].f;
                --stackSize;
                break;
            case LT:
                stack[stackSize - 2].b = stack[stackSize - 2].f < stack[stackSize - 1].f;
                --stackSize;
                break;
            case GE:
                stack[stackSize - 2].b = stack[stackSize - 2].f >= stack[stackSize - 1].f;
                --stackSize;
                break;
            case LE:
                stack[stackSize - 2].b = stack[stackSize - 2].f <= stack[stackSize - 1].f;
                --stackSize;
                break;
            case EQ:
                stack[stackSize - 2].b = std::fabs(stack[stackSize - 2].f - stack[stackSize - 1].f) <= std::numeric_limits<float>::epsilon();
                --stackSize;
                break;
            case ADD:
                stack[stackSize - 2].f = stack[stackSize - 2].f + stack[stackSize - 1].f;
                --stackSize;
                break;
            case SUB:
                stack[stackSize - 2].f = stack[stackSize - 2].f - stack[stackSize - 1].f;
                --stackSize;
                break;
            case MUL:
                stack[stackSize - 2].f = stack[stackSize - 2].f * stack[stackSize - 1].f;
                --stackSize;
                break;
            case DIV:
                stack[stackSize - 2].f = stack[stackSize - 2].f / stack[stackSize - 1].f;
                --stackSize;
                break;
            case POW:
                stack[stackSize - 2].f = std::pow(stack[stackSize - 2].f ,stack[stackSize - 1].f);
                --stackSize;
                break;
            case ABS:
                stack[stackSize].f = std::fabs(stack[stackSize].f);
                break;
            case MAX:
                stack[stackSize - 2].f = std::fmax(stack[stackSize - 2].f, stack[stackSize - 1].f);
                --stackSize;
                break;
            case MIN:
                stack[stackSize - 2].f = std::fmin(stack[stackSize - 2].f, stack[stackSize - 1].f);
                --stackSize;
                break;
            case MOD:
                stack[stackSize - 2].f = std::fmod(stack[stackSize - 2].f, stack[stackSize - 1].f);
                --stackSize;
                break;
            case TER:
                stack[stackSize - 3].f = stack[stackSize - 3].b ? stack[stackSize - 2].f : stack[stackSize - 1].f;
                stackSize -= 2;
                break;
            case X:
                stack[stackSize].f = static_cast<float>(x);
                ++stackSize;
                break;
            case Y:
                stack[stackSize].f = static_cast<float>(y);
                ++stackSize;
                break;
            case NUM:
                stack[stackSize].f = strToFloat(node.opStr);
                ++stackSize;
                break;
        }
    }

    return stack[0].f;
}
