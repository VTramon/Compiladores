#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <tchar.h>
#include <windows.h>

using namespace std;

// Definição dos tokens
enum Token {
    T_INTEIRO, T_REAL,
    T_SE, T_ENTAO, T_SENAO,
    T_ENQUANTO,
    T_REPITA, T_ATE,
    T_LER,
    T_MOSTRAR,
    T_IGUAL,
    T_ID,
    T_NUM_INTEIRO, T_NUM_REAL,
    T_OU, T_E,
    T_MAIOR, T_MAIOR_IGUAL, T_MENOR, T_MENOR_IGUAL, T_IGUAL_IGUAL, T_DIFERENTE,
    T_MAIS, T_MENOS, T_MULT, T_DIV,
    T_ABRE_CHAVES, T_FECHA_CHAVES, T_ABRE_PARENTESES, T_FECHA_PARENTESES,
    T_PONTO_VIRGULA, T_VIRGULA,
    T_UNKNOWN,
    T_EOF
};


struct TokenValue {
    Token token;
    string lexema;
    double value;
};

// Tabela de símbolos: armazena o tipo de cada variável
unordered_map<string, string> tabelaDeSimbolos;  // variável -> tipo

// Estrutura para a árvore sintática
struct ArvoreNode {
    string type;
    string value;
    string semantico;
    vector<ArvoreNode*> children;

    ArvoreNode(string t, string v = "", string s = "") : type(t), value(v), semantico(s) {}
};


ArvoreNode* Programa();
ArvoreNode* Declaracao();
ArvoreNode* Tipo();
ArvoreNode* IdLista(string tipo);
ArvoreNode* Id(string escopo);
ArvoreNode* Corpo();
ArvoreNode* Comando();
ArvoreNode* Atribuicao();
ArvoreNode* Repeticao();
ArvoreNode* Enquanto();
ArvoreNode* Condicao();
ArvoreNode* Mostrar();
ArvoreNode* Ler();
ArvoreNode* Expressao(string tipo, string tipoParent);
ArvoreNode* ExpressaoLogica();
ArvoreNode* ExpressaoRelacional();
ArvoreNode* ExpressaoAritimetica();
void nextStep();


TokenValue getNextToken();
void printToken(TokenValue tok);

string input;
size_t posicao = 0;
TokenValue currentToken;
ofstream outputSintFile;
ofstream outputSemFile;
ofstream outputASFile;

// Avança para o próximo token
void nextStep() {
    currentToken = getNextToken();
}

// Função de erro para imprimir mensagem e finalizar o programa quando a sintaxe está incorreta
void error(string msg) {
    cout << "Erro: " << msg << " na posição " << posicao << endl;
    exit(1);
}

// Verifica se o token atual é o esperado e avança se verdadeiro
void match(Token expectedToken) {
    if (currentToken.token == expectedToken) {
        nextStep();
    }
    else {
        error("Token inesperado: " + currentToken.lexema);
    }
}

// Função para imprimir a árvore sintática no arquivo de saída
void printArvoreSintatica(ArvoreNode* node, int depth = 0) {
    for (int i = 0; i < depth; ++i) {
        outputSintFile << "  ";
    }

    outputSintFile << node->type;

    if (!node->value.empty()) {
        outputSintFile << " (" << node->value << ")";
    }

    outputSintFile << endl;

    for (ArvoreNode* child : node->children) {
        printArvoreSintatica(child, depth + 1);
    }
}

