#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <algorithm>
using namespace std;
using namespace std::chrono;

// **Алгоритм Рабина-Карпа**
vector<int> rabinKarp(const string& text, const string& pattern) {
    const int PRIME = 101;
    int n = text.size();
    int m = pattern.size();
    vector<int> result;
    long long patternHash = 0, textHash = 0, power = 1;

    for (int i = 0; i < m - 1; i++) power = (power * PRIME);
    for (int i = 0; i < m; i++) {
        patternHash = (patternHash * PRIME + pattern[i]);
        textHash = (textHash * PRIME + text[i]);
    }

    for (int i = 0; i <= n - m; i++) {
        if (patternHash == textHash && text.substr(i, m) == pattern) {
            result.push_back(i);
        }
        if (i < n - m) {
            textHash = (textHash - text[i] * power) * PRIME + text[i + m];
        }
    }
    return result;
}

// **Алгоритм Кнута-Морриса-Пратта (КМП)**
vector<int> knuthMorrisPratt(const string& text, const string& pattern) {
    int n = text.size(), m = pattern.size();
    vector<int> result, lps(m, 0);
    int j = 0;

    for (int i = 1; i < m; i++) {
        while (j > 0 && pattern[i] != pattern[j]) j = lps[j - 1];
        if (pattern[i] == pattern[j]) lps[i] = ++j;
    }

    j = 0;
    for (int i = 0; i < n; i++) {
        while (j > 0 && text[i] != pattern[j]) j = lps[j - 1];
        if (text[i] == pattern[j]) j++;
        if (j == m) {
            result.push_back(i - m + 1);
            j = lps[j - 1];
        }
    }
    return result;
}

// **Улучшенный алгоритм Бойера-Мура**
unordered_map<char, int> badCharacterTable(const string& pattern) {
    unordered_map<char, int> table;
    int m = pattern.size();
    for (int i = 0; i < m; i++) {
        table[pattern[i]] = i;
    }
    return table;
}

vector<int> goodSuffixTable(const string& pattern) {
    int m = pattern.size();
    vector<int> table(m + 1, m);
    vector<int> border(m + 1, 0);

    int j = m;
    border[m] = j;
    for (int i = m - 1; i >= 0; i--) {
        while (j < m && pattern[i] != pattern[j]) {
            if (table[j] == m) {
                table[j] = m - i - 1;
            }
            j = border[j];
        }
        j--;
        border[i] = j;
    }

    for (int i = 0; i < m; i++) {
        if (table[i] == m) {
            table[i] = m - j - 1;
        }
        j = border[j];
    }

    return table;
}

std::vector<int> computePrefixFunction(const std::string& s) {
    std::vector<int> p(s.length());
    int k = 0;
    p[0] = 0;
    for (size_t i = 1; i < s.length(); ++i) {
        while (k > 0 && s[k] != s[i]) {
            k = p[k - 1];
        }
        if (s[k] == s[i]) {
            ++k;
        }
        p[i] = k;
    }
    return p;
}

// Функция для поиска подстроки t в строке s с использованием алгоритма Бойера-Мура
int boyerMoore(const std::string& s, const std::string& t) {
    if (t.empty()) {
        return 0; // Пустой шаблон всегда "находится" в начале строки
    }
    if (s.length() < t.length()) {
        return -1; // Шаблон длиннее строки, поиск невозможен
    }

    // Построение таблицы стоп-символов
    std::unordered_map<char, int> stopTable;
    for (size_t i = 0; i < t.length(); ++i) {
        stopTable[t[i]] = i;
    }

    // Построение таблицы суффиксов
    std::string rt(t.rbegin(), t.rend());
    std::vector<int> p = computePrefixFunction(t);
    std::vector<int> pr = computePrefixFunction(rt);
    std::vector<int> suffixTable(t.length() + 1, t.length() - p.back());
    for (size_t i = 1; i < t.length(); ++i) {
        int j = pr[i];
        suffixTable[j] = std::min(suffixTable[j], static_cast<int>(i - pr[i] + 1));
    }

    // Поиск подстроки
    for (size_t shift = 0; shift <= s.length() - t.length();) {
        int pos = t.length() - 1;
        while (t[pos] == s[shift + pos]) {
            if (pos == 0) {
                return shift; // Найдено совпадение
            }
            --pos;
        }
        int stopShift = pos - (stopTable.count(s[shift + pos]) ? stopTable[s[shift + pos]] : -1);
        int suffixShift = suffixTable[t.length() - pos - 1];
        shift += std::max(stopShift, suffixShift);
    }
    return -1; // Совпадение не найдено
}
// **Тесты производительности**
void testPerformance(const string& text, const string& pattern) {
    cout << "\n--- Testing Performance ---" << endl;

    // Рабин-Карп
    auto start = high_resolution_clock::now();
    rabinKarp(text, pattern);
    auto end = high_resolution_clock::now();
    cout << "Rabin-Karp Duration: " << duration_cast<microseconds>(end - start).count() << " microseconds" << endl;

    // Кнут-Моррис-Пратт
    start = high_resolution_clock::now();
    knuthMorrisPratt(text, pattern);
    end = high_resolution_clock::now();
    cout << "KMP Duration: " << duration_cast<microseconds>(end - start).count() << " microseconds" << endl;

    // Бойера-Мура
    start = high_resolution_clock::now();
    boyerMoore(text, pattern);
    end = high_resolution_clock::now();
    cout << "Boyer-Moore Duration: " << duration_cast<microseconds>(end - start).count() << " microseconds" << endl;
}

int main() {
    // Тест 1: Маленький текст
    string text = "aaaaa";
    string pattern = "aaa";
    testPerformance(text, pattern);

    // Тест 2: Текст с длинным шаблоном
    text = "This is a simple test string for Boyer-Moore algorithm.";
    pattern = "Boyer-Moore";
    testPerformance(text, pattern);

    // Тест 3: Большой текст с несколькими совпадениями
    text = string(10000, 'a') + "test" + string(100, 'a') + "test" + string(10000, 'b');
    pattern = "test";
    testPerformance(text, pattern);

    // Тест 4: Текст без совпадений
    text = string(20000, 'a');
    pattern = "test";
    testPerformance(text, pattern);

    // Тест 5: Очень большой текст
    std::string text1(1000000, 'a');
    text1.replace(500000, 6, "pattern");
    pattern = "pattern";
    testPerformance(text1, pattern);

    return 0;
}