//
// Created by boristab on 08.11.2019.
//

#include <iostream>
#include <fstream>
#include <cassert>
#include <cstring>
#include <cmath>
#include "fileRead.h"

enum errors {
    SUCCESS = 0,
    FILE_OPEN_FAILED = 1,
    NOT_FOUND_TREE_IN_FILE = 2,
    WRONG_KEY = 3,
    SYNTAX_ERROR = 4,
};

enum nodeTypes {
    NO_TYPE = 0,
    NUMBER = 1,
    OPERATION = 2,
    VARIABLE = 3,
};

#define DEF_CMD(name, num, sign, code, texCode) \
    name = num,

enum operations {
    NOTHING = 0,
#include "dsl.h"
#undef DEF_CMD
};

#define DEF_CMD(name, num, sign, code, texCode) \
    case num: return sign;

const char *getEnumName(int op) {
    switch (op) {
#include "dsl.h"
#undef DEF_CMD
        default: return nullptr;
    }
}

const char dumpFilePath[FILENAME_MAX] = "../TreeDumpFile.txt";

int spaceN (const char *buffer);

template <class elemType>
class Node {
public:
    Node <elemType> *parent = nullptr;
    Node <elemType> *leftChild = nullptr;
    Node <elemType> *rightChild = nullptr;
    elemType value = {};
    int nodeType = 0;

    explicit Node(elemType value) {
        this->value = value;
    }
};


template <class elemType>
class Tree {
private:
    Node <elemType> *root = nullptr;
    char *s = nullptr;

    void printNodeInit(std::ofstream *dumpFile, Node <elemType> *node) {
        assert(node);
        assert(dumpFile);



        *dumpFile << "node_" << node << " [shape=record, label=\" { "<< node
                  << " | { Val: ";

        if (node->nodeType == VARIABLE) *dumpFile << (char) (node->value + 'a');
        else if (node->nodeType == OPERATION) *dumpFile << getEnumName(node->value);
        else *dumpFile << node->value;

        *dumpFile << " | Type: " << node->nodeType
                  << " } | { left: " << node->leftChild
                  << " | right: " << node->rightChild << " } } \"];\n";

        if (node->leftChild) printNodeInit(dumpFile, node->leftChild);
        if (node->rightChild) printNodeInit(dumpFile, node->rightChild);
    }

    void printNodeRel(std::ofstream *dumpFile, Node <elemType> *node) {
        if (node->leftChild) *dumpFile << "node_" << node << "-- node_" << node->leftChild << ";\n";
        if (node->rightChild) *dumpFile << "node_" << node << "-- node_" << node->rightChild << ";\n";

        if (node->leftChild) printNodeRel(dumpFile, node->leftChild);
        if (node->rightChild) printNodeRel(dumpFile, node->rightChild);
    }

    void saveNode(std::ofstream *outFile, Node <elemType> *node) {
        assert(outFile);
        assert(node);

        *outFile << "{ \"" << node->value << "\" ";

        if (!node->leftChild && !node->rightChild) {
            *outFile << "} ";
            return;
        }

        if (node->leftChild) saveNode(outFile, node->leftChild);
        else *outFile << "$ ";

        if (node->rightChild) saveNode(outFile, node->rightChild);
        else *outFile << "$ ";

        *outFile << "} ";
    }

    void writeNode(char **buffer, Node <elemType> *node) {
        if (**buffer == '$') (*buffer) += 2;
        else if (**buffer == '{'){
            (*buffer) += 2 + spaceN((*buffer) + 1);
            *(strchr(*buffer, '"')) = '\0';

            insertLeft(node, *buffer);
            (*buffer) += strlen(*buffer) + 2;
            writeNode(buffer, node->leftChild);
        }

        if (**buffer == '$') (*buffer) += 2;
        else if (**buffer == '{'){
            (*buffer) += 2 + spaceN((*buffer) + 1);
            *(strchr(*buffer, '"')) = '\0';

            insertRight(node, *buffer);
            (*buffer) += strlen(*buffer) + 2;
            writeNode(buffer, node->rightChild);
        }

        if (**buffer == '}') {
            (*buffer) += 2;
            return;
        }
    }