bool firstComando = false;
void printArvoreSemantica(ArvoreNode* node, int depth = 0, string atrib = "") {
    // Exibe a árvore com informações de tipo e valor

    if (node->type == "Comando") {
        if (firstComando == false) {
            firstComando = true;
            for (ArvoreNode* child : node->children) {
                printArvoreSemantica(child, depth + 1);
            }
        }
        else {
            for (size_t i = 0; i < node->children.size(); i++)
            {
                printArvoreSemantica(node->children[i], depth + 1 + i);
            }
        }
    }else
    if (node->type == "Expressao") {
        if (atrib == "ate") {
            for (ArvoreNode* child : node->children) {
                if (node->children[0] == child) {
                    printArvoreSemantica(child, depth + 1, atrib);
                }
                else {
                    printArvoreSemantica(child, depth + 1);
                }
            }
        }
        else {
            for (ArvoreNode* child : node->children) {
                printArvoreSemantica(child, depth + 1);
            }
        }
    }
    else {

        if (node->type == "Tipo") {
            outputSemFile << endl;
            for (int i = 0; i < depth; ++i) {
                outputSemFile << "  ";
            }
            outputSemFile << node->value << " ";
            for (ArvoreNode* child : node->children) {
                printArvoreSemantica(child, depth + 1);
            }
        }
        else if (node->type == "IdLista") {
            for (ArvoreNode* child : node->children) {
                printArvoreSemantica(child, depth + 1);
            }
        }
        else if (node->type == "ID" || node->type == "INTEIRO" || node->type == "REAL" || node->value == "==" ||
            node->value == "+" || node->value == "-" || node->value == "*" || node->value == "/" || node->value == "<" ||
            node->value == "<=" || node->value == ">" || node->value == ">=" || node->value == "!=") {
            if (atrib == "ate") {
            //outputFile << endl << node->type << " " << node->value ;
                outputSemFile << endl;
                for (int i = 0; i < depth; ++i) {
                    outputSemFile << "  ";
                }
                outputSemFile << node->value;
                //outputFile << endl << endl << endl << endl;
            }else
            if (atrib == "=") {
                for (int i = 0; i < depth; ++i) {
                    outputSemFile << "  ";
                }
                outputSemFile << node->value << " =";
            }
            else if (node->type == "INTEIRO") {
                outputSemFile << " " << (int) stod(node->value);
            }
            else if (node->type == "REAL") {
                outputSemFile << " " << nextafter(stod(node->value), 0.00);
            }
            else {
                outputSemFile <<  " " << node->value;
            }
        }
        else if (node->type == "Atribuição") {
            outputSemFile << endl;

            //for (int i = 0; i < depth; ++i) {
            //    outputFile << "  ";
            //}
            //outputFile << node->type;
            //outputFile << endl;

            for (ArvoreNode* child : node->children) {
                printArvoreSemantica(child, depth + 1, "=");
            }
        }
        else {
            outputSemFile << endl;

            for (int i = 0; i < depth; ++i) {
                outputSemFile << "  ";
            }
            outputSemFile << node->type;
            if (!node->value.empty()) {
                outputSemFile << " (" << node->value << ")";
            }

            if (node->type == "Repeticao" || node->type == "Enquanto") {
                for (ArvoreNode* child : node->children) {

                    if ((node->type == "Repeticao" && node->children[1] == child) || 
                        (node->type == "Enquanto" && node->children[0] == child)) {
                        //outputFile << "endl";
                        //for (int i = 0; i < (depth + 2); ++i) {
                        //    outputFile << "  ";
                        //}
                        printArvoreSemantica(child, depth + 1, "ate");
                    }
                    else {
                        printArvoreSemantica(child, depth + 1);
                    }
                }
            }
            else {
                for (ArvoreNode* child : node->children) {
                    printArvoreSemantica(child, depth + 1);
                }
            }

        }


        // Adiciona o tipo de dado do nó se disponível
        //if (!node->children.empty() && node->type != "Corpo") {
        //    outputFile << ": " << node->children[0]->type;
        //}



    }

}


void printTabelaDeSimbolos() {
    for (auto value : tabelaDeSimbolos) {
        outputASFile << value.first << ": " << value.second << endl;
    }
}
//void printArvore(ArvoreNode* node, int depth = 0) {
//    for (int i = 0; i < depth; ++i) {
//        outputFile << "  ";
//    }
//
//    outputFile << node->type;
//
//    if (!node->value.empty()) {
//        outputFile << " (" << node->value << ")";
//    }
//
//    outputFile << endl;
//
//    for (ArvoreNode* child : node->children) {
//        printArvore(child, depth + 1);
//    }
//}

// Função para verificar se uma variável foi declarada
void verificarDeclaracao(string id) {
    if (tabelaDeSimbolos.find(id) == tabelaDeSimbolos.end()) error("Variável '" + id + "' não foi declarada.");
}

// Função para verificar se uma variável já foi declarada
void verificarRedeclaracao(string id) {
    if (tabelaDeSimbolos.find(id) != tabelaDeSimbolos.end()) error("Variável '" + id + "' já foi declarada.");
}

// Função para verificar o tipo da expressão na atribuição
void verificarAtribuicao(string tipoVar, ArvoreNode* expressao) {
    string tipoExpr = expressao->type;

    if (tipoExpr == "Expressao") {
        tipoExpr = expressao->children[0]->type;
    }

    if (tipoVar == "INTEIRO" && tipoExpr != "INTEIRO" && tipoExpr != "ID") {
        error("Tipo incompatível: Esperado inteiro na expressão.");
    }
    //cout << '\n\n' << tipoVar << '\n'<<'\n';
    if (tipoVar == "REAL" && (tipoExpr != "REAL" && tipoExpr != "ID")) {
        error("Tipo incompatível: Esperado real ou inteiro na expressão. " + tipoExpr + " --");
    }
}


