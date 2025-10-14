#include <iostream>
#include <vector>
#include <cmath>
#include <omp.h>

// Função que resolve a equação de segundo grau
std::pair<double, double> resolver_bhaskara(double a, double b, double c)
{
    double delta = (b * b) - (4 * a * c);
    if (delta < 0)
        return {0.0, 0.0}; // sem raízes reais
    double x1 = (-b + std::sqrt(delta)) / (2 * a);
    double x2 = (-b - std::sqrt(delta)) / (2 * a);
    return {x1, x2};
}

int main()
{
    int N;
    std::cout << "Digite a quantidade de equacoes a serem resolvidas: ";
    std::cin >> N;

    std::vector<double> a(N), b(N), c(N);
    for (int i = 0; i < N; ++i)
    {
        a[i] = 1.0;
        if (i % 2 == 0) { b[i] = -7.0; c[i] = 10.0; } 
        else { b[i] = 2.0; c[i] = 5.0; }
    }

    
    int reais_ordered = 0, irreais_ordered = 0;
    std::cout << "\nOrdered\n";
    double inicio = omp_get_wtime();

#pragma omp parallel for ordered
    for (int i = 0; i < N; ++i)
    {
        auto res = resolver_bhaskara(a[i], b[i], c[i]);

 #pragma omp ordered
        {
            if (res.first == 0.0 && res.second == 0.0)
                irreais_ordered++;
            else
                reais_ordered++;

           /* std::cout << "Equacao " << i << ": ";
            if (res.first == 0.0 && res.second == 0.0)
                std::cout << "sem raizes reais\n";
            else
                std::cout << "x1 = " << res.first << ", x2 = " << res.second << "\n";*/
        }
    }

    double fim = omp_get_wtime();
    std::cout << "Resumo final: Reais = " << reais_ordered << ", Irreais = " << irreais_ordered << "\n";
    std::cout << "Tempo: " << (fim - inicio) << " segundos\n";

    
    std::cout << "\nBarrier\n";
    inicio = omp_get_wtime();

    std::vector<std::pair<double,double>> resultados(N); // vetor compartilhado para armazenar raízes
    int reais_barrier = 0, irreais_barrier = 0;

#pragma omp parallel
    {
        // ===== Fase 1: calcular as raízes =====
#pragma omp for
        for (int i = 0; i < N; ++i)
        {
            resultados[i] = resolver_bhaskara(a[i], b[i], c[i]);
        }

        // ===== Fase 2: esperar todas terminarem (barrier) e depois analisar =====
#pragma omp barrier
#pragma omp for reduction(+:reais_barrier, irreais_barrier)
        for (int i = 0; i < N; ++i)
        {
            if (resultados[i].first == 0.0 && resultados[i].second == 0.0)
                irreais_barrier++;
            else
                reais_barrier++;
        }
    }

    fim = omp_get_wtime();
    std::cout << "Resumo final: Reais = " << reais_barrier << ", Irreais = " << irreais_barrier << "\n";
    std::cout << "Tempo: " << (fim - inicio) << " segundos\n";

    return 0;
}