    void prefixRead(const char *inPath) {
        int fileSize = getFileSize(inPath);

        char *buffer = new char[fileSize] ();
        char *bufferStart = buffer;
        readFile(inPath, buffer, fileSize);

        getG(buffer);
    }

    Node <elemType> *getG(char *str) {
        s = str;
        Node <elemType> *valNode = getE();
        if (*s != '\0') {
            printf("Syntax error: expected end of row\n");
            exit(SYNTAX_ERROR);
        }
        root = valNode;
        return valNode;
    }

    Node <elemType> *getN() {

        if ((*s == '-' && isdigit(*(s + 1))) || isdigit(*s)){
            elemType val = 0;
            int size = 0;

            sscanf(s, "%lf%n", &val, &size);
            s += size;

            Node <elemType> *node = newNode(val);
            node->nodeType = NUMBER;
            return node;
        }

        double val = *s - 'a';
        s++;

        Node <elemType> *node = newNode(val);
        node->nodeType = VARIABLE;
        return node;
    }

    Node <elemType> *getT() {
        Node <elemType> *valLeft = getD();
        Node <elemType> *node = nullptr;
        Node <elemType> *valRight = nullptr;

        while (*s == '*' || *s == '/') {
            char op = *s;
            s++;

            valRight = getD();

            if (op == '*') {
                node = newNode(MUL);
            }else node = newNode(DIV);

            tyingNodes(node, valLeft, valRight);
            node->nodeType = OPERATION;

            valLeft = node;
        }

        return valLeft;
    }

    void tyingNodes(Node <elemType> *node, Node <elemType> *valLeft, Node <elemType> *valRight) {
        node->leftChild = valLeft;
        if (valLeft) node->leftChild->parent = node;

        node->rightChild = valRight;
        node->rightChild->parent = node;
    }

    Node <elemType> *getE() {
        Node <elemType> *valLeft = getT();
        Node <elemType> *node = nullptr;
        Node <elemType> *valRight = nullptr;

        while (*s == '+' || *s == '-') {
            char op = *s;
            s++;

            valRight = getT();

            if (op == '+') {
                node = newNode(ADD);
            } else node = newNode(SUB);

            tyingNodes(node, valLeft, valRight);
            node->nodeType = OPERATION;

            valLeft = node;
        }

        return valLeft;
    }

    Node <elemType> *getP() {
        if (*s == '(') {
            s++;
            Node <elemType> *valNode = getE();
            if (*s != ')') {
                printf("Syntax error: expected ')'\n");
                exit(SYNTAX_ERROR);
            }
            s++;
            return valNode;
        } else return getN();
    }

    Node <elemType> *getD() {
        Node <elemType> *valLeft = getF();
        Node <elemType> *node = nullptr;
        Node <elemType> *valRight = nullptr;

        while (*s == '^') {
            char op = *s;
            s++;

            valRight = getF();
            node = newNode(DEG);
        }

        if (!node) return valLeft;
        tyingNodes(node, valLeft, valRight);
        node->nodeType = OPERATION;

        return node;
    }

    Node <elemType> *getF() {
        Node <elemType> *node = nullptr;
        Node <elemType> *valRight = nullptr;

        if (!isalpha(*s)) {
            return getP();
        }

        char func[10] = "";
        char *funcStart = s;
        while (isalpha(*s)) {
            func[s - funcStart] = *s;
            s++;
        }

        if (*s != '(') {
            s = funcStart;
            return getN();
        }
        s++;

        int val = 0;
#define DEF_CMD(name, num, sign, code, texCode) \
    if (!strcmp(func, sign)) val = num; \
    else
#include "dsl.h"
#undef DEF_CMD
        {
            printf("Syntax error: unknown function %s", func);
            exit(SYNTAX_ERROR);
        }

        valRight = getE();

        if (*s != ')') {
            printf("Syntax error: expected ')' after %s argument", func);
            exit(SYNTAX_ERROR);
        }
        s++;

        node = newNode(val, OPERATION);
        tyingNodes(node, nullptr, valRight);
        return node;
    }