void verificarLerMostrar(string lex) {
    verificarDeclaracao(lex);

    //if (lex == "desconhecido") {
    //    error("Tipo incompatível: Esperado real ou inteiro na função.");
    //}
    //cout << endl << tabelaDeSimbolos.at(lex) << endl << endl;

    //cout << tipo << '\n';
    if (tabelaDeSimbolos.at(lex) != "inteiro" && tabelaDeSimbolos.at(lex) != "real") {
        error("Tipo incompatível: Esperado real ou inteiro na função 'ler()'.");
    }

}

bool verificarBooleano(string operador) {
    //cout << operador << '\n';
    return operador == "<" || operador == ">" || operador == "<=" || operador == ">=" || operador == "==" || operador == "!=";
}


Token verificarExpressaoEritimetica(ArvoreNode* esquerda, string operador, ArvoreNode* direita) {
    string tipoEsq = esquerda->type == "ID" ? tabelaDeSimbolos[esquerda->value] : esquerda->type;
    string tipoDir = direita->type == "ID" ? tabelaDeSimbolos[direita->value] : direita->type;

    //cout << '\n' << node->type << " => " << node->value << '\n' << '\n';
    //cout << '\n' << tipoEsq << " => " << esquerda[1] << '\n' << tipoDir << " => " << direita[1] << '\n' << '\n';

    if (operador == "+" || operador == "-" || operador == "/" || operador == "*") {
        if ((tipoEsq == "inteiro" || tipoEsq == "real" || tipoEsq == "INTEIRO" || tipoEsq == "REAL") &&
            (tipoDir == "inteiro" || tipoDir == "real" || tipoDir == "INTEIRO" || tipoDir == "REAL")) {
            if (operador == "/" && direita->value == "0") {
                error("Erro: divisão por zero.");
                return T_UNKNOWN;
            }
            if (((tipoEsq == "inteiro" || tipoEsq == "INTEIRO") && (tipoDir == "real" || tipoDir == "REAL")) || 
                (tipoEsq == "real" || tipoEsq == "REAL") && (tipoDir == "inteiro" || tipoDir == "INTEIRO")) {
                esquerda->type == "ID" ? tabelaDeSimbolos[esquerda->value] : esquerda->value;
                return T_REAL;
            }
            return T_INTEIRO;
        }
        else {
            error("Tipos incompativeis");
            return T_UNKNOWN;
        }
    }
}


ArvoreNode* Programa() {
    ArvoreNode* node = new ArvoreNode("Programa");

    // Primeira parte → Seção onde são declaradas as variáveis do programa.
    node->children.push_back(Declaracao());

    // Segunda parte → Corpo do programa, onde são colocados os comandos que serão executados.
    node->children.push_back(Corpo());

    if (currentToken.token != T_EOF) {
        error("Esperado EOF no final do programa");
    }
    return node;
}

ArvoreNode* Declaracao() {
    ArvoreNode* node = new ArvoreNode("Decl");
    //cout << '\n' << node->children.size() << '\n' << '\n';

    if (currentToken.token != T_INTEIRO && currentToken.token != T_REAL) {
        //cout << to_string(currentToken.token);
        error("Esperado declaração de tipo (inteiro ou real)");
    }
    while (currentToken.token == T_INTEIRO || currentToken.token == T_REAL) {
        //if (!node->children.empty()) {
        //    node->children.push_back(Tipo());
        //    for (size_t i = 1; i < node->children.size(); i++)
        //    {
        //        //if (node->children[i + 1]) {
        //            //cout << '\n' << node->children.size() << '\n' << '\n';
        //            //node->children[i]->siblings.push_back(node->children[i + 1]);
        //        //}
        //    }
        //}
        //else
        //{
            node->children.push_back(Tipo());

            string tipoDeclaracao = currentToken.lexema;
            //}
        //for (size_t j = 0; j < node->children.size(); j++)
        //    cout << '\n' << node->children[j]->siblings[0]->value << '\n' << '\n';
        nextStep();

        node->children.push_back(IdLista(tipoDeclaracao));
        match(T_PONTO_VIRGULA);
    }

        /*if (currentToken.token == T_INTEIRO || currentToken.token == T_REAL) {
            node->children.push_back(Declaracao());
        }*/


    return node;
}

ArvoreNode* Tipo() {
    ArvoreNode* node = new ArvoreNode("Tipo", currentToken.token == T_INTEIRO ? "inteiro" : "real");

    return node;
}

ArvoreNode* IdLista(string tipo) {
    ArvoreNode* node = new ArvoreNode("IdLista");

    node->children.push_back(Id(tipo));

    while (currentToken.token == T_VIRGULA) {
        nextStep();
        node->children.push_back(Id(tipo));
    }

    return node;
}

