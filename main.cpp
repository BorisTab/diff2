//#include <iostream>
#include "tree.h"

Node <double> *diffConst(Node <double> *node, Tree <double> *answerTree);

Node <double> *diffNode(Node <double> *node, Tree <double> *answerTree, FILE *tex);

Tree <double> *diffTree(Tree <double> *expression, Tree <double> *answerTree, FILE *tex);

Node <double> *diffAdd(Node <double> *node, Tree <double> *answerTree, FILE *tex);

Node <double> *diffVar(Node <double> *node, Tree <double> *answerTree);

Node <double> *diffMul(Node <double> *node, Tree <double> *answerTree, FILE *tex);

void partDiffForMulDiv(Node <double> *answer, Node <double> *node, Tree <double> *answerTree, FILE *tex);

Node <double> *diffDiv(Node <double> *node, Tree <double> *answerTree, FILE *tex);

Node <double> *diffSin(Node <double> *node, Tree <double> *answerTree);

Node <double> *diffCos(Node <double> *node, Tree <double> *answerTree);

Node <double> *diffDeg(Node <double> *node, Tree <double> *answerTree);

Node <double> *diffLn(Node <double> *node, Tree <double> *answerTree);

void diffNTime(int n, Tree <double> *expression, Tree <double> *answerTree, FILE *tex);

void saveTex(FILE *tex);

void endSaveTex(FILE *tex);

double valueInPoint(Tree <double> *tree, double point);

const int memesN = 5;

const char *memes[memesN] = {
        "Далее просто получем:",
        "Нетрудно заметить, что:",
        "Очевидным переходом получаем:",
        "Абсолютно не думая можно получить следующее:",
        "Тут придется приложить усилее, но решить можно:"
};

int main() {
    FILE *tex = fopen("../diff.tex", "w");
    saveTex(tex);

    int diffOrder = 0;
    double point = 0;
    char answer = 0;
    bool diffPointCheck = false;

    printf("Какую производную брать будем, введите число:\n");
    scanf("%d%c", &diffOrder, &answer);
    printf("Посчитать производную в точке?[y/n]\n");
    scanf("%c", &answer);
    if (answer == 'y') {
        diffPointCheck = true;
        printf("Введите точку x:\n");
        scanf("%lg", &point);
    }

    char startText[200] = "";
    sprintf(startText, "Легким движением руки дифференцируем данную функцию %d раз(а):", diffOrder);

    Tree <double> expression ('P', "../formula.txt");
    expression.saveTreeTex(expression.getRoot(), tex, startText,"f ");

    Tree <double> answerTree = {};
    diffNTime(diffOrder, &expression, &answerTree, tex);
    answerTree.saveTreeTex(answerTree.getRoot(), tex, "Из вышесказанного очевидным образом получаем ответ:","f^\\prime ");

    if (diffPointCheck) {
        double diffPoint = valueInPoint(&answerTree, point);

        printf("\n\n%g", diffPoint);
    }

    endSaveTex(tex);
    return 0;
}

void diffNTime(int n, Tree <double> *expression, Tree <double> *answerTree, FILE *tex) {
    for (int i = 1; i <= n; i++) {
        diffTree(expression, answerTree, tex);

        if (i != n) {
            Node <double> *exRoot = expression->getRoot();
            expression->deleteChildren(exRoot);
            expression->setRoot(answerTree->copySubtree(answerTree->getRoot()));

            delete exRoot;
        }
    }
}

double valueInPoint(Tree <double> *tree, double point) {
    Tree <double> calcTree;
    calcTree.setRoot(tree->copySubtree(tree->getRoot()));

    Node <double> *node = nullptr;
    calcTree.findElem(calcTree.getRoot(), &node, 'x'-'a');

    while (node) {
        if (node->nodeType == VARIABLE) {
            calcTree.changeVal(node, point);
            calcTree.changeType(node, NUMBER);
        }

        calcTree.findElem(calcTree.getRoot(), &node, 'x'-'a');
        node = nullptr;
    }

    calcTree.simplify();
    return calcTree.getRoot()->value;
}

