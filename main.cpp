#include <iostream>
#include <cmath>
#include <fstream>
#include <cstdlib>
#include <windows.h>
#include <clocale>

using namespace std;

// Тут просто лимиты, чтобы массивы не улетели
const int MAX_TOCHEK = 500;
const int MAX_TREUG = 3000;
const long double EPS = 1e-12;

// Обычная точка на плоскости
struct Tochka {
    long double x;
    long double y;
};

// Треугольник храним через индексы точек
struct Treug {
    int a;
    int b;
    int c;
    bool est; // true = треугольник пока живой
};

Tochka tochki[MAX_TOCHEK];
Treug treugi[MAX_TREUG];

int kolvoToch = 0;
int kolvoTreug = 0;

// Проверка направления (левый/правый поворот)
long double orient(Tochka a, Tochka b, Tochka c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

// Проверяем, попала ли точка p внутрь описанной окружности треугольника abc
long double vOpisOkruzh(Tochka a, Tochka b, Tochka c, Tochka p) {
    long double ax = a.x - p.x;
    long double ay = a.y - p.y;
    long double bx = b.x - p.x;
    long double by = b.y - p.y;
    long double cx = c.x - p.x;
    long double cy = c.y - p.y;

    return (ax * ax + ay * ay) * (bx * cy - by * cx)
         - (bx * bx + by * by) * (ax * cy - ay * cx)
         + (cx * cx + cy * cy) * (ax * by - ay * bx);
}

// Добавляем треугольник и разворачиваем вершины, если порядок не тот
void dobavitTreug(int a, int b, int c) {
    if (orient(tochki[a], tochki[b], tochki[c]) < 0) {
        int tmp = b;
        b = c;
        c = tmp;
    }

    treugi[kolvoTreug].a = a;
    treugi[kolvoTreug].b = b;
    treugi[kolvoTreug].c = c;
    treugi[kolvoTreug].est = true;
    kolvoTreug++;
}

// строим триангуляцию Делоне 
void postroitDelone() {
    long double minx = tochki[0].x;
    long double maxx = tochki[0].x;
    long double miny = tochki[0].y;
    long double maxy = tochki[0].y;

    for (int i = 0; i < kolvoToch; i++) {
        minx = min(minx, tochki[i].x);
        maxx = max(maxx, tochki[i].x);
        miny = min(miny, tochki[i].y);
        maxy = max(maxy, tochki[i].y);
    }

    long double dx = maxx - minx;
    long double dy = maxy - miny;
    long double d = max(dx, dy);

    // Добавляем супер-треугольник, который покрывает все точки
    tochki[kolvoToch++] = {minx - 10 * d, miny - d};
    tochki[kolvoToch++] = {maxx + 10 * d, miny - d};
    tochki[kolvoToch++] = {(minx + maxx) / 2, maxy + 10 * d};

    int sa = kolvoToch - 3;
    int sb = kolvoToch - 2;
    int sc = kolvoToch - 1;

    dobavitTreug(sa, sb, sc);

    // Вставляем точки по одной
    for (int p = 0; p < kolvoToch - 3; p++) {
        bool plohie[MAX_TREUG] = {0};

        // Ищем плохие треугольники (чья окружность содержит новую точку)
        for (int i = 0; i < kolvoTreug; i++) {
            if (!treugi[i].est) {
                continue;
            }

            Treug t = treugi[i];
            if (vOpisOkruzh(tochki[t.a], tochki[t.b], tochki[t.c], tochki[p]) > EPS) {
                plohie[i] = true;
                treugi[i].est = false;
            }
        }

        // Собираем ребра удаленных треугольников
        int u[2 * MAX_TREUG];
        int v[2 * MAX_TREUG];
        int kolvoReber = 0;

        for (int i = 0; i < kolvoTreug; i++) {
            if (!plohie[i]) {
                continue;
            }

            int a = treugi[i].a;
            int b = treugi[i].b;
            int c = treugi[i].c;

            u[kolvoReber] = a; v[kolvoReber++] = b;
            u[kolvoReber] = b; v[kolvoReber++] = c;
            u[kolvoReber] = c; v[kolvoReber++] = a;
        }

        // Внутренние ребра встречаются дважды в обратном порядке
        bool vnutr[2 * MAX_TREUG] = {0};
        for (int i = 0; i < kolvoReber; i++) {
            for (int j = i + 1; j < kolvoReber; j++) {
                if (u[i] == v[j] && v[i] == u[j]) {
                    vnutr[i] = true;
                    vnutr[j] = true;
                }
            }
        }

        // По граничным ребрам собираем новые треугольники
        for (int i = 0; i < kolvoReber; i++) {
            if (!vnutr[i]) {
                dobavitTreug(u[i], v[i], p);
            }
        }
    }

    // Удаляем все треугольники, которые используют вершины супер-треугольника
    for (int i = 0; i < kolvoTreug; i++) {
        if (!treugi[i].est) {
            continue;
        }

        if (treugi[i].a >= sa || treugi[i].b >= sa || treugi[i].c >= sa) {
            treugi[i].est = false;
        }
    }
}

// Сохраняем сетку в svg, чтобы открыть как картинку
void sohranitSvg() {
    ofstream out("out.svg");
    out << "<svg xmlns='http://www.w3.org/2000/svg' width='800' height='800'>\n";

    for (int i = 0; i < kolvoTreug; i++) {
        if (!treugi[i].est) {
            continue;
        }

        Tochka a = tochki[treugi[i].a];
        Tochka b = tochki[treugi[i].b];
        Tochka c = tochki[treugi[i].c];

        out << "<polygon points='"
            << a.x << "," << 800 - a.y << " "
            << b.x << "," << 800 - b.y << " "
            << c.x << "," << 800 - c.y
            << "' fill='none' stroke='black'/>\n";
    }

    // Рисуем сами точки
    for (int i = 0; i < kolvoToch; i++) {
        out << "<circle cx='" << tochki[i].x
            << "' cy='" << 800 - tochki[i].y
            << "' r='3' fill='red'/>\n";
    }

    out << "</svg>";
    out.close();
}

int main() {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
    setlocale(LC_ALL, ".UTF-8");

    kolvoToch = 0;

    // Если хотим каждый запуск разные точки, ставим srand(time(0));
    srand(1);

    for (int i = 0; i < 100; i++) {
        tochki[kolvoToch++] = {
            (long double)(rand() % 700 + 50),
            (long double)(rand() % 700 + 50)
        };
    }

    postroitDelone();
    sohranitSvg();

    cout << "Готово. Результат сохранен в out.svg\n";
    return 0;
}