ArvoreNode* Id(string tipo = "") {
    ArvoreNode* node = new ArvoreNode("ID", currentToken.lexema);

    if (tipo == "inteiro" || tipo == "real") {
        // Verifica se a variável já foi declarada
        verificarRedeclaracao(currentToken.lexema);
        tabelaDeSimbolos[currentToken.lexema] = tipo == "inteiro" ? "inteiro" : "real";
    }
    //cout << endl << tabelaDeSimbolos[currentToken.lexema] << endl << endl;
    match(T_ID);


    return node;
}

ArvoreNode* Corpo() {
    ArvoreNode* node = new ArvoreNode("Corpo");

    if (currentToken.token == T_ID || currentToken.token == T_REPITA ||
        currentToken.token == T_MOSTRAR || currentToken.token == T_ENQUANTO ||
        currentToken.token == T_SE || currentToken.token == T_LER) {
        node->children.push_back(Comando());
    }
    else {
        error("Esperado corpo do programa");
    }

    return node;
}


ArvoreNode* Comando() {
    ArvoreNode* node = new ArvoreNode("Comando");

    while (currentToken.token == T_ID || currentToken.token == T_REPITA ||
        currentToken.token == T_MOSTRAR || currentToken.token == T_ENQUANTO ||
        currentToken.token == T_SE || currentToken.token == T_LER) {
        
        if (currentToken.token == T_ID) {
            node->children.push_back(Atribuicao());
        }
        else if (currentToken.token == T_REPITA) {
            node->children.push_back(Repeticao());
        }
        else if (currentToken.token == T_ENQUANTO) {
            node->children.push_back(Enquanto());
        }
        else if (currentToken.token == T_SE) {
            node->children.push_back(Condicao());
        }
        else if (currentToken.token == T_MOSTRAR) {
            node->children.push_back(Mostrar());
        }
        else if (currentToken.token == T_LER) {
            node->children.push_back(Ler());
        }
        else {
            error("Comando inválido");
        }
    }

    return node;
}

ArvoreNode* Atribuicao() {
    ArvoreNode* node = new ArvoreNode("Atribuição");

    node->children.push_back(Id());

    match(T_IGUAL);

    // Verificar o tipo da variável e o tipo da expressão à direita
    string tipoVar = tabelaDeSimbolos[node->children[0]->value];

    node->children.push_back(Expressao("Atribuicao", tipoVar));
    match(T_PONTO_VIRGULA);

    verificarAtribuicao(tipoVar, node->children[1]);

    return node;
}

ArvoreNode* Repeticao() {
    ArvoreNode* node = new ArvoreNode("Repeticao");
    match(T_REPITA);

    if (currentToken.token == T_ABRE_CHAVES) {
        match(T_ABRE_CHAVES);
        node->children.push_back(Comando());
        match(T_FECHA_CHAVES);
    }
    else {
        node->children.push_back(Comando());
    }
    match(T_ATE);
    node->children.push_back(Expressao("booleano", ""));
    match(T_PONTO_VIRGULA);

    return node;
}

ArvoreNode* Enquanto() {
    ArvoreNode* node = new ArvoreNode("Enquanto");
    match(T_ENQUANTO);
    match(T_ABRE_PARENTESES);

    node->children.push_back(Expressao("booleano",""));

    match(T_FECHA_PARENTESES);

    //verificarExpressaoBooleana(node->children[0]);

    if (currentToken.token == T_ABRE_CHAVES) {
        match(T_ABRE_CHAVES);
        node->children.push_back(Comando());
        match(T_FECHA_CHAVES);
    }
    else {
        node->children.push_back(Comando());
    }

    return node;
}

ArvoreNode* Condicao() {
    ArvoreNode* node = new ArvoreNode("Condicao");

    match(T_SE);
    node->children.push_back(Expressao("booleano", ""));
    match(T_ENTAO);

    if (currentToken.token == T_ABRE_CHAVES) {
        match(T_ABRE_CHAVES);
        node->children.push_back(Comando());
        match(T_FECHA_CHAVES);
    }
    else {
        node->children.push_back(Comando());
    }

    if (currentToken.token == T_SENAO) {
        nextStep();
        if (currentToken.token == T_ABRE_CHAVES) {
            match(T_ABRE_CHAVES);
            node->children.push_back(Comando());
            match(T_FECHA_CHAVES);
        }
        else {
            node->children.push_back(Comando());
        }
    }
    return node;

}

ArvoreNode* Mostrar() {
    ArvoreNode* node = new ArvoreNode("Mostrar");
    match(T_MOSTRAR);
    match(T_ABRE_PARENTESES);
    verificarLerMostrar(currentToken.lexema);
    node->children.push_back(Expressao("", ""));
    match(T_FECHA_PARENTESES);
    match(T_PONTO_VIRGULA);
    return node;
}

