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

struct ExpTreeNode {
    std::string opStr;
    int opNum;

    std::shared_ptr<ExpTreeNode> leftChild;
    std::shared_ptr<ExpTreeNode> rightSibling;

    ExpTreeNode(): opStr(), opNum(0), leftChild(), rightSibling() {}

    void setSibling(const std::shared_ptr<ExpTreeNode>& child) {
        if (this->rightSibling) {
            this->rightSibling->setSibling(child);
        } else {
            this->rightSibling = child;
        }
    }

    void setChild(const std::shared_ptr<ExpTreeNode>& child) {
        if (this->leftChild) {
            if (this->leftChild->rightSibling) {
                this->leftChild->rightSibling->setSibling(child);
            } else {
                this->leftChild->rightSibling = child;
            }
        } else {
            this->leftChild = child;
        }
    }
};

std::shared_ptr<ExpTreeNode> buildExpressionTree(std::string &expStr);
float doCalcExpression(const std::shared_ptr<ExpTreeNode> &root, int x, int y);

#endif //VSDRAW_DRAW_H
