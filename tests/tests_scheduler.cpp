#include <gtest/gtest.h>
#include <string>
#include "scheduler.h"

// Хелпер — из любого значения делаем Input::makeValue
auto V = [](auto x){ return Arg(x); };

// 1) Одиночная задача без аргументов
TEST(TaskScheduler15, SingleNoArgs) {
  TTaskScheduler s;
  auto id = s.add([] { return 99; });
  EXPECT_EQ(s.getResult<int>(id), 99);
}

// 2) Две задачи с зависимостью
TEST(TaskScheduler15, TwoTasksDependency) {
  TTaskScheduler s;
  auto id1 = s.add([](int a, int b){ return a + b; }, V(3), V(4));
  auto id2 = s.add([](int v){ return v * 10; },
                   s.getFutureResult<int>(id1));
  EXPECT_EQ(s.getResult<int>(id2), 70);
}

// 3) Цепочка глубиной 3
TEST(TaskScheduler15, DeepChain) {
  TTaskScheduler s;
  auto a = s.add([] { return 2; });
  auto b = s.add([](int x){ return x + 5; }, s.getFutureResult<int>(a));
  auto c = s.add([](int x){ return x * x; }, s.getFutureResult<int>(b));
  EXPECT_EQ(s.getResult<int>(c), 49);
}

// 4) Разветвлённый граф
TEST(TaskScheduler15, Branching) {
  TTaskScheduler s;
  auto base  = s.add([] { return 3; });
  auto left  = s.add([](int v){ return v + 1; }, s.getFutureResult<int>(base));
  auto right = s.add([](int v){ return v - 1; }, s.getFutureResult<int>(base));
  auto sum   = s.add([](int l, int r){ return l + r; },
                    s.getFutureResult<int>(left),
                    s.getFutureResult<int>(right));
  EXPECT_EQ(s.getResult<int>(sum), 6);
}

// 5) Ленивое вычисление
TEST(TaskScheduler15, Lazy) {
  TTaskScheduler s;
  int counter = 0;
  auto id = s.add([&]{ ++counter; return 1; });
  EXPECT_EQ(counter, 0);
  s.getResult<int>(id);
  EXPECT_EQ(counter, 1);
}

// 6) Кеш: вычисляется только один раз
TEST(TaskScheduler15, Cache) {
  TTaskScheduler s;
  int hits = 0;
  auto id = s.add([&]{ ++hits; return 8; });
  s.getResult<int>(id);
  s.getResult<int>(id);
  EXPECT_EQ(hits, 1);
}

// 7) Работа с double
TEST(TaskScheduler15, DoubleType) {
  TTaskScheduler s;
  auto id = s.add([]{ return 0.1 + 0.2; });
  EXPECT_DOUBLE_EQ(s.getResult<double>(id), 0.3);
}

// 8) Работа со строкой
TEST(TaskScheduler15, StringType) {
  TTaskScheduler s;
  auto id = s.add([]{ return std::string("hello"); });
  EXPECT_EQ(s.getResult<std::string>(id), "hello");
}

// 9) Смешанные типы: строка → int → умножаем на double
TEST(TaskScheduler15, MixedTypesMath) {
  TTaskScheduler s;
  auto idStr = s.add([]{ return std::string("7"); });
  auto idInt = s.add([](const std::string& t){ return std::stoi(t); },
                     s.getFutureResult<std::string>(idStr));
  auto idMul = s.add([](int v, double k){ return v * k; },
                     s.getFutureResult<int>(idInt),
                     V(2.5));
  EXPECT_DOUBLE_EQ(s.getResult<double>(idMul), 17.5);
}

// 10) Пользовательская структура
struct Vec2 { double x,y; };
TEST(TaskScheduler15, CustomStruct) {
  TTaskScheduler s;
  auto id = s.add([](Vec2 v){ return v.x*v.x + v.y*v.y; },
                  V(Vec2{3,4}));
  EXPECT_DOUBLE_EQ(s.getResult<double>(id), 25.0);
}

// 11) Указатель на метод
struct Adder {
  int add(int a) const { return a + bias; }
  int bias{5};
};
TEST(TaskScheduler15, MethodPointer) {
  TTaskScheduler s;
  Adder obj{3};
  auto id = s.add(&Adder::add, obj, V(7));
  EXPECT_EQ(s.getResult<int>(id), 10);
}

// 12) Большие числа и точность
TEST(TaskScheduler15, LargeNumbers) {
  TTaskScheduler s;
  auto id = s.add([]{ return 1e12 + 3.0; });
  EXPECT_DOUBLE_EQ(s.getResult<double>(id), 1e12 + 3.0);
}

// 13) Несколько независимых задач
TEST(TaskScheduler15, IndependentTasks) {
  TTaskScheduler s;
  auto a = s.add([]{ return 1; });
  auto b = s.add([]{ return 2; });
  auto c = s.add([]{ return 3; });
  EXPECT_EQ(s.getResult<int>(a)
            + s.getResult<int>(b)
            + s.getResult<int>(c), 6);
}