ArvoreNode* Ler() {
    ArvoreNode* node = new ArvoreNode("Ler");
    match(T_LER);
    match(T_ABRE_PARENTESES);
    verificarLerMostrar(currentToken.lexema);
    node->children.push_back(new ArvoreNode("ID", currentToken.lexema));
    match(T_ID);
    match(T_FECHA_PARENTESES);
    match(T_PONTO_VIRGULA);
    return node;
}


ArvoreNode* Expressao(string tipo, string tipoParent = "") {
    ArvoreNode* node = new ArvoreNode("Expressao");

    if (currentToken.token == T_ID || currentToken.token == T_NUM_INTEIRO || currentToken.token == T_NUM_REAL ||
        currentToken.token == T_MAIS || currentToken.token == T_MENOS ||
        currentToken.token == T_MULT || currentToken.token == T_DIV ||
        currentToken.token == T_OU || currentToken.token == T_E ||
        currentToken.token == T_MAIOR || currentToken.token == T_MAIOR_IGUAL ||
        currentToken.token == T_MENOR || currentToken.token == T_MENOR_IGUAL ||
        currentToken.token == T_IGUAL_IGUAL || currentToken.token == T_DIFERENTE) {
        
        if (currentToken.token == T_ID) {
            string tipoVar = tabelaDeSimbolos[currentToken.lexema];
            if (tipoVar.empty()) {
                error("Variável não declarada: " + currentToken.lexema);
            }

            node->children.push_back(new ArvoreNode("ID", currentToken.lexema));
            /*node->children[0]->type*/
            match(T_ID);
        }
        else if (currentToken.token == T_NUM_INTEIRO ) {
            node->children.push_back(new ArvoreNode("INTEIRO", to_string(currentToken.value)));
            match(T_NUM_INTEIRO);
        }
        else if (currentToken.token == T_NUM_REAL) {
            node->children.push_back(new ArvoreNode("REAL", to_string(currentToken.value)));
            match(T_NUM_REAL);
        }


        if (currentToken.token == T_MAIS || currentToken.token == T_MENOS ||
            currentToken.token == T_MULT || currentToken.token == T_DIV) {
            node->children.push_back(ExpressaoAritimetica());
        }
        else if (currentToken.token == T_OU || currentToken.token == T_E) {
            node->children.push_back(ExpressaoLogica());
        }
        else if (currentToken.token == T_MAIOR || currentToken.token == T_MAIOR_IGUAL ||
            currentToken.token == T_MENOR || currentToken.token == T_MENOR_IGUAL ||
            currentToken.token == T_IGUAL_IGUAL || currentToken.token == T_DIFERENTE) {

            verificarBooleano(currentToken.lexema);

            node->children.push_back(ExpressaoRelacional());
        }



        if (currentToken.token == T_ID) {
            string tipoVar = tabelaDeSimbolos[currentToken.lexema];
            if (tipoVar.empty()) {
                error("Variável não declarada: " + currentToken.lexema);
            }

            node->children.push_back(new ArvoreNode("ID", currentToken.lexema));
            match(T_ID);
        }
        else if (currentToken.token == T_NUM_INTEIRO) {
            node->children.push_back(new ArvoreNode("INTEIRO", to_string(currentToken.value)));
            match(T_NUM_INTEIRO);
        }
        else if (currentToken.token == T_NUM_REAL) {
            node->children.push_back(new ArvoreNode("REAL", to_string(currentToken.value)));
            match(T_NUM_REAL);
        }


        if (node->children.size() == 3 && tipo != "booleano") {
            Token verificaTipo = verificarExpressaoEritimetica(node->children[0], node->children[1]->value, node->children[2]);
            
            //cout << endl << tipoParent << " || " << verificaTipo << endl << endl;

            if (tipo == "Atribuicao" && 
                ((verificaTipo == T_INTEIRO && (tipoParent == "REAL" || tipoParent == "real")) || (verificaTipo == T_REAL && (tipoParent == "INTEIRO" || tipoParent == "inteiro")))) {
                error("Atribuição de tipos incompativeis");
            }
            //if (tipo == "Atribuicao" && (node->children[1]->type == "MENOS" || node->children[1]->type == "MAIS" 
            //    || node->children[1]->type == "MULT" || node->children[1]->type == "DIV")) {
            //    cout << endl << tabelaDeSimbolos[node->children[1]->value] << endl << endl;
            //    
            //}
        }

        //if (tipo == "booleano") {
            /*if (!verificarBooleano((LPCWSTR) _T(input[posicao]))) {
                error("Expressões contidas em funções de repetição e condição devem ser booleanas.");
            }*/
        //}
    }
    else {
        error("Expressão inválida");
    }

    return node;
}