void saveTex(FILE *tex) {
    fprintf(tex, "\\documentclass[12pt,a4paper]{scrartcl}\n"
                 "\\usepackage[utf8]{inputenc}\n"
                 "\\usepackage{resizegather}\n"
                 "\\usepackage[english,russian]{babel}\n"
                 "\\usepackage{indentfirst}\n"
                 "\\usepackage{misccorr}\n"
                 "\\usepackage{graphicx}\n"
                 "\\usepackage{amsmath}\n"
                 "\\begin{document}\n");
}

void endSaveTex(FILE *tex) {
    fprintf(tex, "\\end{document}");
    fclose(tex);
    std::system("pdflatex ../diff.tex");
}

Tree <double> *diffTree(Tree <double> *expression, Tree <double> *answerTree, FILE *tex) {
    assert(expression);
    assert(answerTree);

    answerTree->setRoot(diffNode(expression->getRoot(), answerTree, tex));
    answerTree->simplify();
    return answerTree;
}

Node <double> *diffNode(Node <double> *node, Tree <double> *answerTree, FILE *tex) {
    assert(node);
    assert(answerTree);

    answerTree->saveTreeTex(node, tex, memes[rand() % (memesN)],"g ");

    Node <double> *answer = nullptr;

    if (node->nodeType == OPERATION){

#define DEF_CMD(name, num, sign, code, texCode) \
        if (node->value == num) code \
        else
#include "dsl.h"
#undef DEF_CMD
        {}

        if (node->value == DEG) {
            Node <double> *fullAnswer = answerTree->newNode(MUL, OPERATION);
            fullAnswer->rightChild = answer;

            if (answerTree->findVarInSubtree(node->leftChild) && !answerTree->findVarInSubtree(node->rightChild)) {
                fullAnswer->leftChild = diffNode(node->leftChild, answerTree, tex);
            }

            if (!answerTree->findVarInSubtree(node->leftChild) && answerTree->findVarInSubtree(node->rightChild)) {
                fullAnswer->leftChild = diffNode(node->rightChild, answerTree, tex);
            }

            answerTree->saveTreeTex(fullAnswer, tex, memes[rand() % (memesN)],"g^\\prime ");
            return fullAnswer;
        }

        if (node->value > DEG){
            Node <double> *fullAnswer = answerTree->newNode(MUL, OPERATION);

            fullAnswer->rightChild = answer;
            fullAnswer->leftChild = diffNode(node->rightChild, answerTree, tex);

            answerTree->saveTreeTex(fullAnswer, tex, memes[rand() % (memesN)],"g^\\prime ");
            return fullAnswer;
        }
    }

    else if (node->nodeType == NUMBER) answer =  diffConst(node, answerTree);
    else if (node->nodeType == VARIABLE) answer =  diffVar(node, answerTree);

    answerTree->saveTreeTex(answer, tex, memes[rand() % (memesN)],"g^\\prime ");
    return answer;
}

Node <double> *diffConst(Node <double> *node, Tree <double> *answerTree) {
    assert(node);
    assert(answerTree);

    return answerTree->newNode(0, NUMBER);
}

Node <double> *diffVar(Node <double> *node, Tree <double> *answerTree) {
    assert(node);
    assert(answerTree);

    return answerTree->newNode(1, NUMBER);
}

Node <double> *diffAdd(Node <double> *node, Tree <double> *answerTree, FILE *tex) {
    assert(node);
    assert(answerTree);

    Node <double> *answer = answerTree->newNode(node->value, OPERATION);
    answer->leftChild = diffNode(node->leftChild, answerTree, tex);
    answer->rightChild = diffNode(node->rightChild, answerTree, tex);
    return answer;
}

Node <double> *diffMul(Node <double> *node, Tree <double> *answerTree, FILE *tex) {
    assert(node);
    assert(answerTree);

    Node <double> *answer = answerTree->newNode(ADD, OPERATION);
    partDiffForMulDiv(answer, node, answerTree, tex);
    return answer;
}

