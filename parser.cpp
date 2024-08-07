/*
    Implementação de um parser LR(1) para a linguagem CePe
    PRÓXIMAS FUNÇÕES:
    - Uma para criar um estado inteiro, dado um conjunto de posições inicias
    - Uma para criar TODOS os estados, utilizando a função acima
*/

#include <iostream>
#include <vector>
#include <map>
#include <chrono>
#include <algorithm>
#include <stack>

using namespace std;
using namespace std::chrono;

const int terminalInt = 10;

enum NonTerminals
{
    B = 512,
    S,
    E,
    T
};

struct InfoNaoTerminal
{
    int indexComeco; // Informa quando começam as regras para aquele não terminal na gramática
    int indexFim;    // Informa quando terminam as regras para aquele não terminal
};

/*
    Códifica uma "posição"
    Exemplo: Posicao{{E, E, '+', T}, 1, {'+', '-', EOF}} == E -> E . + T {+, -, EOF}

    O "corpo" de uma posição é a mistura do não terminal com a regra
*/
struct Posicao
{
    int regra;              // Index da regra na gramática para esse não terminal
    int posicao;            // Posição específica onde estamos na regra
    vector<int> lookaheads; // Conjunto de elementos que podem resultar numa redução
};

bool operator==(Posicao pos1, Posicao pos2)
{
    return (
        pos1.regra == pos2.regra &&
        pos1.posicao == pos2.posicao &&
        pos1.lookaheads == pos2.lookaheads);
}

map<int, string> simbolosNomes = {
    {EOF, "EOF"},
    {S, "S"},
    {E, "E"},
    {T, "T"},
    {B, "B"},
    {terminalInt, "int"},
    {'+', "+"},
    {'-', "-"}};

// Gramática contendo todas as regras
// Cada regra é armazenada como um vetor de inteiros, onde o primeiro símbolo
// é o não terminal formado a partir dos próximos símbolos na regra
vector<vector<int>> gramatica = {
    {S, E},
    {E, E, '+', T},
    {E, E, '-', T},
    {E, T},
    {T, terminalInt}};

const int tamanhoGramatica = gramatica.size();

map<int, InfoNaoTerminal> infoNaoTerminais = {
    {S, {0, 1}},
    {E, {1, 4}},
    {T, {4, 5}}};

vector<Posicao> estadoInicial = {{0, 1, {EOF}}};

// Armazena todos os estados da gramática
vector<vector<Posicao>> estados;

// Armazena todos os terminais que podem estar no começo de uma regra que forma um não terminal
map<int, vector<int>> firstTabela;

// Armazena todos os terminais que podem seguir um dado não terminal
map<int, vector<int>> followTabela;

// Armazena a tabela ACTION, que, dado um estado e um terminal, diz qual a próxima ação a ser tomada
map<int, map<int, string>> actionTabela;

void printGramatica();
void printFirst();
void printFollow();
void printTabelaEstados();

template <typename T>
void acumular(T &acumulado, void (*acumulante)(T &));
void FIRST(map<int, vector<int>> &tabela);
void FOLLOW(map<int, vector<int>> &tabela);
int buscaPorCorpo(vector<Posicao> elementos, Posicao pos);
void criarEstadoFinal(vector<Posicao> &estadoInicial);
void criarEstados(vector<vector<Posicao>> &estados);
void PARSE(vector<int> entrada);

int estadosCriados;

const vector<int> TOKENS = {terminalInt, '+', '-', EOF};

int main()
{
    auto start = high_resolution_clock::now();

    const int ITER = 1;

    for (int i = 0; i < ITER; i++)
    {
        firstTabela.clear();
        followTabela = {{S, {EOF}}};
        actionTabela.clear();
        estados = {estadoInicial};
        estadosCriados = 1;

        // Código que cria as tabelas necessárias para o parsing
        acumular<map<int, vector<int>>>(firstTabela, FIRST);
        acumular<map<int, vector<int>>>(followTabela, FOLLOW);
        acumular<vector<vector<Posicao>>>(estados, criarEstados);

        // Código que realiza o parsing
        PARSE(TOKENS);
    }

    auto end = high_resolution_clock::now();
    duration<double, milli> duration = end - start;

    // Código fru fru

    // printGramatica();
    // printFirst();
    // printFollow();
    printTabelaEstados();
    cout << endl
         << "Tempo médio: " << to_string(duration.count() / double(ITER)) << " ms." << endl;

    return 0;
}

