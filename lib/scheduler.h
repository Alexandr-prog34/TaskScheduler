#pragma once
#include <memory>
#include <vector>
#include <stdexcept>

template<typename T>
decltype(auto) Arg(T&& value) { return std::forward<T>(value); }

class TTaskScheduler {
public:
    using TaskIdentifier = size_t;

    template<typename T>
    class FutureResult {
        TTaskScheduler* schedulerInstance;
        TaskIdentifier taskId;
    public:
        FutureResult(TTaskScheduler* scheduler, TaskIdentifier id) : schedulerInstance(scheduler), taskId(id) {}

        T get() const {
            return schedulerInstance->getResult<T>(taskId);
        }

        operator T() const { return get(); }
        TaskIdentifier getId() const { return taskId; }
    };

    template<typename T>
    class DeferredArgument {
        bool isFutureValue = false;
        T value{};
        FutureResult<T> futureValue{ nullptr, 0 };

    public:
        DeferredArgument(const T& v) : isFutureValue(false), value(v) {}
        DeferredArgument(const FutureResult<T>& f) : isFutureValue(true), futureValue(f) {}

        T resolve() const {
            if (isFutureValue) {
                return futureValue.get();
            }
            else {
                return value;
            }
        }
    };

    struct BaseTask {
        virtual ~BaseTask() = default;
        virtual void execute(TTaskScheduler& scheduler) = 0;
        virtual void* getRawResult() = 0;
        bool isExecuted = false;
    };

    template<typename Func>
    struct TaskWithoutArgs : BaseTask {
        Func taskFunction;
        using ResultType = decltype(taskFunction());
        ResultType result;

        TaskWithoutArgs(Func f) : taskFunction(f) {}

        void execute(TTaskScheduler&) override {
            if (!isExecuted) {
                result = taskFunction();
                isExecuted = true;
            }
        }

        void* getRawResult() override { return &result; }
    };

    template<typename Func, typename Arg>
    struct TaskWithOneArg : BaseTask {
        Func taskFunction;
        DeferredArgument<Arg> argument;
        using ResultType = decltype(taskFunction(std::declval<Arg>()));
        ResultType result;

        TaskWithOneArg(Func f, const DeferredArgument<Arg>& a) : taskFunction(f), argument(a) {}

        void execute(TTaskScheduler&) override {
            if (!isExecuted) {
                result = taskFunction(argument.resolve());
                isExecuted = true;
            }
        }

        void* getRawResult() override { return &result; }
    };

    template<typename Func, typename Arg1, typename Arg2>
    struct TaskWithTwoArgs : BaseTask {
        Func taskFunction;
        DeferredArgument<Arg1> firstArgument;
        DeferredArgument<Arg2> secondArgument;
        using ResultType = decltype(taskFunction(std::declval<Arg1>(), std::declval<Arg2>()));
        ResultType result;

        TaskWithTwoArgs(Func f, const DeferredArgument<Arg1>& a1, const DeferredArgument<Arg2>& a2)
            : taskFunction(f), firstArgument(a1), secondArgument(a2) {}

        void execute(TTaskScheduler&) override {
            if (!isExecuted) {
                result = taskFunction(firstArgument.resolve(), secondArgument.resolve());
                isExecuted = true;
            }
        }

        void* getRawResult() override { return &result; }
    };

    std::vector<std::unique_ptr<BaseTask>> taskList;
    TaskIdentifier nextTaskId = 0;


    template<typename Func>
    TaskIdentifier add(Func function) {
        auto task = std::make_unique<TaskWithoutArgs<Func>>(function);

        TaskIdentifier id = nextTaskId++;
        taskList.push_back(std::move(task));

        return id;
    }

    template<typename Func, typename Arg>
    TaskIdentifier add(Func function, Arg argument) {
        auto deferredArg = DeferredArgument<Arg>(argument);

        auto task = std::make_unique<TaskWithOneArg<Func, Arg>>(function, deferredArg);

        TaskIdentifier id = nextTaskId++;
        taskList.push_back(std::move(task));

        return id;
    }

    template<typename Func, typename Arg>
    TaskIdentifier add(Func function, FutureResult<Arg> argument) {
        auto deferredArg = DeferredArgument<Arg>(argument);

        auto task = std::make_unique<TaskWithOneArg<Func, Arg>>(function, deferredArg);

        TaskIdentifier id = nextTaskId++;
        taskList.push_back(std::move(task));

        return id;
    }