Node <double> *diffDiv(Node <double> *node, Tree <double> *answerTree, FILE *tex) {
    assert(node);
    assert(answerTree);

    Node <double> *answer = answerTree->newNode(DIV, OPERATION);

    answer->leftChild = answerTree->newNode(SUB, OPERATION);
    partDiffForMulDiv(answer->leftChild, node, answerTree, tex);

    Node <double> *denom = answerTree->newNode(DEG, OPERATION);
    denom->leftChild = answerTree->copySubtree(node->rightChild);
    denom->rightChild = answerTree->newNode(2, NUMBER);

    answer->rightChild = denom;
    return answer;
}

void partDiffForMulDiv(Node <double> *answer, Node <double> *node, Tree <double> *answerTree, FILE *tex) {
    assert(answer);
    assert(node);
    assert(answerTree);

    Node <double> *leftMul = answer->leftChild = answerTree->newNode(MUL, OPERATION);
    leftMul->leftChild = diffNode(node->leftChild, answerTree, tex);
    leftMul->rightChild = answerTree->copySubtree(node->rightChild);

    Node <double> *rightMul = answer->rightChild = answerTree->newNode(MUL, OPERATION);
    rightMul->leftChild = answerTree->copySubtree(node->leftChild);
    rightMul->rightChild = diffNode(node->rightChild, answerTree, tex);
}

Node <double> *diffSin(Node <double> *node, Tree <double> *answerTree) {
    assert(node);
    assert(answerTree);

    Node <double> *answer = answerTree->newNode(COS, OPERATION);
    Node <double> *rightVal = answerTree->copySubtree(node->rightChild);

    answer->rightChild = rightVal;
    return answer;
}

Node <double> *diffCos(Node <double> *node, Tree <double> *answerTree) {
    assert(node);
    assert(answerTree);

    Node <double> *answer = answerTree->newNode(MUL, OPERATION);
    Node <double> *leftVal = answerTree->newNode(-1, NUMBER);
    Node <double> *rightVal = answerTree->newNode(SIN, OPERATION);
    rightVal->rightChild = answerTree->copySubtree(node->rightChild);

    answer->rightChild = rightVal;
    answer->leftChild = leftVal;
    return answer;
}

Node <double> *diffDeg(Node <double> *node, Tree <double> *answerTree) {
    assert(node);
    assert(answerTree);

    Node <double> *answer = nullptr;

    if (!answerTree->findVarInSubtree(node)) return node;

    if (answerTree->findVarInSubtree(node->leftChild)) {
        if (!answerTree->findVarInSubtree(node->rightChild)) {
            answer = answerTree->newNode(MUL, OPERATION);

            answer->leftChild = answerTree->copySubtree(node->rightChild);
            Node <double> *rightNode = answer->rightChild = answerTree->newNode(DEG, OPERATION);

            rightNode->leftChild = answerTree->copySubtree(node->leftChild);
            Node <double> *subNode = rightNode->rightChild = answerTree->newNode(SUB, OPERATION);

            subNode->leftChild = answerTree->copySubtree(node->rightChild);
            subNode->rightChild = answerTree->newNode(1, NUMBER);
        }
    }

    if (!answerTree->findVarInSubtree(node->leftChild)) {
        if (answerTree->findVarInSubtree(node->rightChild)) {
            answer = answerTree->newNode(MUL, OPERATION);

            answer->leftChild= answerTree->copySubtree(node);
            Node <double> *rightNode = answer->rightChild = answerTree->newNode(LN, OPERATION);

            rightNode->rightChild = answerTree->copySubtree(node->leftChild);
        }
    }
    return answer;
}

Node <double> *diffLn(Node <double> *node, Tree <double> *answerTree) {
    assert(node);
    assert(answerTree);

    Node <double> *answer = answerTree->newNode(DIV, OPERATION);
    Node <double> *leftVal = answerTree->newNode(1, NUMBER);
    Node <double> *rightVal = answerTree->copySubtree(node->rightChild);

    answer->rightChild = rightVal;
    answer->leftChild = leftVal;
    return answer;
}