void printGramatica()
{
    cout << endl
         << "=== GRAMÁTICA ===";

    int lastSimb = 0;
    for (auto regra : gramatica)
    {
        if (lastSimb != regra[0])
            cout << endl
                 << simbolosNomes[regra[0]] << ": ";
        else
            cout << "\n| ";

        for (int i = 1; i < regra.size(); i++)
        {
            cout << simbolosNomes[regra[i]] << " ";
        }

        lastSimb = regra[0];
    }

    cout << endl;
}

void printFirst()
{
    cout << endl
         << "=== FIRST ===" << endl;
    for (auto el = firstTabela.begin(); el != firstTabela.end(); el++)
    {
        cout << simbolosNomes[el->first] << ": {";
        for (const int &i : el->second)
        {
            cout << simbolosNomes[i] << ",";
        }
        cout << "}" << endl;
    }
}

void printFollow()
{
    cout << endl
         << "=== FOLLOW ===" << endl;
    for (auto el = followTabela.begin(); el != followTabela.end(); el++)
    {
        cout << simbolosNomes[el->first] << ": {";
        for (const int &i : el->second)
        {
            cout << simbolosNomes[i] << ",";
        }
        cout << "}" << endl;
    }
}

void printTabelaEstados()
{
    cout << endl
         << "=== TABELA DE ESTADOS ===" << endl;
    for (int i = 0; i < estados.size(); i++)
    {
        vector<Posicao> estado = estados[i];
        map<int, string> actionAtual = actionTabela[i];

        cout << endl
             << "== ESTADO " << to_string(i) << " == " << endl;
        for (Posicao pos : estado)
        {
            vector<int> regra = gramatica[pos.regra];
            cout << simbolosNomes[regra[0]] << " -> ";

            // Regra
            for (int i = 1; i < regra.size(); i++)
            {
                if (i == pos.posicao)
                    cout << ". ";

                cout << simbolosNomes[regra[i]] << " ";
            }

            if (pos.posicao == regra.size())
                cout << ". ";

            // Lookaheads
            cout << "{";

            for (int i = 0; i < pos.lookaheads.size(); i++)
            {
                cout << simbolosNomes[pos.lookaheads[i]] << ",";
            }
            cout << "}" << endl;
        }
        for (auto acao = actionAtual.begin(); acao != actionAtual.end(); acao++)
        {
            cout << simbolosNomes[acao->first] << ": " << acao->second << endl;
        }
    }
}

// Utiliza a propriedade de "transitive closure" das funções abaixo
template <typename T>
void acumular(T &acumulado, void (*acumulante)(T &))
{
    T temp;

    do
    {
        temp = acumulado;
        acumulante(acumulado);
    } while (temp != acumulado);
}

// Retorna um conjunto de símbolos terminais que podem estar
// no começo de dado símbolo não terminal
void FIRST(map<int, vector<int>> &tabela)
{
    for (auto regra : gramatica)
    {
        // Vetor auxiliar (foi o jeito que achei de manter o conjunto exclusivo)
        vector<int> temp;

        // O não terminal em questão
        const int naoTerminal = regra[0];

        const int primeiroSimbolo = regra[1];

        // Se for terminal, adicionar ao conjunto
        if (primeiroSimbolo < 512)
        {
            temp.push_back(primeiroSimbolo);
        }
        // Se for não terminal e for diferente do não terminal passado,
        // chamar FIRST(primeiroSimbolo) e adicionar o resultado ao conjunto
        else if (primeiroSimbolo != naoTerminal)
        {
            const vector<int> terminaisSimbolo = tabela[primeiroSimbolo];
            temp.insert(temp.end(), terminaisSimbolo.begin(), terminaisSimbolo.end());
        }

        // Atualizar tabela
        tabela[naoTerminal] = temp;
    }
}