// 14) future в разных позициях
TEST(TaskScheduler15, FutureAsAnyPosition) {
  TTaskScheduler s;
  auto base = s.add([]{ return 10; });
  auto id = s.add([](int x,int y){ return x - y; },
                  s.getFutureResult<int>(base),
                  V(3));
  EXPECT_EQ(s.getResult<int>(id), 7);
}

// 15) Последовательные задачи с разными типами
TEST(TaskScheduler15, SequentialVariousTypes) {
  TTaskScheduler s;
  // int → double → bool → std::string
  auto id1 = s.add([] { return 4; });
  auto id2 = s.add([](int x) { return x + 0.5; },
                   s.getFutureResult<int>(id1));                 // 4.5
  auto id3 = s.add([](double d) { return d >= 4.5; },          // теперь true
                   s.getFutureResult<double>(id2));
  auto id4 = s.add([](bool b) { return b ? std::string("yes")
                                        : std::string("no"); },
                   s.getFutureResult<bool>(id3));
  EXPECT_EQ(s.getResult<std::string>(id4), "yes");
}



//16
TEST(TTaskSchedulerTest, RemoveTaskMethod) {
  TTaskScheduler scheduler;

  // Добавляем несколько задач
  auto id1 = scheduler.add([]() { return 42; });
  auto id2 = scheduler.add([](int x) { return x * 2; }, 21);

  // Проверяем, что задачи добавлены
  ASSERT_TRUE(scheduler.hasTask(id1));
  ASSERT_TRUE(scheduler.hasTask(id2));

  // Удаляем первую задачу
  scheduler.removeTask(id1);

  // Проверяем, что первая задача удалена
  ASSERT_FALSE(scheduler.hasTask(id1));
  ASSERT_TRUE(scheduler.hasTask(id2));

  // Проверяем, что удаление несуществующей задачи вызывает исключение
  ASSERT_THROW(scheduler.removeTask(id1), std::out_of_range);
}


//17
TEST(TTaskSchedulerTest, HasTaskMethod) {
  TTaskScheduler scheduler;

  // Добавляем задачу
  auto id = scheduler.add([]() { return 42; });

  // Проверяем, что задача существует
  ASSERT_TRUE(scheduler.hasTask(id));

  // Удаляем задачу
  scheduler.removeTask(id);

  // Проверяем, что задача больше не существует
  ASSERT_FALSE(scheduler.hasTask(id));
}



//19
TEST(TTaskSchedulerTest, MultiArgumentTask) {
  TTaskScheduler scheduler;

  // Добавляем задачу с двумя аргументами
  auto id = scheduler.add([](int a, int b) { return a * b; }, 3, 4);

  // Проверяем результат
  EXPECT_EQ(scheduler.getResult<int>(id), 12);
}

//20
TEST(TTaskSchedulerTest, FutureResultAsArgument) {
  TTaskScheduler scheduler;

  // Добавляем первую задачу
  auto id1 = scheduler.add([]() { return 10; });

  // Добавляем вторую задачу, которая зависит от первой
  auto id2 = scheduler.add([](int x) { return x * 2; }, scheduler.getFutureResult<int>(id1));

  // Проверяем результат второй задачи
  EXPECT_EQ(scheduler.getResult<int>(id2), 20);
}

//21
TEST(TTaskSchedulerTest, MultipleFutureResults) {
  TTaskScheduler scheduler;

  // Добавляем задачи
  auto id1 = scheduler.add([]() { return 5; });
  auto id2 = scheduler.add([]() { return 3; });

  // Добавляем задачу, которая зависит от двух предыдущих
  auto id3 = scheduler.add([](int a, int b) { return a + b; },
                           scheduler.getFutureResult<int>(id1),
                           scheduler.getFutureResult<int>(id2));

  // Проверяем результат
  EXPECT_EQ(scheduler.getResult<int>(id3), 8);
}

//22
TEST(TTaskSchedulerTest, TaskWithException) {
  TTaskScheduler scheduler;

  // Добавляем задачу, которая выбрасывает исключение
  auto id = scheduler.add([]() -> int {
      throw std::runtime_error("Test exception");
  });

  // Проверяем, что выполнение задачи вызывает исключение
  EXPECT_THROW(scheduler.getResult<int>(id), std::runtime_error);
}


//23
TEST(TTaskSchedulerTest, LambdaWithCapture) {
  TTaskScheduler scheduler;

  int base = 10;

  // Добавляем задачу с захватом переменной
  auto id = scheduler.add([=]() { return base * 2; });

  // Проверяем результат
  EXPECT_EQ(scheduler.getResult<int>(id), 20);
}

//24
struct Point {
  int x, y;
};

TEST(TTaskSchedulerTest, CustomTypeTask) {
  TTaskScheduler scheduler;

  // Добавляем задачу с пользовательским типом
  auto id = scheduler.add([](Point p) { return p.x + p.y; }, Point{3, 7});

  // Проверяем результат
  EXPECT_EQ(scheduler.getResult<int>(id), 10);
}