ArvoreNode* ExpressaoRelacional() {
    if (currentToken.token == T_MAIOR) {
        ArvoreNode* node = new ArvoreNode("MAIOR", currentToken.lexema);
        match(T_MAIOR);
        return node;
    }
    else if (currentToken.token == T_MAIOR_IGUAL) {
        ArvoreNode* node = new ArvoreNode("MAIOR IGUAL", currentToken.lexema);
        match(T_MAIOR);
        return node;
    }
    else if (currentToken.token == T_MENOR) {
        ArvoreNode* node = new ArvoreNode("MENOR", currentToken.lexema);
        match(T_MENOR);
        return node;
    }
    else if (currentToken.token == T_MENOR_IGUAL) {
        ArvoreNode* node = new ArvoreNode("MENOR IGUAL", currentToken.lexema);
        match(T_MENOR_IGUAL);
        return node;
    }
    else if (currentToken.token == T_IGUAL_IGUAL) {
        ArvoreNode* node = new ArvoreNode("IGUAL IGUAL", currentToken.lexema);
        match(T_IGUAL_IGUAL);
        return node;
    }
    else if (currentToken.token == T_DIFERENTE) {
        ArvoreNode* node = new ArvoreNode("DIFERENTE", currentToken.lexema);
        match(T_DIFERENTE);
        return node;
    }
}

ArvoreNode* ExpressaoLogica() {
    if (currentToken.token == T_OU) {
        ArvoreNode* node = new ArvoreNode("OU", currentToken.lexema);
        match(T_OU);
        return node;
    }
    else if (currentToken.token == T_E) {
        ArvoreNode* node = new ArvoreNode("E", currentToken.lexema);
        match(T_E);
        return node;
    }
}

ArvoreNode* ExpressaoAritimetica() {

    if (currentToken.token == T_MAIS) {
        ArvoreNode* node = new ArvoreNode("MAIS", currentToken.lexema);
        match(T_MAIS);
        return node;
    }
    else if (currentToken.token == T_MENOS) {
        ArvoreNode* node = new ArvoreNode("MENOS", currentToken.lexema);
        match(T_MENOS);
        return node;
    }
    else if (currentToken.token == T_MULT) {
        ArvoreNode* node = new ArvoreNode("MULT", currentToken.lexema);
        match(T_MULT);
        return node;
    }
    else if (currentToken.token == T_DIV) {
        ArvoreNode* node = new ArvoreNode("DIV", currentToken.lexema);
        match(T_DIV);
        return node;
    }
}

