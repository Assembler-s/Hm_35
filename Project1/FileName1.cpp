#include <iostream>
#include <windows.h>
#include <string>
#include <stdexcept>

using namespace std;

// ==================== ГОТОВІ КЛАСИ ====================

class Starship
{
private:
    string name;
    int maxLoad;                          // вантажопідйомність, т
    int load;                             // поточний вантаж, т
    int fuel;                             // пальне, %

public:
    Starship(const string& name, int maxLoad, int fuel) : name(name), maxLoad(maxLoad), load(0), fuel(fuel)
    {
        if (maxLoad <= 0)
            throw invalid_argument("Вантажопідйомність має бути додатною");
    }

    string getName() const
    {
        return name;
    }

    void loadCargo(const string& item, int weight)
    {
        if (weight <= 0)
            throw invalid_argument("Вага вантажу має бути додатною");

        if (load + weight > maxLoad)
            throw runtime_error("Перевантаження: " + item + " не влазить");

        load += weight;
        cout << "  Завантажено: " << item << " (" << weight << " т), разом " << load << "/" << maxLoad << " т" << endl;
    }

    void launch()
    {
        if (fuel < 50)
            throw runtime_error("Недостатньо пального для старту");

        fuel -= 50;
        cout << name << ": Старт! На борту " << load << " т, залишок пального " << fuel << "%" << endl;
    }
};

// Шлюз: конструктор відкриває, деструктор закриває
class Airlock
{
private:
    string shipName;

public:
    explicit Airlock(const string& shipName) : shipName(shipName)
    {
        cout << "  [шлюз " << shipName << " відкрито]" << endl;
    }

    ~Airlock()
    {
        cout << "  [шлюз " << shipName << " закрито]" << endl;
    }
};

// ===== Ваш код =====

void loadManifest(Starship& ship, const string items[], const int weights[], int count)
{
    Airlock airlock{ ship.getName() };

    for (int i = 0; i < count; i++)
    {
        ship.loadCargo(items[i], weights[i]);
    }
}

int main()
{
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    try
    {
        Starship ghost{ "Привид", 0, 100 };
    }
    catch (const invalid_argument& e)
    {
        cout << "Корабель не створено: " << e.what() << endl;
    }

    Starship dream{ "Мрія", 100, 60 };

    try
    {
        dream.loadCargo("Продукти", 30);
        dream.loadCargo("Вода", 40);
        dream.loadCargo("Руда", 50);
        dream.launch();
    }
    catch (const invalid_argument& e)
    {
        cout << "Некоректні дані: " << e.what() << endl;
    }
    catch (const runtime_error& e)
    {
        cout << "Помилка польоту: " << e.what() << endl;
    }

    cout << "Рейс продовжується" << endl;

    try
    {
        dream.launch();
        dream.launch();
    }
    catch (const exception& e)
    {
        cout << "Помилка: " << e.what() << endl;
    }

    Starship freighter{ "Вантажник-7", 120, 100 };

    const string firstItems[] = { "Пошта", "Запчастини" };
    const int firstWeights[] = { 10, 20 };
    loadManifest(freighter, firstItems, firstWeights, 2);

    const string secondItems[] = { "Генератор", "Броня", "Супутник" };
    const int secondWeights[] = { 50, 40, 30 };

    try
    {
        loadManifest(freighter, secondItems, secondWeights, 3);
    }
    catch (const exception& e)
    {
        cout << "Центр управління: " << e.what() << endl;
    }

    return 0;
}

/*
  ВІДПОВІДІ НА ЧАСТИНУ «ПОЯСНИ»

  1. Исключение - это способ сообщить об ошибке, с которой код на месте не может справиться: выполнение
     прыгает вверх по цепочке вызовов к тому, кто умеет её обработать. loadCargo бросает исключение, а не
     возвращает false, потому что false легко проигнорировать: забыл проверить - и программа молча
     работает дальше с неправильным состоянием. Исключение проигнорировать нельзя: если его никто не
     поймал, программа завершится. Конструктор ничего не возвращает, поэтому сообщить через return он не
     может. Без исключений пришлось бы создавать "полуживой" корабль с флагом isValid() и надеяться, что
     его проверят, или делать отдельную функцию-фабрику, которая вернёт nullptr при ошибке. С исключением
     объект просто не создаётся.

  2. try - это блок, в котором мы ждём ошибку. throw создаёт объект-исключение и немедленно выходит из
     текущего места. catch - обработчик, который это исключение ловит. В шаге 2 выполнились
     "Продукты" (30 т) и "Вода" (разом 70 т). На "Руде" 70 + 50 > 100, и loadCargo бросила
     runtime_error до строки load += weight, поэтому руда не загрузилась. Старт не выполнился вообще.
     Назад в try программа не вернулась, потому что throw - это не вызов функции, из которого можно
     вернуться: оставшаяся часть try пропускается, управление уходит в подходящий catch, а после него
     продолжается код за всем try/catch (поэтому выводится "Рейс продовжується").

  3. По ссылке - чтобы не копировать объект исключения, а главное, чтобы не было срезки (slicing): если
     поймать exception по значению, то из runtime_error получится копия только базовой части, и
     сообщение потеряется. const - потому что мы исключение только читаем, менять его не собираемся.
     Один catch (const exception& e) ловит и invalid_argument, и runtime_error, потому что оба
     наследуются от exception (invalid_argument через logic_error), а обработчик для базового класса
     подходит для любого наследника. Метод what() виртуальный, поэтому вызывается версия с настоящим
     сообщением.

  4. Раскрутка стека - это то, что происходит при выбросе исключения: программа по одной выходит из
     функций, в которых нет подходящего catch, и по пути уничтожает все локальные объекты этих функций
     в порядке, обратном созданию. Шлюз - локальный объект в loadManifest. Когда внутри неё вылетело
     исключение, функция завершилась не обычным способом, но деструктор Airlock всё равно вызвался,
     поэтому "шлюз закрито" выводится раньше, чем main дошёл до catch и написал "Центр управління".
     Это и называется RAII: ресурс освобождается в деструкторе, и это гарантированно срабатывает даже
     при ошибке.

*/