    void nodeToTex(Node <double> *node, FILE *tex) {
        if (!node) return;
        if (node->nodeType == NUMBER) {
            if (node->value < 0) fprintf(tex, "\\left(");
            fprintf(tex, "%g", node->value);
            if (node->value < 0) fprintf(tex, "\\right)");
        }
        else if (node->nodeType == VARIABLE) fprintf(tex, "%c", (char) (node->value + 'a'));
        else if (node->nodeType == OPERATION) {
#define DEF_CMD(name, num, sign, code, texCode) \
            case num: texCode break;

            switch ((int) node->value) {
#include "dsl.h"

                default: printf("Error: unknown function");
            }
#undef DEF_CMD

        }
    }

public:
    explicit Tree(elemType val) {
        auto *node = newNode(val);

        root = node;
    }

    Tree(char type, const char *inPath) {
        if (type == 'P') prefixRead(inPath);

//        else if (type == 'I') infixRead(inPath);

        else {
            printf("Error: Wrong key");
            exit(WRONG_KEY);
        }
    }

    Tree() = default;

    Node <elemType> *newNode(elemType val, elemType type = NOTHING) {
        auto *node = new Node <elemType> (val);
        node->nodeType = type;
        return node;
    }

    void insertNodeLeft(Node <elemType> *parent, Node <elemType> *node) {
        parent->leftChild = node;
    }

    void insertNodeRight(Node <elemType> *parent, Node <elemType> *node) {
        parent->rightChild = node;
    }

    void insertLeft(Node <elemType> *parentNode, elemType val) {
        auto *node = newNode(val);

        parentNode->leftChild = node;
        node->parent = parentNode;
    }

    void insertRight(Node <elemType> *parentNode, elemType val) {
        auto *node = newNode(val);

        parentNode->rightChild = node;
        node->parent = parentNode;
    }

    void deleteChildren(Node <elemType> *node) {
        if (node->leftChild) {
            deleteChildren(node->leftChild);
            delete node->leftChild;
        }
        if (node->rightChild) {
            deleteChildren(node->rightChild);
            delete node->rightChild;
        }

        node->leftChild = nullptr;
        node->rightChild = nullptr;
    }

    Node <elemType> *getRoot() {
        return root;
    }

    Node <elemType> *getLeftChild(Node <elemType> *node) {
        assert(node);

        return node->leftChild;
    }

    Node <elemType> *getRightChild(Node <elemType> *node) {
        assert(node);

        return node->rightChild;
    }

    Node <elemType> getParent(Node <elemType> *node) {
        assert(node);

        return node->parent;
    }

    elemType getVal(Node <elemType> *node) {
        return node->value;
    }

    Node <elemType> *findElem(Node <elemType> *subtree, Node <elemType> **valElem, elemType val) {
        assert(subtree);

        if(subtree->value == val) *valElem =  subtree;

        if (subtree->leftChild) {
            findElem(subtree->leftChild, valElem, val);
        }

        if (subtree->rightChild) {
            findElem(subtree->rightChild, valElem, val);
        }
    }

    void changeVal(Node <elemType> *node, elemType val) {
        node->value = val;
    }

    void changeType(Node <elemType> *node, int type) {
        node->nodeType = type;
    }

    void dump() {
        std::ofstream  dumpFile (dumpFilePath);
        if (!dumpFile) {
            printf("File isn't open\n");
            exit(FILE_OPEN_FAILED);
        }

        dumpFile << "graph G{\n";

        if (root) {
            Node <elemType> *currentNode = root;
            printNodeInit(&dumpFile, root);
            printNodeRel(&dumpFile, root);
        }

        dumpFile << "}\n";

        dumpFile.close();

        char dotCommand[FILENAME_MAX] = "";
        sprintf(dotCommand, "dot -Tpng -O %s", dumpFilePath);
        std::system(dotCommand);
    }

    void saveTo(const char *path) {
        std::ofstream outFile (path);

        if (!outFile) {
            printf("File isn't open\n");
            exit(FILE_OPEN_FAILED);
        }

        saveNode(&outFile, root);
        outFile.close();
    }

    void setRoot(Node <elemType> *node) {
        root = node;
    }