    template<typename Func, typename Arg1, typename Arg2>
    TaskIdentifier add(Func function, Arg1 argument1, Arg2 argument2) {
        auto deferredArg1 = DeferredArgument<Arg1>(argument1);
        auto deferredArg2 = DeferredArgument<Arg2>(argument2);

        auto task = std::make_unique<TaskWithTwoArgs<Func, Arg1, Arg2>>(function, deferredArg1, deferredArg2);

        TaskIdentifier id = nextTaskId++;
        taskList.push_back(std::move(task));

        return id;
    }

    template<typename Func, typename Arg1, typename Arg2>
    TaskIdentifier add(Func function, FutureResult<Arg1> argument1, Arg2 argument2) {
        auto deferredArg1 = DeferredArgument<Arg1>(argument1);
        auto deferredArg2 = DeferredArgument<Arg2>(argument2);

        auto task = std::make_unique<TaskWithTwoArgs<Func, Arg1, Arg2>>(function, deferredArg1, deferredArg2);

        TaskIdentifier id = nextTaskId++;
        taskList.push_back(std::move(task));

        return id;
    }

    template<typename Func, typename Arg1, typename Arg2>
    TaskIdentifier add(Func function, Arg1 argument1, FutureResult<Arg2> argument2) {
        auto deferredArg1 = DeferredArgument<Arg1>(argument1);
        auto deferredArg2 = DeferredArgument<Arg2>(argument2);

        auto task = std::make_unique<TaskWithTwoArgs<Func, Arg1, Arg2>>(
            function, deferredArg1, deferredArg2
        );

        TaskIdentifier id = nextTaskId++;
        taskList.push_back(std::move(task));

        return id;
    }

    template<typename Func, typename Arg1, typename Arg2>
    TaskIdentifier add(Func function, FutureResult<Arg1> argument1, FutureResult<Arg2> argument2) {
        auto deferredArg1 = DeferredArgument<Arg1>(argument1);
        auto deferredArg2 = DeferredArgument<Arg2>(argument2);

        auto task = std::make_unique<TaskWithTwoArgs<Func, Arg1, Arg2>>(function, deferredArg1, deferredArg2);

        TaskIdentifier id = nextTaskId++;
        taskList.push_back(std::move(task));

        return id;
    }

    template<typename Ret, typename Class, typename Arg>
    TaskIdentifier add(Ret(Class::* method)(Arg) const, Class obj, Arg arg) {
        auto lambda = [=](Arg a) {
            return (obj.*method)(a);
            };

        return add(lambda, arg);
    }

    template<typename Ret, typename Class, typename Arg>
    TaskIdentifier add(Ret(Class::* method)(Arg) const, Class obj, FutureResult<Arg> arg) {
        auto lambda = [=](Arg a) {
            return (obj.*method)(a);
            };

        return add(lambda, arg);
    }

    template<typename Ret, typename Class, typename Arg>
    TaskIdentifier add(Ret(Class::* method)(Arg) const, FutureResult<Class> obj, Arg arg) {
        auto lambda = [=](Class o, Arg a) {
            return (o.*method)(a);
            };

        return add(lambda, obj, arg);
    }

    template<typename Ret, typename Class, typename Arg>
    TaskIdentifier add(Ret(Class::* method)(Arg) const, FutureResult<Class> obj, FutureResult<Arg> arg) {
        auto lambda = [=](Class o, Arg a) {
            return (o.*method)(a);
            };

        return add(lambda, obj, arg);
    }


    void executeAll() {
        for (auto& task : taskList) {
            if (task && !task->isExecuted) {
                task->execute(*this);
            }
        }
    }

    void clear() {
        taskList.clear();
        nextTaskId = 0;
    }

    void removeTask(TaskIdentifier id) {
        if (id >= taskList.size() || !taskList[id]) {
            throw std::out_of_range("Bad task ID");
        }
        taskList[id].reset();
    }

    bool hasTask(TaskIdentifier id) const {
        return id < taskList.size() && taskList[id] != nullptr;
    }

    template<typename T>
    T getResult(TaskIdentifier id) {
        if (id >= taskList.size()) {
            throw std::out_of_range("Bad task ID");
        }
        if (!taskList[id]->isExecuted) {
            taskList[id]->execute(*this);
        }
        return *static_cast<T*>(taskList[id]->getRawResult());
    }

    template<typename T>
    FutureResult<T> getFutureResult(TaskIdentifier id) {
        return FutureResult<T>(this, id);
    }
};

