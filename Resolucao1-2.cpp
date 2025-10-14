#include <iostream>
#include <vector>
#include <cmath>
#include <omp.h>

// Função para calcular as raízes de uma equação de segundo grau
std::pair<double, double> resolver_bhaskara(double a, double b, double c)
{
    double delta = (b * b) - (4 * a * c);

    if (delta < 0)
    {
        return {0.0, 0.0}; // Sem raízes reais
    }

    double x1 = (-b + std::sqrt(delta)) / (2 * a);
    double x2 = (-b - std::sqrt(delta)) / (2 * a);
    return {x1, x2};
}

int main()
{
    // Exercício 2: agora o usuário define quantas equações quer resolver
    int N;
    std::cout << "Digite a quantidade de equacoes a serem resolvidas: ";
    std::cin >> N;

    std::vector<double> a(N), b(N), c(N);

    // Preenche os vetores com valores de exemplo
    for (int i = 0; i < N; ++i)
    {
        a[i] = 1.0;
        if (i % 2 == 0)
        {
            b[i] = -7.0;
            c[i] = 10.0; // Raízes reais
        }
        else
        {
            b[i] = 2.0;
            c[i] = 5.0; // Sem raízes reais
        }
    }

    int reais1 = 0, irreais1 = 0; // Usando reduction
    int reais2 = 0, irreais2 = 0; // Usando critical
    int reais3 = 0, irreais3 = 0; // Usando atomic

    std::cout << "\nReduction\n";
    double inicio = omp_get_wtime();

    // - Cada thread tem uma cópia local das variáveis de contagem.
    //- Ao final do loop, o OpenMP soma todas as cópias automaticamente.
   // - Muito eficiente, evita conflitos de threads (race conditions) sem bloquear.
#pragma omp parallel for reduction(+ : reais1, irreais1)
    for (int i = 0; i < N; ++i)
    {
        auto res = resolver_bhaskara(a[i], b[i], c[i]);
        if (res.first == 0.0 && res.second == 0.0)
            irreais1++;
        else
            reais1++;
    }
    double fim = omp_get_wtime();
    std::cout << "Reais: " << reais1 << " | Irreais: " << irreais1 << "\n";
    std::cout << "Tempo: " << (fim - inicio) << " segundos\n";

    std::cout << "\nCritical\n";
    inicio = omp_get_wtime();


// - Cada thread entra na **seção crítica** para atualizar a variável.
//   - Apenas **uma thread por vez** pode executar o código dentro do critical.
//  - Funciona, mas é menos eficiente que reduction porque threads podem esperar.
#pragma omp parallel for
    for (int i = 0; i < N; ++i)
    {
        auto res = resolver_bhaskara(a[i], b[i], c[i]);
        if (res.first == 0.0 && res.second == 0.0)
        {

#pragma omp critical
            irreais2++;
        }
        else
        {

#pragma omp critical
            reais2++;
        }
    }
    fim = omp_get_wtime();
    std::cout << "Reais: " << reais2 << " | Irreais: " << irreais2 << "\n";
    std::cout << "Tempo: " << (fim - inicio) << " segundos\n";

    std::cout << "\nAtomic\n";
    inicio = omp_get_wtime();

// - Operação simples (como +=) é feita de forma **atômica**.
// - Garante que nenhuma thread interrompa a operação enquanto outra faz a mesma.
// - Mais eficiente que critical para incrementos simples, mas não funciona para código complexo.

#pragma omp parallel for
    for (int i = 0; i < N; ++i)
    {
        auto res = resolver_bhaskara(a[i], b[i], c[i]);
        if (res.first == 0.0 && res.second == 0.0)
        {
#pragma omp atomic
            irreais3++;
        }
        else
        {
#pragma omp atomic
            reais3++;
        }
    }
     fim = omp_get_wtime();
    std::cout << "Reais: " << reais3 << " | Irreais: " << irreais3 << "\n";
     std::cout << "Tempo: " << (fim - inicio) << " segundos\n";

    return 0;
}