TokenValue getNextToken() {
    TokenValue tok;

    // ignora espaços em branco
    while (isspace(input[posicao])) posicao++;

    if (posicao >= input.size()) {
        tok.token = T_EOF;
        printToken(tok);
        return tok;
    }

    if (isalpha(input[posicao])) {
        tok.lexema = "";
        while (isalpha(input[posicao])) {
            tok.lexema += input[posicao];
            posicao++;
        }
        if (tok.lexema == "inteiro") {
            tok.token = T_INTEIRO;
            printToken(tok);
            return { T_INTEIRO, tok.lexema };
        }
        if (tok.lexema == "real") {
            tok.token = T_REAL;
            printToken(tok);
            return { T_REAL, tok.lexema };
        }
        if (tok.lexema == "repita") {
            tok.token = T_REPITA;
            printToken(tok);
            return { T_REPITA, tok.lexema };
        }
        if (tok.lexema == "enquanto") {
            tok.token = T_ENQUANTO;
            printToken(tok);
            return { T_ENQUANTO, tok.lexema };
        }
        if (tok.lexema == "se") {
            tok.token = T_SE;
            printToken(tok);
            return { T_SE, tok.lexema };
        }
        if (tok.lexema == "senao") {
            tok.token = T_SENAO;
            printToken(tok);
            return { T_SENAO, tok.lexema };
        }
        if (tok.lexema == "entao") {
            tok.token = T_ENTAO;
            printToken(tok);
            return { T_ENTAO, tok.lexema };
        }
        if (tok.lexema == "ate") {
            tok.token = T_ATE;
            printToken(tok);
            return { T_ATE, tok.lexema };
        }
        if (tok.lexema == "mostrar") {
            tok.token = T_MOSTRAR;
            printToken(tok);
            return { T_MOSTRAR, tok.lexema };
        }
        if (tok.lexema == "ler") {
            tok.token = T_LER;
            printToken(tok);
            return { T_LER, tok.lexema };
        }

        tok.token = T_ID;
        printToken(tok);
        return { T_ID, tok.lexema };
    }

    if (input[posicao] == '=') {
        posicao++;
        if (input[posicao] == '=') {
            tok.token = T_IGUAL_IGUAL;
            printToken(tok);
            posicao++;
            return { T_IGUAL_IGUAL, "==" };
        }
        tok.token = T_IGUAL;
        printToken(tok);
        return { T_IGUAL, "=" };
    }

    if (input[posicao] == '!') {
        posicao++;
        if (input[posicao] == '=') {
            tok.token = T_DIFERENTE;
            printToken(tok);
            posicao++;
            return { T_DIFERENTE, "!=" };
        }

        error("Token inesperado");
    }


    if (isdigit(input[posicao]) || (input[posicao] == '-' && isdigit(input[posicao + 1]))) {
        bool isNegative = false;
        bool isReal = false;
        tok.lexema = "";


        if (input[posicao] == '-') {
            isNegative = true;
            tok.lexema += "-";
            posicao++;
        }

        tok.value = 0;
        while (isdigit(input[posicao])) {

            tok.lexema += input[posicao];
            tok.value = tok.value * 10 + (input[posicao] - '0');
            posicao++;
        }
        if (input[posicao] == '.') {
            isReal = true;
            tok.lexema += ".";
            posicao++;
        }
        while (isdigit(input[posicao])) {
            tok.lexema += input[posicao];
            //tok.value = tok.value * 10 + ((input[posicao] - '0') / 10);
            posicao++;
        }
        tok.value = stod(tok.lexema);
        tok.token = isReal ? T_NUM_REAL : T_NUM_INTEIRO;

        printToken(tok);

        // inverte o valor do token
        isNegative ? tok.value = -tok.value : NULL;
        return tok;
    }

    if (input[posicao] == '+') { tok.token = T_MAIS; tok.lexema = "+"; printToken(tok); posicao++; return tok; }
    if (input[posicao] == '-') { tok.token = T_MENOS; tok.lexema = "-"; printToken(tok); posicao++; return tok; }
    if (input[posicao] == '*') { tok.token = T_MULT; tok.lexema = "*"; printToken(tok); posicao++; return tok; }
    if (input[posicao] == '/') { tok.token = T_DIV; tok.lexema = "/"; printToken(tok); posicao++; return tok; }
    if (input[posicao] == '>') { tok.token = T_MAIOR; tok.lexema = ">"; printToken(tok); posicao++; return tok; }
    if (input[posicao] == '>=') { tok.token = T_MAIOR_IGUAL; tok.lexema = ">="; printToken(tok); posicao++; return tok; }
    if (input[posicao] == '<') { tok.token = T_MENOR; tok.lexema = "<"; printToken(tok); posicao++; return tok; }
    if (input[posicao] == '<=') { tok.token = T_MENOR_IGUAL; tok.lexema = "<="; printToken(tok); posicao++; return tok; }
    if (input[posicao] == '&&') { tok.token = T_E; tok.lexema = "&&"; printToken(tok); posicao++; return tok; }
    if (input[posicao] == '||') { tok.token = T_OU; tok.lexema = "||"; printToken(tok); posicao++; return tok; }
    if (input[posicao] == ';') { tok.token = T_PONTO_VIRGULA; tok.lexema = ";"; printToken(tok); posicao++; return tok; }
    if (input[posicao] == ',') { tok.token = T_VIRGULA; tok.lexema = ","; printToken(tok); posicao++; return tok; }
    if (input[posicao] == '{') { tok.token = T_ABRE_CHAVES; tok.lexema = "{"; printToken(tok); posicao++; return tok; }
    if (input[posicao] == '}') { tok.token = T_FECHA_CHAVES; tok.lexema = "}"; printToken(tok); posicao++; return tok; }
    if (input[posicao] == '(') { tok.token = T_ABRE_PARENTESES; tok.lexema = "("; printToken(tok); posicao++; return tok; }
    if (input[posicao] == ')') { tok.token = T_FECHA_PARENTESES; tok.lexema = ")"; printToken(tok); posicao++; return tok; }



    // Caractere desconhecido
    tok.token = T_UNKNOWN;
    tok.lexema = "desconhecido";
    //printToken(tok);
    posicao++;
    return tok;
}