    Node <elemType> *copySubtree(Node <elemType> *node) {
        Node <elemType> *subtree = newNode(node->value, node->nodeType);

        if (node->leftChild) subtree->leftChild = copySubtree(node->leftChild);
        if (node->rightChild) subtree->rightChild = copySubtree(node->rightChild);

        return subtree;
    }

    void saveTreeTex(Node <elemType> *node, FILE *tex, const char *str, const char *funcName) {
        fprintf(tex, "%s\n", str);

        fprintf(tex, "\\begin{gather}\\label{eq:%x}", node);

        fprintf(tex, "%s = ", funcName);
        nodeToTex(node, tex);

        fprintf(tex, "\\end{gather}\n");

    }

    void simplify() {
        size_t simplifyCount = 0;

        simplifyCount += simplifyMul0(root, simplifyCount);

        simplifyCount += simplifyDiv0(root, simplifyCount);

        simplifyCount += simplifyAdd0(&root, simplifyCount);

        simplifyCount += simplifyMul1(&root, simplifyCount);

        simplifyCount += simplifySub0(&root, simplifyCount);

        simplifyCount += simplifyDiv1(&root, simplifyCount);

        simplifyCount += convolutionConst(root, simplifyCount);

        if (simplifyCount) simplify();
    }

    size_t simplifyMul0(Node <elemType> *node, size_t simplifyCount){
        if (node->nodeType == OPERATION && node->value == MUL) {
            bool check = false;
            if (node->leftChild && node->leftChild->nodeType == NUMBER && node->leftChild->value == 0) check = true;
            if (node->rightChild && node->rightChild->nodeType == NUMBER && node->rightChild->value == 0) check = true;

            if (check) {
                deleteChildren(node);
                node->value = 0;
                node->nodeType = NUMBER;
                simplifyCount++;
            }
        }
        if (node->leftChild) simplifyCount += simplifyMul0(node->leftChild, simplifyCount);
        if (node->rightChild) simplifyCount += simplifyMul0(node->rightChild, simplifyCount);

        return simplifyCount;
    }

    size_t simplifyDiv0(Node <elemType> *node, size_t simplifyCount){
        if (node->nodeType == OPERATION && node->value == DIV) {
            if (node->leftChild->nodeType == NUMBER && node->leftChild->value == 0) {
                deleteChildren(node);
                node->value = 0;
                node->nodeType = NUMBER;
                simplifyCount++;
            }
        }
        if (node->leftChild) simplifyCount += simplifyDiv0(node->leftChild, simplifyCount);
        if (node->rightChild) simplifyCount += simplifyDiv0(node->rightChild, simplifyCount);

        return simplifyCount;
    }

    size_t simplifyAdd0(Node <elemType> **nodePointer, size_t simplifyCount) {
        Node <elemType> *node = *nodePointer;

        if (node && node->nodeType == OPERATION && node->value == ADD) {
            if (node->leftChild && node->leftChild->nodeType == NUMBER && node->leftChild->value == 0) {
                delete node->leftChild;
                node->leftChild = nullptr;
                *nodePointer = node->rightChild;
                simplifyCount++;
            }
            else if (node->rightChild && node->rightChild->nodeType == NUMBER && node->rightChild->value == 0) {
                delete node->rightChild;
                node->rightChild = nullptr;
                *nodePointer = node->leftChild;
                simplifyCount++;
            }
        }
        if ((*nodePointer)->leftChild) simplifyCount += simplifyAdd0(&((*nodePointer)->leftChild), simplifyCount);
        if ((*nodePointer)->rightChild) simplifyCount += simplifyAdd0(&((*nodePointer)->rightChild), simplifyCount);

        return simplifyCount;
    }

