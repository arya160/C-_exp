#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <string>
#include <functional>
#include <chrono>
#include "csv.hpp"  // Підключення бібліотеки csv-parser

static const int inter_num = 1000;
 
// Клас Tag
class Tag {
public:
    std::string id;
    std::string name;
    Tag(const std::string& id, const std::string& name) : id(id), name(name) {}
};
 
// Клас Question
class Question {
public:
    std::string id;
    std::string author_id;
    std::string date_added;
    std::string title;
    std::string body;
    std::list<Tag*> tags;  // Зв'язок з тегами
 
    Question(const std::string& id, const std::string& author_id, const std::string& date_added,
             const std::string& title, const std::string& body)
        : id(id), author_id(author_id), date_added(date_added), title(title), body(body) {}
};
 
// Клас Answer
class Answer {
public:
    std::string id;
    std::string question_id;
    std::string date_added;
    std::string body;
    Question* question = nullptr;  // Зв'язок з питанням
 
    Answer(const std::string& id, const std::string& question_id, const std::string& date_added, const std::string& body)
        : id(id), question_id(question_id), date_added(date_added), body(body) {}
};
 
// Глобальні структури
std::vector<Question> questions;
std::vector<Answer> answers;
std::vector<Tag> tags;
std::map<std::string, Question*> question_map_idx;
std::unordered_map<std::string, Question*> question_unordered_map_idx;
std::map<std::string, std::list<Tag*>> tag_map_idx;
std::unordered_map<std::string, std::list<Tag*>> tag_unordered_map_idx;
 
