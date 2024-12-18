#include <iostream>          // Библиотека для ввода и вывода данных
#include <string>            // Библиотека для работы со строками
#include <vector>            // Библиотека для использования динамических массивов (векторов)
#include <unordered_map>     // Библиотека для использования хэш-таблиц
#include <chrono>            // Библиотека для замера времени выполнения кода
#include <algorithm>         // Библиотека для стандартных алгоритмов (например, max и min)

using namespace std;         // Использование пространства имен std для сокращения записи
using namespace std::chrono; // Использование пространства имен chrono для удобного доступа к измерениям времени

// **Класс "Совпадение"** - представляет информацию о найденном совпадении
class Match {
public:
    int position;        // Позиция начала совпадения в тексте
    double matchPercent; // Процент совпадения (0-100)
    bool isFullMatch;    // Полное совпадение или нет (true/false)

    // Конструктор класса Match для инициализации переменных
    Match(int pos, double percent, bool fullMatch)
        : position(pos), matchPercent(percent), isFullMatch(fullMatch) {}

    // Метод для вывода информации о совпадении
    void print() const {
        cout << "Position: " << position
             << ", Match Percent: " << matchPercent
             << "%, Full Match: " << (isFullMatch ? "Yes" : "No") << endl;
    }
};

// **Функция для получения фрагмента текста вокруг совпадения**
string getCompactFragment(const string& text, int position, int matchLength, int contextSize = 10) {
    int start = max(0, position - contextSize); // Определение начала фрагмента (с учетом границ текста)
    int end = min((int)text.size(), position + matchLength + contextSize); // Конец фрагмента

    // Извлечение фрагмента текста вокруг совпадения
    string fragment = text.substr(start, end - start);

    // Вставка квадратных скобок вокруг найденного совпадения
    fragment.insert(position - start, "[");
    fragment.insert(position - start + matchLength + 1, "]");

    return fragment; // Возвращаем отформатированный фрагмент
}

// **Алгоритм Рабина-Карпа**
vector<Match> rabinKarp(const string& text, const string& pattern) {
    const int PRIME = 101; // Простое число для расчета хэша
    int n = text.size();   // Длина текста
    int m = pattern.size(); // Длина шаблона
    vector<Match> matches;  // Вектор для хранения совпадений
    long long patternHash = 0, textHash = 0, power = 1; // Переменные для хранения хэшей и степеней

    // Вычисление степени PRIME^(m-1)
    for (int i = 0; i < m - 1; i++) power = (power * PRIME);

    // Вычисление хэша для шаблона и первого окна текста
    for (int i = 0; i < m; i++) {
        patternHash = (patternHash * PRIME + pattern[i]);
        textHash = (textHash * PRIME + text[i]);
    }

    // Сканирование текста для поиска совпадений
    for (int i = 0; i <= n - m; i++) {
        if (patternHash == textHash) { // Проверка совпадения хэшей
            int matchCount = 0; // Счетчик совпадающих символов
            for (int j = 0; j < m; j++) {
                if (text[i + j] == pattern[j]) matchCount++;
            }
            double matchPercent = (matchCount * 100.0) / m; // Расчет процента совпадения
            bool isFullMatch = (matchCount == m);          // Полное совпадение
            matches.emplace_back(i, matchPercent, isFullMatch); // Добавление совпадения
        }
        // Пересчет хэша для следующего окна
        if (i < n - m) {
            textHash = (textHash - text[i] * power) * PRIME + text[i + m];
        }
    }
    return matches; // Возвращение списка совпадений
}

// **Алгоритм Кнута-Морриса-Пратта (КМП)**
vector<Match> knuthMorrisPratt(const string& text, const string& pattern) {
    int n = text.size(), m = pattern.size();
    vector<Match> matches;     // Вектор для хранения совпадений
    vector<int> lps(m, 0);     // Таблица LPS (Longest Prefix Suffix)

    // Построение таблицы lps
    int j = 0; // Индекс для совпадающего префикса
    for (int i = 1; i < m; i++) {
        while (j > 0 && pattern[i] != pattern[j]) j = lps[j - 1];
        if (pattern[i] == pattern[j]) j++;
        lps[i] = j;
    }

    // Поиск совпадений
    j = 0; // Индекс для шаблона
    for (int i = 0; i < n; i++) {
        while (j > 0 && text[i] != pattern[j]) j = lps[j - 1];
        if (text[i] == pattern[j]) j++;
        if (j == m) { // Найдено полное совпадение
            matches.emplace_back(i - m + 1, 100.0, true); // Добавляем совпадение
            j = lps[j - 1]; // Продолжаем поиск
        }
    }
    return matches; // Возвращаем список совпадений
}

