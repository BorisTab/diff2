//
// Created by boristab on 26.11.2019.
//

DEF_CMD(ADD, 1, "+", {
    answer =  diffAdd(node, answerTree, tex);
}, {
    nodeToTex(node->leftChild, tex);
    fprintf(tex, " + ");
    nodeToTex(node->rightChild, tex);
})

DEF_CMD(SUB, 2, "-", {
    answer = diffAdd(node, answerTree, tex);
}, {
    nodeToTex(node->leftChild, tex);
    fprintf(tex, " - ");
    nodeToTex(node->rightChild, tex);
})

DEF_CMD(MUL, 3, "*", {
    answer = diffMul(node, answerTree, tex);
}, {
    if (node->leftChild->nodeType == NUMBER && node->leftChild->value == -1) {
        fprintf(tex, " \\left(-");
        nodeToTex(node->rightChild, tex);
        fprintf(tex, "\\right) ");
    } else {
        if (node->leftChild->nodeType == OPERATION && node->leftChild->value < MUL) fprintf(tex, "\\left(");
        nodeToTex(node->leftChild, tex);
        if (node->leftChild->nodeType == OPERATION && node->leftChild->value < MUL) fprintf(tex, "\\right)");

        fprintf(tex, " \\cdot ");

        if (node->rightChild->nodeType == OPERATION && node->rightChild->value < MUL) fprintf(tex, "\\left(");
        nodeToTex(node->rightChild, tex);
        if (node->rightChild->nodeType == OPERATION && node->rightChild->value < MUL) fprintf(tex, "\\right)");
    }
})

DEF_CMD(DIV, 4, "/", {
    answer = diffDiv(node, answerTree, tex);
}, {
    fprintf(tex, "\\frac{");
    nodeToTex(node->leftChild, tex);
    fprintf(tex, "}{");
    nodeToTex(node->rightChild, tex);
    fprintf(tex, "}");
})

DEF_CMD(DEG, 5, "^", {
    answer = diffDeg(node, answerTree);
    }, {
    if (node->rightChild->nodeType == NUMBER && node->rightChild->value == 1) nodeToTex(node->leftChild, tex);
    else {
        if (node->leftChild->nodeType == OPERATION) fprintf(tex, "{ \\left(");
        nodeToTex(node->leftChild, tex);
        if (node->leftChild->nodeType == OPERATION) fprintf(tex, "\\right) }");
        fprintf(tex, " ^ {");
        nodeToTex(node->rightChild, tex);
        fprintf(tex, "}");
    }
})

DEF_CMD(SIN, 6, "sin", {
    answer = diffSin(node, answerTree);
}, {
    fprintf(tex, " \\sin{ ");
    if (node->rightChild->nodeType == OPERATION) fprintf(tex, "\\left(");
    nodeToTex(node->rightChild, tex);
    if (node->rightChild->nodeType == OPERATION) fprintf(tex, "\\right)");
    fprintf(tex, " } ");
})

DEF_CMD(COS, 7, "cos", {
    answer = diffCos(node, answerTree);
    }, {
    fprintf(tex, " \\cos{ ");
    if (node->rightChild->nodeType == OPERATION) fprintf(tex, "\\left(");
    nodeToTex(node->rightChild, tex);
    if (node->rightChild->nodeType == OPERATION) fprintf(tex, "\\right)");
    fprintf(tex, " } ");
})

DEF_CMD(LN, 8, "ln", {
    answer = diffLn(node, answerTree);
    }, {
fprintf(tex, " \\ln{ ");
    if (node->rightChild->nodeType == OPERATION) fprintf(tex, "\\left(");
    nodeToTex(node->rightChild, tex);
    if (node->rightChild->nodeType == OPERATION) fprintf(tex, "\\right)");
    fprintf(tex, " } ");
})