    size_t simplifySub0(Node <elemType> **nodePointer, size_t simplifyCount) {
        Node <elemType> *node = *nodePointer;

        if (node->nodeType == OPERATION && node->value == SUB) {
            if (node->leftChild && node->leftChild->nodeType == NUMBER && node->leftChild->value == 0) {
                node->value = MUL;
                node->nodeType = OPERATION;
                node->leftChild->value = -1;
                simplifyCount++;
            }
            else if (node->rightChild && node->rightChild->nodeType == NUMBER && node->rightChild->value == 0) {
                delete node->rightChild;
                *nodePointer = node->leftChild;
                simplifyCount++;
            }
        }
        if ((*nodePointer)->leftChild) simplifyCount += simplifySub0(&((*nodePointer)->leftChild), simplifyCount);
        if ((*nodePointer)->rightChild) simplifyCount += simplifySub0(&((*nodePointer)->rightChild), simplifyCount);

        return simplifyCount;
    }

    size_t simplifyMul1(Node <elemType> **nodePointer, size_t simplifyCount) {
        Node <elemType> *node = *nodePointer;

        if (node->nodeType == OPERATION && node->value == MUL) {
            if (node->leftChild && node->leftChild->nodeType == NUMBER && node->leftChild->value == 1) {
                delete node->leftChild;
                *nodePointer = node->rightChild;
                simplifyCount++;
            }
            else if (node->rightChild && node->rightChild->nodeType == NUMBER && node->rightChild->value == 1) {
                delete node->rightChild;
                *nodePointer = node->leftChild;
                simplifyCount++;
            }
        }
        if ((*nodePointer)->leftChild) simplifyCount += simplifyMul1(&((*nodePointer)->leftChild), simplifyCount);
        if ((*nodePointer)->rightChild) simplifyCount += simplifyMul1(&((*nodePointer)->rightChild), simplifyCount);

        return simplifyCount;
    }

    size_t simplifyDiv1(Node <elemType> **nodePointer, size_t simplifyCount) {
        Node <elemType> *node = *nodePointer;

        if (node->nodeType == OPERATION && node->value == DIV) {
            if (node->rightChild && node->rightChild->nodeType == NUMBER && node->rightChild->value == 1) {
                delete node->rightChild;
                *nodePointer = node->leftChild;
                simplifyCount++;
            }
        }
        if (node->leftChild) simplifyCount += simplifyDiv1(&(node->leftChild), simplifyCount);
        if (node->rightChild) simplifyCount += simplifyDiv1(&(node->rightChild), simplifyCount);

        return simplifyCount;
    }

    size_t convolutionConst(Node <elemType> *node, size_t simplifyCount) {
        if (node->leftChild && node->rightChild) {
            if (node->nodeType == OPERATION && node->leftChild->nodeType == NUMBER && node->rightChild->nodeType == NUMBER) {
                node->nodeType = NUMBER;
                double leftNode = node->leftChild->value;
                double rightNode = node->rightChild->value;

                if (node->value == ADD) node->value = leftNode + rightNode;
                else if (node->value == SUB) node->value = leftNode - rightNode;
                else if (node->value == MUL) node->value = leftNode * rightNode;
                else if (node->value == DIV) node->value = leftNode / rightNode;
                else if (node->value == DEG) node->value = pow(leftNode, rightNode);

                deleteChildren(node);
                simplifyCount++;
            }
        }

        if (node->rightChild) {
            if (node->nodeType == OPERATION && node->leftChild == nullptr && node->rightChild->nodeType == NUMBER) {
                node->nodeType = NUMBER;
                double rightNode = node->rightChild->value;

                if (node->value == SIN) node->value = sin(rightNode);
                else if (node->value == COS) node->value = cos(rightNode);
                else if (node->value == LN) node->value = log(rightNode);

                deleteChildren(node);
                simplifyCount++;
            }
        }

        if (node->leftChild) simplifyCount += convolutionConst(node->leftChild, simplifyCount);
        if (node->rightChild) simplifyCount += convolutionConst(node->rightChild, simplifyCount);

        return simplifyCount;
    }

    bool findVarInSubtree(Node <elemType> *node) {
        bool checker = false;
        if (node->nodeType == VARIABLE) return true;

        if (node->leftChild) checker = findVarInSubtree(node->leftChild);
        if (checker) return true;
        if (node->rightChild) checker = findVarInSubtree(node->rightChild);

        return checker;
    }
};

int spaceN (const char *buffer) {
    int count = 0;

    while (*buffer + count == ' ') {
        count++;
    }

    return count;
}