// Retorna um conjunto de símbolos terminais que podem seguir
// dado símbolo não terminal
void FOLLOW(map<int, vector<int>> &tabela)
{
    map<int, vector<int>> temp = {{S, {EOF}}};

    for (auto regra : gramatica)
    {
        // Seleciona cada símbolo da regra atual, pulando o não terminal gerado a partir da regra
        for (int i = 1; i < regra.size(); i++)
        {
            const int simbolo = regra[i];

            // Se o símbolo for terminal, pode pular
            if (simbolo < 512)
                continue;

            // Verifica se o próximo símbolo seria o final da regra
            if (i + 1 == regra.size())
            {
                temp[simbolo] = tabela[regra[0]];
                continue;
            }

            // Extraí o próximo símbolo
            const int proxSimbolo = regra[i + 1];

            // Se for terminal, adicionar ao conjunto
            if (proxSimbolo < 512)
            {
                temp[simbolo].push_back(proxSimbolo);
            }
            // Se for não terminal, adiciona FIRST(proxSimbolo) ao conjunto de terminais
            else
            {
                const vector<int> terminaisSimbolo = firstTabela[proxSimbolo];
                temp[simbolo].insert(tabela[simbolo].end(), terminaisSimbolo.begin(), terminaisSimbolo.end());
            }
        }
    }

    // Atualiza a tabela
    tabela = temp;
}

int buscaPorCorpo(vector<Posicao> elementos, Posicao pos)
{
    int len = elementos.size();

    for (int i = 0; i < len; i++)
    {
        Posicao posAtual = elementos[i];
        if (posAtual.regra == pos.regra && posAtual.posicao == pos.posicao)
            return i;
    }

    return len;
}

// Cria todas as possíveis posições para um estado dado um estado inicial
void criarEstadoFinal(vector<Posicao> &estadoInicial)
{
    // E -> E . + T {+, -, EOF}
    // Posicao {naoTerminal = E, Regra = {E + T}, posicao = 2, lookaheads = {+, -, EOF}}
    vector<Posicao> temp = estadoInicial;
    for (Posicao pos : estadoInicial)
    {
        vector<int> regra = gramatica[pos.regra];

        // Chegamos no final da regra
        if (pos.posicao == regra.size())
            continue;

        const int proxSimbolo = regra[pos.posicao];

        // O próximo símbolo é um terminal
        if (proxSimbolo < 512)
            continue;

        // A partir daqui, o próximo símbolo é não terminal

        // Construir o conjunto de lookahead antes do loop é mais eficiente
        vector<int> lookaheads;

        // Se o elemento após o próximo não existir
        if (pos.posicao + 1 == regra.size())
            lookaheads = followTabela[regra[0]];
        // Se o o elemento após o próximo for um terminal
        else if (regra[pos.posicao + 1] < 512)
            lookaheads = {regra[pos.posicao + 1]};
        // Por fim, se o elemento após o próximo for um não terminal
        else
            lookaheads = firstTabela[regra[pos.posicao + 1]];

        // Criando uma variável para as novas posições
        Posicao novaPos;
        novaPos.posicao = 1;
        novaPos.lookaheads = lookaheads;

        InfoNaoTerminal naoTerminalAtual = infoNaoTerminais[proxSimbolo];

        for (int i = naoTerminalAtual.indexComeco; i < naoTerminalAtual.indexFim; i++)
        {
            novaPos.regra = i;

            int index = buscaPorCorpo(temp, novaPos);

            if (index == temp.size())
                temp.push_back(novaPos);
            else
            {
                vector<int> lookahdeasPos = temp[index].lookaheads;

                for (int i = 0; i < lookaheads.size(); i++)
                {
                    if (find(lookahdeasPos.begin(), lookahdeasPos.end(), lookaheads[i]) == lookahdeasPos.end())
                        lookahdeasPos.push_back(lookaheads[i]);
                }

                temp[index].lookaheads = lookahdeasPos;
            }
        }
    }

    estadoInicial = temp;
}