// **Алгоритм Бойера-Мура**
vector<Match> boyerMoore(const string& text, const string& pattern) {
    int n = text.size(), m = pattern.size();
    vector<Match> matches;

    if (m > n) return matches; // Если шаблон длиннее текста, возвращаем пустой список

    unordered_map<char, int> badChar; // Таблица для правила плохого символа
    for (int i = 0; i < m; i++) {
        badChar[pattern[i]] = i; // Сохраняем индексы символов шаблона
    }

    int shift = 0; // Сдвиг окна поиска
    while (shift <= n - m) {
        int j = m - 1;

        while (j >= 0 && pattern[j] == text[shift + j]) j--; // Сравнение символов

        if (j < 0) { // Полное совпадение
            matches.emplace_back(shift, 100.0, true);
            shift += (shift + m < n) ? m - badChar[text[shift + m]] : 1;
        } else {
            shift += max(1, j - badChar[text[shift + j]]);
        }
    }
    return matches; // Возвращаем список совпадений
}

// **Вывод совпадений с компактным контекстом**
void printMatchesWithCompactContext(const vector<Match>& matches, const string& text, int matchLength) {
    for (const auto& match : matches) {
        match.print(); // Вывод информации о совпадении
        string fragment = getCompactFragment(text, match.position, matchLength);
        cout << "Context: " << fragment << endl;
    }
}

// **Тест производительности**
void testPerformanceWithCompactContext(const string& text, const string& pattern) {
    cout << "\n--- Testing Performance with Compact Context ---" << endl;

    // Рабин-Карп
    auto start = high_resolution_clock::now();
    auto matchesRK = rabinKarp(text, pattern);
    auto end = high_resolution_clock::now();
    cout << "Rabin-Karp Duration: " << duration_cast<microseconds>(end - start).count() << " microseconds" << endl;
    printMatchesWithCompactContext(matchesRK, text, pattern.size());

    // Кнут-Моррис-Пратт
    start = high_resolution_clock::now();
    auto matchesKMP = knuthMorrisPratt(text, pattern);
    end = high_resolution_clock::now();
    cout << "Knuth-Morris-Pratt Duration: " << duration_cast<microseconds>(end - start).count() << " microseconds" << endl;
    printMatchesWithCompactContext(matchesKMP, text, pattern.size());

    // Бойера-Мура
    start = high_resolution_clock::now();
    auto matchesBM = boyerMoore(text, pattern);
    end = high_resolution_clock::now();
    cout << "Boyer-Moore Duration: " << duration_cast<microseconds>(end - start).count() << " microseconds" << endl;
    printMatchesWithCompactContext(matchesBM, text, pattern.size());
}

int main() {
    // Пример текста и шаблона
    string text = "This is a simple test example with some errors and a test.";
    string pattern = "test";

    cout << "=== Test 1: Basic Example ===" << endl;
    testPerformanceWithCompactContext(text, pattern);

    // Очень большой текст
    cout << "\n=== Test 2: Very Large Text with One Match ===" << endl;
    string largeText(1000000, 'a');
    largeText.replace(500000, 6, "pattern");
    pattern = "pattern";
    testPerformanceWithCompactContext(largeText, pattern);

    // Текст с множественными совпадениями
    cout << "\n=== Test 3: Text with Multiple Matches ===" << endl;
    string multipleMatchesText = "test test test test";
    pattern = "test";
    testPerformanceWithCompactContext(multipleMatchesText, pattern);

    // Частичное совпадение (неполное соответствие)
    cout << "\n=== Test 4: Text with Partial Matches ===" << endl;
    string partialText = "testing testy testers test";
    pattern = "test";
    testPerformanceWithCompactContext(partialText, pattern);

    // Нет совпадений
    cout << "\n=== Test 5: Text with No Matches ===" << endl;
    string noMatchText = "completely unrelated text with no pattern";
    pattern = "pattern";
    testPerformanceWithCompactContext(noMatchText, pattern);

    // Маленький шаблон в большом тексте
    cout << "\n=== Test 6: Small Pattern in Large Text ===" << endl;
    string smallPatternText = string(500000, 'x') + "a" + string(500000, 'x');
    pattern = "a";
    testPerformanceWithCompactContext(smallPatternText, pattern);

    return 0;
}