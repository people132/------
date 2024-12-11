#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <algorithm>
using namespace std;
using namespace std::chrono;

// **Класс "Совпадение"**
class Match {
public:
    int position;        // Позиция совпадения
    double matchPercent; // Процент совпадения (0-100)
    bool isFullMatch;    // Полное совпадение или частичное

    Match(int pos, double percent, bool fullMatch)
        : position(pos), matchPercent(percent), isFullMatch(fullMatch) {}

    void print() const {
        cout << "Position: " << position
             << ", Match Percent: " << matchPercent
             << "%, Full Match: " << (isFullMatch ? "Yes" : "No") << endl;
    }
};

// **Функция для получения фрагмента текста вокруг совпадения**
string getCompactFragment(const string& text, int position, int matchLength, int contextSize = 10) {
    int start = max(0, position - contextSize); // Начало фрагмента
    int end = min((int)text.size(), position + matchLength + contextSize); // Конец фрагмента

    string fragment = text.substr(start, end - start);
    // Добавляем квадратные скобки вокруг найденного совпадения
    fragment.insert(position - start, "[");
    fragment.insert(position - start + matchLength + 1, "]");

    return fragment;
}

// **Алгоритм Рабина-Карпа**
vector<Match> rabinKarp(const string& text, const string& pattern) {
    const int PRIME = 101;
    int n = text.size();
    int m = pattern.size();
    vector<Match> matches;
    long long patternHash = 0, textHash = 0, power = 1;

    for (int i = 0; i < m - 1; i++) power = (power * PRIME);
    for (int i = 0; i < m; i++) {
        patternHash = (patternHash * PRIME + pattern[i]);
        textHash = (textHash * PRIME + text[i]);
    }

    for (int i = 0; i <= n - m; i++) {
        if (patternHash == textHash) {
            int matchCount = 0;
            for (int j = 0; j < m; j++) {
                if (text[i + j] == pattern[j]) matchCount++;
            }
            double matchPercent = (matchCount * 100.0) / m;
            bool isFullMatch = (matchCount == m);
            matches.emplace_back(i, matchPercent, isFullMatch);
        }
        if (i < n - m) {
            textHash = (textHash - text[i] * power) * PRIME + text[i + m];
        }
    }
    return matches;
}

// **Алгоритм Кнута-Морриса-Пратта (КМП)**
vector<Match> knuthMorrisPratt(const string& text, const string& pattern) {
    int n = text.size(), m = pattern.size();
    vector<Match> matches;
    vector<int> lps(m, 0); // Это таблица lps

    // Построение таблицы lps
    int j = 0;
    for (int i = 1; i < m; i++) {
        while (j > 0 && pattern[i] != pattern[j]) j = lps[j - 1];
        if (pattern[i] == pattern[j]) j++;
        lps[i] = j;
    }

    // Поиск совпадений
    j = 0;
    for (int i = 0; i < n; i++) {
        while (j > 0 && text[i] != pattern[j]) j = lps[j - 1];
        if (text[i] == pattern[j]) j++;
        if (j == m) {
            matches.emplace_back(i - m + 1, 100.0, true); // Полное совпадение
            j = lps[j - 1];
        }
    }
    return matches;
}

// **Алгоритм Бойера-Мура**
vector<Match> boyerMoore(const string& text, const string& pattern) {
    int n = text.size(), m = pattern.size();
    vector<Match> matches;

    if (m > n) return matches;

    unordered_map<char, int> badChar;
    for (int i = 0; i < m; i++) {
        badChar[pattern[i]] = i;
    }

    int shift = 0;
    while (shift <= n - m) {
        int j = m - 1;

        while (j >= 0 && pattern[j] == text[shift + j]) {
            j--;
        }

        if (j < 0) {
            matches.emplace_back(shift, 100.0, true);
            shift += (shift + m < n) ? m - badChar[text[shift + m]] : 1;
        } else {
            shift += max(1, j - badChar[text[shift + j]]);
        }
    }
    return matches;
}

// **Вывод совпадений с компактным контекстом**
void printMatchesWithCompactContext(const vector<Match>& matches, const string& text, int matchLength) {
    for (const auto& match : matches) {
        match.print();
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