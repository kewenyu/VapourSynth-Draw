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

#ifndef VSDRAW_DRAW_H
#define VSDRAW_DRAW_H

#include <vector>
#include <memory>
#include <string>

union Operand {
    bool b;
    float f;

    Operand(): b() {}
    explicit Operand(float value_f): f(value_f) {}
    explicit Operand(bool value_b): b(value_b) {}
};

enum OpEnum {
    NUM,

    X,
    Y,

    NOT,
    AND,
    OR,
    GT,
    LT,
    GE,
    LE,
    EQ,

    ABS,
    ADD,
    SUB,
    MUL,
    DIV,
    POW,
    MAX,
    MIN,
    MOD,

    TER,
};

struct Operator {
    std::string opStr;
    OpEnum opId;
    int opNum;

    Operator(): opStr(), opNum(0), opId() {}
};

void tokenize(std::string &expStr, std::vector<Operator> &tokens);
void stripRedundantSpace(std::string &str) noexcept;
float parseExpression(const std::vector<Operator> &tokens, int x, int y);

#endif //VSDRAW_DRAW_H