// Função para exibir os tokens
void printToken(TokenValue tok) {
    switch (tok.token) {
    case T_INTEIRO: cout << "Token: T_INTEIRO, " << tok.lexema << endl; break;
    case T_REAL: cout << "Token: T_REAL, " << tok.lexema << endl; break;
    case T_REPITA: cout << "Token: T_REPITA, " << tok.lexema << endl; break;
    case T_ENQUANTO: cout << "Token: T_ENQUANTO, " << tok.lexema << endl; break;
    case T_SE: cout << "Token: T_SE, " << tok.lexema << endl; break;
    case T_SENAO: cout << "Token: T_SENAO, " << tok.lexema << endl; break;
    case T_ENTAO: cout << "Token: T_ENTAO, " << tok.lexema << endl; break;
    case T_ATE: cout << "Token: T_ATE, " << tok.lexema << endl; break;
    case T_MOSTRAR: cout << "Token: T_MOSTRAR, " << tok.lexema << endl; break;
    case T_LER: cout << "Token: T_LER, " << tok.lexema << endl; break;
    case T_ID: cout << "Token: T_ID, " << tok.lexema << endl; break;
    //case T_NUM: cout << "Token: T_NUM, " << tok.lexema << ", Valor: " << tok.value << endl; break;
    case T_NUM_REAL: cout << "Token: T_NUM_REAL, " << tok.lexema << ", Valor: " << tok.value << endl; break;
    case T_NUM_INTEIRO: cout << "Token: T_NUM_INTEIRO, " << tok.lexema << ", Valor: " << tok.value << endl; break;
    case T_IGUAL: cout << "Token: T_IGUAL, " << tok.lexema << endl; break;
    case T_IGUAL_IGUAL: cout << "Token: T_IGUAL_IGUAL, " << tok.lexema << endl; break;
    case T_DIFERENTE: cout << "Token: T_DIFERENTE, " << tok.lexema << endl; break;
    case T_MAIS: cout << "Token: T_MAIS, " << tok.lexema << endl; break;
    case T_MENOS: cout << "Token: T_MENOS, " << tok.lexema << endl; break;
    case T_MULT: cout << "Token: T_MULT, " << tok.lexema << endl; break;
    case T_DIV: cout << "Token: T_DIV, " << tok.lexema << endl; break;
    case T_MAIOR: cout << "Token: T_MAIOR, " << tok.lexema << endl; break;
    case T_MAIOR_IGUAL: cout << "Token: T_MAIOR_IGUAL, " << tok.lexema << endl; break;
    case T_MENOR: cout << "Token: T_MENOR, " << tok.lexema << endl; break;
    case T_MENOR_IGUAL: cout << "Token: T_MENOR_IGUAL, " << tok.lexema << endl; break;
    case T_OU: cout << "Token: T_OU, " << tok.lexema << endl; break;
    case T_E: cout << "Token: T_E, " << tok.lexema << endl; break;
    case T_ABRE_CHAVES: cout << "Token: T_ABRE_CHAVES, " << tok.lexema << endl; break;
    case T_FECHA_CHAVES: cout << "Token: T_FECHA_CHAVES, " << tok.lexema << endl; break;
    case T_ABRE_PARENTESES: cout << "Token: T_ABRE_PARENTESES, " << tok.lexema << endl; break;
    case T_FECHA_PARENTESES: cout << "Token: T_FECHA_PARENTESES, " << tok.lexema << endl; break;
    case T_PONTO_VIRGULA: cout << "Token: T_PONTO_VIRGULA, " << tok.lexema << endl; break;
    case T_VIRGULA: cout << "Token: T_VIRGULA, " << tok.lexema << endl; break;
    case T_EOF: cout << "Token: T_EOF" << endl; break;
    case T_UNKNOWN: cout << "Token: T_UNKNOWN, " << tok.lexema << endl; break;
    }
}


int main() {
    cout << "Digite o código de entrada (insira 'FIM' para finalizar):" << endl;

    string line;
    input = "";

    while (true) {
        getline(cin, line);
        if (line == "FIM") break;
        input += line + "\n";
    }

    cout << endl << endl << endl << endl;

    outputSemFile.open("./arvore_semantica.txt");
    outputSintFile.open("./arvore_sintatica.txt");
    outputASFile.open("./arvore_de_simbolos.txt");

    if (!outputSemFile.is_open()) {
        cout << "Erro ao abrir o arquivo de saída da arvore semantica." << endl;
        return 1;
    }

    if (!outputSintFile.is_open()) {
        cout << "Erro ao abrir o arquivo de saída da arvore sintatica." << endl;
        return 1;
    }

    if (!outputASFile.is_open()) {
        cout << "Erro ao abrir o arquivo de saída da tabela de simbolos." << endl;
        return 1;
    }

    nextStep();
    ArvoreNode* ast = Programa();

    printArvoreSemantica(ast);
    printArvoreSintatica(ast);
    printTabelaDeSimbolos();

    outputSemFile.close();
    outputSintFile.close();
    outputASFile.close();

    cout << "Análise sintática concluída. Veja 'arvore_sintatica.txt', 'arvore_semantica.txt' e 'tabela_de_simbolos.txt' para o resultado." << endl;

    return 0;
}