// Cria todos os estados a partir de um estado inicial
void criarEstados(vector<vector<Posicao>> &estados)
{
    vector<vector<Posicao>> temp = estados;

    // Armazena quantos estados foram criados na última iteração
    // static int n = 1;
    const int fimTemp = temp.size();
    const int comecoLoop = fimTemp - estadosCriados;

    for (int i = comecoLoop; i < fimTemp; i++)
    {
        vector<Posicao> estadoAtual = temp[i];

        // Atualizar o estado
        acumular<vector<Posicao>>(estadoAtual, criarEstadoFinal);
        temp[i] = estadoAtual;

        vector<vector<Posicao>> novosEstados;
        map<int, int> simboloParaEstado;

        for (Posicao pos : estadoAtual)
        {
            const vector<int> regra = gramatica[pos.regra];

            if (pos.posicao == regra.size())
            {
                for (int lk : pos.lookaheads)
                {
                    string action = "r" + to_string(pos.regra);
                    if (actionTabela[i][lk] == action)
                    {
                        string previousAction(1, actionTabela[i][lk][0]);
                        actionTabela[i][lk] = "c" + previousAction + "r";
                    }
                    else
                        actionTabela[i][lk] = action;
                }
                continue;
            }

            const int proxSimbolo = regra[pos.posicao];
            Posicao novaPos = pos;
            novaPos.posicao++;

            // Se o próximo símbolo da regra atual ainda não levar a nenhum novo estado
            if (simboloParaEstado.count(proxSimbolo) == 0)
            {
                const int novosLen = novosEstados.size();
                string action = "s" + to_string(fimTemp + novosLen);
                simboloParaEstado[proxSimbolo] = novosLen;
                if (actionTabela[i][proxSimbolo] == action)
                    actionTabela[i][proxSimbolo] = "cs" + string(1, actionTabela[i][proxSimbolo][0]);
                else
                    actionTabela[i][proxSimbolo] = action;
                novosEstados.push_back({});
            }

            novosEstados[simboloParaEstado[proxSimbolo]].push_back(novaPos);
        }

        temp.insert(temp.end(), novosEstados.begin(), novosEstados.end());
    }
    // A quantidade de estados criada é igual a diferença entre o
    // tamanho final e inicial do vetor de estados
    estadosCriados = temp.size() - fimTemp;

    estados = temp;
}

void PARSE(vector<int> entrada)
{
    /*
        Preciso de:
        - Fila de estados
        - Fila de caracters
    */

    stack<int> estados;
    stack<int> simbolos;
    stack<int> tokens;

    for (int i = entrada.size(); i >= 0; i--)
    {
        tokens.push(entrada[i]);
    }

    // Inicializar as filas;
    estados.push(0);  // Começamos no estado 0
    simbolos.push(S); // Começamos com o símbolo S

    while (true)
    {
        int tokenAtual = tokens.top();
        int estadoAtual = estados.top();
        string acaoAtual = actionTabela[estadoAtual][tokenAtual];

        if (acaoAtual.empty())
        {
            cerr << "Erro de sintaxe." << endl;
            exit(1);
        }

        if (acaoAtual == "r0")
        {
            cout << "Entrada aceita" << endl;
            return;
        }

        // Ação de shift
        if (acaoAtual[0] == 's')
        {
            int proxEstado = acaoAtual[1] - '0';

            estados.push(proxEstado);
            simbolos.push(tokenAtual);
            tokens.pop();
            continue;
        }

        // Ação de reduce
        vector<int> regraReduce = gramatica[acaoAtual[1] - '0'];
        int simboloReduce = regraReduce[0];
        int tamanhoReduce = regraReduce.size() - 1;

        for (int i = 0; i < tamanhoReduce; i++)
        {
            simbolos.pop();
            estados.pop();
        }

        tokens.push(simboloReduce);
    }
}
