#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <map>
using namespace std;

// Функция для вычисления расстояния Левенштейна
int levenshteinDistance(const string& s1, const string& s2) {
    int n = s1.size(), m = s2.size();                       // Получаем длины строк
    vector<vector<int>> dp(n + 1, vector<int>(m + 1));      // Создаём DP-таблицу (n+1)x(m+1)

    for (int i = 0; i <= n; i++) dp[i][0] = i;              // Инициализация первой колонки
    for (int j = 0; j <= m; j++) dp[0][j] = j;              // Инициализация первой строки

    // Заполняем DP-таблицу
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
            if (s1[i - 1] == s2[j - 1]) {                   // Если символы совпадают
                dp[i][j] = dp[i - 1][j - 1];                // Берём диагональное значение
            } else {                                        // Если символы различаются
                dp[i][j] = 1 + min({dp[i - 1][j],           // Удаление
                                   dp[i][j - 1],           // Вставка
                                   dp[i - 1][j - 1]});     // Замена
            }
        }
    }

    return dp[n][m];                                       // Возвращаем расстояние Левенштейна
}

// Функция для вычисления процента совпадения
double calculateSimilarity(const string& text, const string& pattern) {
    int distance = levenshteinDistance(text, pattern);      // Вычисляем расстояние Левенштейна

    // Процент схожести рассчитывается относительно длины шаблона
    double similarity = (1.0 - static_cast<double>(distance) / pattern.size()) * 100.0;

    // Убедимся, что процент находится в диапазоне [0, 100]
    return max(0.0, min(100.0, similarity));                // Ограничиваем значение от 0 до 100
}

// Функция нечёткого поиска
vector<pair<int, double>> fuzzySearch(const string& text, const string& pattern, double threshold) {
    vector<pair<int, double>> allMatches;                  // Храним все совпадения (позиция, схожесть)
    int textLength = text.size();                          // Длина исходного текста
    int patternLength = pattern.size();                    // Длина искомого шаблона

    // Проходим по всем возможным подстрокам текста
    for (int i = 0; i < textLength; i++) {
        for (int len = 1; len <= patternLength && i + len <= textLength; len++) {
            string substring = text.substr(i, len);        // Извлекаем подстроку из текста
            double similarity = calculateSimilarity(substring, pattern); // Считаем схожесть

            // Если схожесть выше порога, добавляем результат
            if (similarity >= threshold) {
                allMatches.emplace_back(i, similarity);    // Добавляем позицию и схожесть в вектор
            }
        }
    }

    // Убираем пересекающиеся совпадения с выбором максимальной схожести
    map<int, double> bestMatches;                         // Map для хранения максимальной схожести по позиции
    for (const auto& match : allMatches) {
        int position = match.first;                       // Позиция совпадения
        double similarity = match.second;                 // Схожесть

        // Если для данной позиции схожесть выше предыдущей, обновляем значение
        if (bestMatches.find(position) == bestMatches.end() || bestMatches[position] < similarity) {
            bestMatches[position] = similarity;
        }
    }

    // Преобразуем map в vector
    vector<pair<int, double>> filteredMatches;           // Вектор для отфильтрованных совпадений
    for (const auto& match : bestMatches) {
        filteredMatches.emplace_back(match.first, match.second); // Добавляем в финальный результат
    }

    // Сортируем результат по позициям
    sort(filteredMatches.begin(), filteredMatches.end());
    return filteredMatches;                              // Возвращаем отсортированные совпадения
}

// Тестирование нечёткого поиска
void testFuzzySearch() {
    // Тест 1: Полные совпадения
    string text1 = "hello world hello";
    string pattern1 = "hello";
    double threshold1 = 80.0;
    cout << "\n=== Test 1: Full Matches ===" << endl;

    auto matches1 = fuzzySearch(text1, pattern1, threshold1);
    for (const auto& match : matches1) {
        cout << "Position: " << match.first
             << ", Similarity: " << fixed << setprecision(2)
             << match.second << "%" << endl;
    }

    // Тест 2: Без совпадений
    string text2 = "abcdefgh";
    string pattern2 = "xyz";
    double threshold2 = 50.0;
    cout << "\n=== Test 2: No Matches ===" << endl;

    auto matches2 = fuzzySearch(text2, pattern2, threshold2);
    if (matches2.empty()) {
        cout << "No matches found!" << endl;
    } else {
        for (const auto& match : matches2) {
            cout << "Position: " << match.first
                 << ", Similarity: " << fixed << setprecision(2)
                 << match.second << "%" << endl;
        }
    }

    // Тест 3: Частичные совпадения
    string text3 = "testing testy testers test";
    string pattern3 = "test";
    double threshold3 = 60.0;
    cout << "\n=== Test 3: Partial Matches ===" << endl;

    auto matches3 = fuzzySearch(text3, pattern3, threshold3);
    for (const auto& match : matches3) {
        cout << "Position: " << match.first
             << ", Similarity: " << fixed << setprecision(2)
             << match.second << "%" << endl;
    }

    // Тест 4: Длинный текст с несколькими совпадениями
    string text4 = "Lorem ipsum dolor sit amet, consectetur adipiscing elit.";
    string pattern4 = "ipsum";
    double threshold4 = 70.0;
    cout << "\n=== Test 4: Long Text ===" << endl;

    auto matches4 = fuzzySearch(text4, pattern4, threshold4);
    for (const auto& match : matches4) {
        cout << "Position: " << match.first
             << ", Similarity: " << fixed << setprecision(2)
             << match.second << "%" << endl;
    }

    // Тест 5: Частичный поиск короткого шаблона
    string text5 = "aaaaaa";
    string pattern5 = "aa";
    double threshold5 = 90.0;
    cout << "\n=== Test 5: Repeating Characters ===" << endl;

    auto matches5 = fuzzySearch(text5, pattern5, threshold5);
    for (const auto& match : matches5) {
        cout << "Position: " << match.first
             << ", Similarity: " << fixed << setprecision(2)
             << match.second << "%" << endl;
    }
}

int main() {
    testFuzzySearch();  // Запускаем тестирование функции нечёткого поиска
    return 0;           // Завершаем программу
}