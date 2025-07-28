# Лабораторная работа 9

Шедулер задач

## Задача

Вашей задачей будет разработать класс отвечающий за выполнение связанных по данным между собой задач.
Часто, чтобы решить какую либо задачу, требуется выполнить граф вычислений, где узел графа это задача, ребро связь между результатом выполнения одной задачи и параметром для запуска другой. Вам предстоит разработать класс **TTaskScheduler** , решающий подобную задачу.

Пример такой задачи - нахождение корней квадратного уравнения (предполагаем что коэффициенты гарантирует что корня 2). Решение подобной задачи в лоб - подразумевает вычисление корня из дискриминанта дважды.  Очевидно, что это не оптимально.


Вот так мог бы выглядеть код для решение данной задачи с помощью TTaskScheduler

```cpp
struct AddNumber {
  float add(float a) const {
    return a + number;
  }

  float number;
};

float a = 1;
float b = -2;
float c = 0;
AddNumber add{
  .number = 3
};

TTaskScheduler scheduler;

auto id1 = scheduler.add([](float a, float c) {return -4 * a * c;}, a, c);

auto id2 = scheduler.add([](float b, float v) {return b * b + v;}, b, scheduler.getFutureResult<float>(id1));

auto id3 = scheduler.add([](float b, float d) {return -b + std::sqrt(d);}, b, scheduler.getFutureResult<float>(id2));

auto id4 = scheduler.add([](float b, float d) {return -b - std::sqrt(d);}, b, scheduler.getFutureResult<float>(id2));

auto id5 = scheduler.add([](float a, float v) {return v/(2*a);}, a, scheduler.getFutureResult<float>(id3));

auto id6 = scheduler.add([]{float a, float v} {return v/(2*a);}, a, scheduler.getFutureResult<float>(id4));

auto id7 = scheduler.add(&AddNumber::add, add, scheduler.getFutureResult<float>(id6));

scheduler.executeAll();

std::cout << "x1 = " << scheduler.getResult<float>(id5) << std::endl;
std::cout << "x2 = " << scheduler.getResult<float>(id6) << std::endl;
std::cout << "x3 = " << scheduler.getResult<float>(id7) << std::endl;
```

Где getFutureResult это результат выполнения задачи в будущем,

### Публичный интефейс TTaskScheduler

 - **add** - принимает в качестве аргумента задание для него. Возвращает объект описывающий добавленную таску.
 - **getFutureResult<T>** - возвращает объект, из которого в будущем можно получить результат задания, переданного в качестве результата типа Т
 - **getResult<T>** - возвращает результат выполнения задания определенного типа. Вычисляет его если оно еще не подсчитано, при это не происходит вычисления не нужных заданий
 - **executeAll** - выполняет все запланированные задания

### Требования и ограничения к заданиям

  - [Callable object](https://en.cppreference.com/w/cpp/named_req/Callable)
  - Количество аргументов не больше 2
  - Задание может быть указателем на метод класса. В таком случае первый аргумент является классом, от которого будет вызван метод. Сам класс тоже считается аргументом

## Ограничения

 Запрещено использовать стандартную библиотеку, за исключением [контейнеров](https://en.cppreference.com/w/cpp/container) и [умных указателей](https://en.cppreference.com/w/cpp/memory).

## Тесты

Все вышеуказанный класс должен быть покрыты тестами, с помощью фреймворка [Google Test](http://google.github.io/googletest).

Тесты также являются частью задания, поэтому покрытие будет влиять на максимальный балл.

## NB

1. В данной работе могут быть использованы идеи [Type Erasure](https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Type_Erasure) который мы разбирали на лекции
2. Мы поговорили с вам про std::forward, подумайте над тем где это может быть применимо в данной задаче
3. Получаемые расписания не всегда могут быть выполнимы, предлагается подумать что делать в таких ситуациях

## ТеорМин

1. Labmda
2. Type erasure (какую проблему решает, как реализуется, примеры из стандартной библиотеки)
3. Value categories
4. RValue reference
5. Move Semantics

## Deadline

1. 23.04.25 23:59 0.8
2. 30.04.25 23:59 0.65
3. 07.05.25 23:59 0.5

Максимальное количество баллов - 15
