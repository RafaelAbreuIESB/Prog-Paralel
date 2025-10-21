#include <iostream>
#include <vector>
#include <cmath>
#include <omp.h>
#include <fstream> // biblioteca para manipulação de arquivos


// Função que resolve a equação de segundo grau (Bhaskara)
double resolver_bhaskara(double a, double b, double c)
{
    double delta = (b * b) - (4 * a * c);
    if (delta < 0)
        return 0.0; // sem raízes reais, retorna 0 para simplificar o somatório
    double x1 = (-b + std::sqrt(delta)) / (2 * a);
    double x2 = (-b - std::sqrt(delta)) / (2 * a);
    return x1 + x2; // retornamos a soma das raízes
}

int main()
{
    int N;
    std::cout << "Digite a quantidade de equacoes a serem resolvidas: ";
    std::cin >> N;

    // Vetores que armazenam os coeficientes de cada equação
    std::vector<double> a(N), b(N), c(N);

    // Inicialização dos coeficientes
    for (int i = 0; i < N; ++i)
    {
        a[i] = 1.0;
        if (i % 2 == 0)
        {
            b[i] = -5.0;
            c[i] = 6.0;
        }
        else
        {
            b[i] = -7.0;
            c[i] = 10.0;
        }
    }


    // Variáveis compartilhadas e Locks
    // Teremos duas somas separadas: uma para índices pares e outra para ímpares.
    double soma_pares = 0.0;
    double soma_impares = 0.0;

    // Cada lock protege uma variável de soma.
    omp_lock_t lock_pares;
    omp_lock_t lock_impares;

    // Inicializamos os locks antes do uso
    omp_init_lock(&lock_pares);
    omp_init_lock(&lock_impares);

    double inicio = omp_get_wtime();

    // Região paralela: processamento com locks
#pragma omp parallel for
    for (int i = 0; i < N; ++i)
    {
        // Cada thread calcula a soma das raízes da equação i
        double soma_local = resolver_bhaskara(a[i], b[i], c[i]);

        // Decidimos qual recurso cada thread deve acessar
        if (i % 2 == 0)
        {
            // Lock para proteger a soma dos índices pares
            omp_set_lock(&lock_pares);
            soma_pares += soma_local;
            omp_unset_lock(&lock_pares);
        }
        else
        {
            // Lock para proteger a soma dos índices ímpares
            omp_set_lock(&lock_impares);
            soma_impares += soma_local;
            omp_unset_lock(&lock_impares);
        }
    }

    double fim = omp_get_wtime();


    // Após o uso, destruímos os locks para liberar memória
    omp_destroy_lock(&lock_pares);
    omp_destroy_lock(&lock_impares);


    // Exibição dos resultados no console

    std::cout << "\nResultados com sincronizacao via Locks:\n";
    std::cout << "Soma das equacoes pares: " << soma_pares << std::endl;
    std::cout << "Soma das equacoes impares: " << soma_impares << std::endl;
    std::cout << "Soma total: " << soma_pares + soma_impares << std::endl;
    std::cout << "Tempo de execucao: " << (fim - inicio) << " segundos\n";

    // Escrita dos resultados em arquivo .txt
    std::ofstream arquivo("resultados_locks.txt");

    if (arquivo.is_open())
    {
        arquivo << " Resultados Locks:\n";
        arquivo << "Quantidade de equacoes: " << N << "\n\n";
        arquivo << "Soma das equacoes pares: " << soma_pares << "\n";
        arquivo << "Soma das equacoes impares: " << soma_impares << "\n";
        arquivo << "Soma total: " << soma_pares + soma_impares << "\n";
        arquivo << "Tempo de execucao: " << (fim - inicio) << " segundos\n";
        arquivo.close();

        std::cout << "\nArquivo 'resultados_locks.txt' gerado com sucesso!\n";
    }
    else
    {
        std::cout << "Erro ao criar o arquivo de resultados.\n";
    }

    return 0;
}
