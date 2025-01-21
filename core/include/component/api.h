#pragma once

#include <string>
#include <utility>
#include <memory>
#include <map>
#include <vector>
#include "non_copyable.h"
#include "non_moveable.h"
#include "pure.h"

namespace Core::Component {
class Lifecycle {
public:
    virtual void init() PURE;

    virtual void start() PURE;

    virtual void stop() PURE;

    virtual void finish() PURE;

    virtual ~Lifecycle() = default;
};

class Component :public Lifecycle {
public:
    virtual std::string name() PURE;
};

class ConfigListener {
public:
    virtual void onUpdate() PURE;
    virtual void onCreate() PURE;
    virtual void onDelete() PURE;
    virtual ~ConfigListener() = default;
};

class BaseConfig :public ConfigListener {

};

typedef enum {
    FAIL,
    SUCCESS,
    TIMEOUT,
    DROP
} Status;


class Result {
public:
    Status status() const {
        return status_;
    }
    void changeStatusTo(Status status) {
        status_ = status;
    }
    std::string error() const {
        return message_;
    }

    void setErrorInfo(std::string message) {
        message_ = std::move(message);
    }
    Status status_ = SUCCESS;
    std::string message_;
};

class Data {
public:
    virtual void* data() PURE;
    virtual ~Data() = default;
};


template<class T>
class AnyData :public Data {
public:
    explicit AnyData(std::unique_ptr<T> data) {
        data_ = std::move(data);
    }
    void* data() override {
        return data_.get();
    };
private:
    std::unique_ptr<T> data_;
};

class EventData {
public:
    virtual std::map<std::string, std::string> meta() PURE;
    virtual std::map<std::string, std::string> header() PURE;
    virtual void* data() PURE;
    virtual std::unique_ptr<Data>& dataPtr() PURE;
    virtual std::string name() PURE;
    // Fill event with meta,header,body cannot be nil
    virtual void  fill(const std::map<std::string, std::string>& meta, const std::map<std::string, std::string>& header, std::unique_ptr<Data> data) PURE;
    virtual void release() PURE;

    virtual ~EventData() = default;
};

class Batch :public Nonmoveable, Noncopyable {
public:
    virtual std::map<std::string, std::string> meta() PURE;
    virtual std::vector<std::unique_ptr<Core::Component::EventData>>& events() PURE;
    virtual void fill(std::map<std::string, std::string> meta, std::vector<std::unique_ptr<Core::Component::EventData>>& events) PURE;
    virtual void Release() PURE;
    virtual ~Batch() = default;
};

class Pipeline;

class Queue :public Nonmoveable, Noncopyable{
public:
    virtual void run() PURE;
    virtual void stop() PURE;
    virtual ~Queue() = default;
};


class Consumer :public Noncopyable, Nonmoveable {
public:
    virtual Result Consume(std::shared_ptr<Batch>& batch) PURE;

    virtual std::shared_ptr<Queue>& channel() PURE;

    virtual bool bindChannel(std::shared_ptr<Queue> channel) PURE;

    virtual Result  Consume(Batch& batch) PURE;

    virtual ~Consumer() = default;
};

class Interceptor {
public:
    virtual Core::Component::Result intercept(std::unique_ptr<Core::Component::Batch>& batch) PURE;

    virtual Core::Component::Result intercept(Core::Component::Batch& batch) PURE;

    virtual ~Interceptor() = default;
};

class Pipeline {
public:
    virtual void addIntercept(std::unique_ptr<Interceptor>& interceptor) PURE;

    virtual void addConsumer(std::unique_ptr<Consumer>& consumer) PURE;

    virtual void bindChannel(std::shared_ptr<Queue>& queue) PURE;

    virtual void push(std::unique_ptr<EventData> e) PURE;

    virtual void flush() PURE;
    virtual ~Pipeline() = default;
};

}