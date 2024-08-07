/*
    Implementação do lexer para a linguagem CePe
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <Windows.h>
#include <cstdio>

using namespace std;

// Tipos de token na linguagem
enum Tokens
{
    ID = 256,
    INT_NUM,
    FLOAT_NUM,
    TRUE_TK,
    FALSE_TK,
    INT_TK,
    FLOAT_TK,
    CHAR_TK,
    STRING_TK,
    LIST_TK,
    BOOL_TK,
    FUNCTION_TK,
    FOR_TK,
    WHILE_TK,
    IF_TK,
    ELSE_TK,
    END_TK
};

map<int, string> nomesTokens = {
    {Tokens::ID, "id"},
    {Tokens::INT_NUM, "num int"},
    {Tokens::FLOAT_NUM, "num float"},
    {Tokens::TRUE_TK, "true"},
    {Tokens::FALSE_TK, "false"},
    {Tokens::INT_TK, "int"},
    {Tokens::FLOAT_TK, "float"},
    {Tokens::CHAR_TK, "char"},
    {Tokens::STRING_TK, "string"},
    {Tokens::LIST_TK, "list"},
    {Tokens::BOOL_TK, "bool"},
    {Tokens::FUNCTION_TK, "function"},
    {Tokens::FOR_TK, "for"},
    {Tokens::WHILE_TK, "while"},
    {Tokens::IF_TK, "if"},
    {Tokens::ELSE_TK, "else"},
    {Tokens::END_TK, "end"}
};

class Token
{
public:
    int tipo;
    string texto;
    int linha;
    int coluna;
};

int main()
{

    ifstream file("entrada.txt");

    if (!file)
    {
        cout << "Deu pra abrir não";
        return 1;
    }

    // Armazena a linha e coluna atuais
    int linha = 1, coluna = 1;

    // Sequência de tokens
    vector<Token> tokens;

    char ch;
    while (file.get(ch))
    {

        if (ch == ' ')
        {
            coluna++;
        }
        else if (ch == '\t')
        {
            coluna += 4;
        }
        else if (ch == '\n')
        {
            coluna = 1;
            linha++;
        }
        else if (ch == '"') // STRINGS
        {
            string lexema{ch};

            Token tk;
            tk.linha = linha;
            tk.coluna = coluna;

            while (file.peek() != '"' && file.get(ch))
            {
                lexema += ch;
            }

            // Pegar a última aspa
            file.get(ch);
            lexema += ch;

            coluna += lexema.length();

            tk.tipo = Tokens::STRING_TK;
            tk.texto = lexema;

            tokens.push_back(tk);
        }
        else if (isdigit(ch)) // INTEIROS OU FLOATS
        {
            string lexema{ch};

            Token tk;
            tk.linha = linha;
            tk.coluna = coluna;

            while (isdigit(file.peek()) && file.get(ch))
            {
                lexema += ch;
            }

            if (file.peek() == '.') // FLOAT
            {
                file.get(ch);
                lexema += ch;

                while (isdigit(file.peek()) && file.get(ch))
                {
                    lexema += ch;
                }

                coluna += lexema.length();

                tk.tipo = Tokens::FLOAT_NUM;
                tk.texto = lexema;

                tokens.push_back(tk);

                continue;
            }

            coluna += lexema.length();

            tk.tipo = Tokens::INT_NUM;
            tk.texto = lexema;

            tokens.push_back(tk);
        }
        else if (isalpha(ch)) // IDENTIFICADORES OU PALAVRAS-CHAVE
        {
            string lexema{ch};

            Token tk;
            tk.linha = linha;
            tk.coluna = coluna;

            while (isalnum(file.peek()) && file.get(ch))
            {
                lexema += ch;
            }

            coluna += lexema.length();

            if (lexema == "verperdapadepe")
                tk.tipo = Tokens::TRUE_TK;
            else if (lexema == "fapalapacipiapa")
                tk.tipo = Tokens::FALSE_TK;
            else if (lexema == "inpintepe")
                tk.tipo = Tokens::INT_TK;
            else if (lexema == "virpirgupulapa")
                tk.tipo = Tokens::FLOAT_TK;
            else if (lexema == "simpim") // SIMbolo
                tk.tipo = Tokens::CHAR_TK;
            else if (lexema == "serperiepie")
                tk.tipo = Tokens::STRING_TK;
            else if (lexema == "lispistapa")
                tk.tipo = Tokens::LIST_TK;
            else if (lexema == "boopoo")
                tk.tipo = Tokens::BOOL_TK;
            else if (lexema == "funpuncaopao")
                tk.tipo = Tokens::FUNCTION_TK;
            else if (lexema == "paparapa")
                tk.tipo = Tokens::FOR_TK;
            else if (lexema == "dupuranpantepe")
                tk.tipo = Tokens::WHILE_TK;
            else if (lexema == "sepe")
                tk.tipo = Tokens::IF_TK;
            else if (lexema == "sepenaopao")
                tk.tipo = Tokens::ELSE_TK;
            else if (lexema == "fimpim")
                tk.tipo = Tokens::END_TK;
            else
                tk.tipo = Tokens::ID;

            tk.texto = lexema;
            tokens.push_back(tk);
        }
        else
        {
            string lexema{ch};

            Token tk;
            tk.tipo = ch;
            tk.texto = lexema;
            tk.linha = linha;
            tk.coluna = coluna;

            tokens.push_back(tk);
            coluna++;
        }
    }

    // Close the file
    file.close();

    for (Token tk : tokens)
    {
        if (nomesTokens.count(tk.tipo) > 0)
        {
            cout << nomesTokens[tk.tipo] << ": " << tk.texto << "\n\tLinha: " << tk.linha << ", coluna: " << tk.coluna << endl;
            continue;
        }

        cout << char(tk.tipo) << ": " << tk.texto << "\n\tLinha: " << tk.linha << ", coluna: " << tk.coluna << endl;
    }

    return 0;
}