// Завантаження CSV за допомогою бібліотеки csv-parser
template <typename T>
std::vector<T> readCSV(const std::string& filename, std::function<T(const csv::CSVRow&)> factory) {
    std::vector<T> objects;
    try {
        csv::CSVReader reader(filename);
        for (csv::CSVRow& row : reader) {
            objects.push_back(factory(row));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error reading file " << filename << ": " << e.what() << std::endl;
    }
    return objects;
}
 
// Фабричні функції для створення об'єктів
Tag createTag(const csv::CSVRow& row) {
    return Tag(row["tags_tag_id"].get<std::string>(), row["tags_tag_name"].get<std::string>());
}
 
Question createQuestion(const csv::CSVRow& row) {
    return Question(row["questions_id"].get<std::string>(), row["questions_author_id"].get<std::string>(),
                    row["questions_date_added"].get<std::string>(), row["questions_title"].get<std::string>(),
                    row["questions_body"].get<std::string>());
}
 
Answer createAnswer(const csv::CSVRow& row) {
    return Answer(row["answers_id"].get<std::string>(), row["answers_question_id"].get<std::string>(),
                  row["answers_date_added"].get<std::string>(), row["answers_body"].get<std::string>());
}
 
// Функція для зчитування зв'язків питання/тег
void readTagQuestions(const std::string& filename) {
    try {
        csv::CSVReader reader(filename);
        for (csv::CSVRow& row : reader) {
            std::string question_id = row["tag_questions_question_id"].get<std::string>();
            std::string tag_id = row["tag_questions_tag_id"].get<std::string>();
            if (question_map_idx.find(question_id) != question_map_idx.end() && tag_map_idx.find(tag_id) != tag_map_idx.end()) {
                question_map_idx[question_id]->tags.push_back(tag_map_idx[tag_id].front());
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error reading file " << filename << ": " << e.what() << std::endl;
    }
}
 
// Побудова індексів
void buildMapIndexes() {
    for (auto& q : questions) {
        question_map_idx[q.id] = &q;
        question_unordered_map_idx[q.id] = &q;
    }
    for (auto& t : tags) {
        tag_map_idx[t.id].push_back(&t);
        tag_unordered_map_idx[t.id].push_back(&t);
    }
}
// Експерименти з холостим проходом
void emptyTraversalWithMapIndex() {
    volatile size_t totalAnswers = 0, totalQuestions = 0, totalTags = 0;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < inter_num; ++i) {
        for (const auto& answer : answers) {
            totalAnswers++;
            auto qi = question_map_idx.find(answer.question_id);
            if (qi != question_map_idx.end()) {
                auto q = qi->second;
                totalQuestions++;
                totalTags += q->tags.size();
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Map index traversal time: "
<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
<< " ms\n";
}
 
void emptyTraversalWithUnorderedMapIndex() {
    volatile size_t totalAnswers = 0, totalQuestions = 0, totalTags = 0;
    auto start = std::chrono::high_resolution_clock::now();
 
    for (int i = 0; i < inter_num; ++i) {
        for (const auto& answer : answers) {
            totalAnswers++;
            auto qi = question_unordered_map_idx.find(answer.question_id);
            if (qi != question_unordered_map_idx.end()) {
                auto q = qi->second;
                totalQuestions++;
                totalTags += q->tags.size();
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Unordered map index traversal time: "
<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
<< " ms\n";
}
 
void emptyTraversalWithDirectPointers() {
    volatile size_t totalAnswers = 0, totalQuestions = 0, totalTags = 0;
    auto start = std::chrono::high_resolution_clock::now();
 
    for (int i = 0; i < inter_num; ++i) {
        for (const auto& answer : answers) {
            totalAnswers++;
            if (answer.question != nullptr) {
                totalQuestions++;
                totalTags += answer.question->tags.size();
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Direct pointer traversal time: "
<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
<< " ms\n";
    std::cout << "Total answers: " << totalAnswers << ", total questions: "
        << totalQuestions << ", total tags: " << totalTags << std::endl;
}
 
// Експерименти з конкатенацією рядків
void traversalWithStringConcatMapIndex() {
    std::string result;  // Стандартна змінна для збереження кінцевого результату
    auto start = std::chrono::high_resolution_clock::now();
 
    for (int i = 0; i < inter_num; ++i) {
        for (const auto& answer : answers) {
            auto qi = question_map_idx.find(answer.question_id);
            if (qi != question_map_idx.end()) {
                auto q = qi->second;
                result += q->title + q->body;
                for (const auto& tag : q->tags) {
                    result += tag->name;
                }
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "String concat map index traversal time: "
<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
<< " ms\n";
    std::cout << "Result length: " << result.size() << std::endl;
}
 
void traversalWithStringConcatUnorderedMapIndex() {
    std::string result;  // Стандартна змінна для збереження кінцевого результату
    auto start = std::chrono::high_resolution_clock::now();
 
    for (int i = 0; i < inter_num; ++i) {
        for (const auto& answer : answers) {
            auto qi = question_unordered_map_idx.find(answer.question_id);
            if (qi != question_unordered_map_idx.end()) {
                auto q = qi->second;
                result += q->title;
                result += q->body;
                for (const auto& tag : q->tags) {
                    result += tag->name;
                }
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "String concat unordered_map index traversal time: "
<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
<< " ms\n";
    std::cout << "Result length: " << result.size() << std::endl;
}
 
void traversalWithStringConcatDirectPointers() {
    std::string result;  // Стандартна змінна для збереження кінцевого результату
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < inter_num; ++i) {
        for (const auto& answer : answers) {
            if (answer.question != nullptr) {
                result += answer.question->title;
                result += answer.question->body;
                for (const auto& tag : answer.question->tags) {
                    result += tag->name;
                }
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "String concat direct pointer traversal time: "
<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
<< " ms\n";
    std::cout << "Result length: " << result.size() << std::endl;
}
 
static void fillPointers() {
    for (auto& answer: answers) {
        if (question_unordered_map_idx.find(answer.question_id) != question_unordered_map_idx.end()) {
            answer.question = question_unordered_map_idx[answer.question_id];
        }
    }
}

int main(int argc, char **argv) {
    std::string prefix;

    if (argc > 1) {
        prefix = argv[1];
        prefix += "/";
    }

    // Завантаження даних
    questions = readCSV<Question>(prefix + "questions.csv", createQuestion);
    answers = readCSV<Answer>(prefix + "answers.csv", createAnswer);
    tags = readCSV<Tag>(prefix + "tags.csv", createTag);
 
    // Побудова індексів та зчитування зв'язків
    buildMapIndexes();
    readTagQuestions(prefix + "tag_questions.csv");
    fillPointers();

    // Запуск експериментів
    emptyTraversalWithMapIndex();
    emptyTraversalWithUnorderedMapIndex();
    emptyTraversalWithDirectPointers();
    traversalWithStringConcatMapIndex();
    traversalWithStringConcatUnorderedMapIndex();
    traversalWithStringConcatDirectPointers();
 
    return 0;
}