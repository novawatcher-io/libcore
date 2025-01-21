/**
******************************************************************************
* @file           : container.h
* @author         : zhanglei
* @brief          : None
* @attention      : None
* @date           : 2024/2/11
******************************************************************************
*/
//
// Created by zhanglei on 2024/2/11.
//

#pragma once

#include <memory>
#include <map>
#include <string>
#include <utility>
#include <functional>

#include "api.h"

namespace Core {
namespace Component {

typedef std::pair<std::shared_ptr<BaseConfig>, std::shared_ptr<Component>> functionMap;
typedef std::map<std::string, functionMap> ContainerMap;
typedef std::function<void(const functionMap&)> OnForeachMapClosure;

class Container :public std::enable_shared_from_this<Container> {
public:

    Container() {

    }

    ~Container() {

    }

    /**
     * @brief 向容器中注册功能
     *
     * @param id
     * @param object
     * @return true
     * @return false
     */
    bool bind(const std::string &id, const functionMap &object) {
        if (id.empty()) {
            return false;
        }
        auto instance = containerMap.find(id);
        if (instance != containerMap.end()) {
            return false;
        }
        containerMap[id] = object;
        return true;
    }

    void foreachContainer(const OnForeachMapClosure& closure) {
        for (auto iter = containerMap.begin(); iter != containerMap.end(); ) {
            closure(iter->second);
            iter++;
        }
    }

    functionMap& get(const std::string &id) {
        if (id.empty()) {
            return emptyObject;
        }

        auto instance = containerMap.find(id);
        if (instance == containerMap.end()) {
            return emptyObject;
        }

        return instance->second;
    }

    /**
     * @brief 初始化服务
     *
     * @return true
     * @return false
     */
    bool init() {
        for (auto iter = containerMap.begin(); iter != containerMap.end(); ) {
            if (iter->second.second)
                iter->second.second->init();
            iter++;
        }
        return true;
    }

    /**
     * @brief 初始化服务
     *
     * @return true
     * @return false
     */
    bool start() {
        for (auto iter = containerMap.begin(); iter != containerMap.end(); ) {
            if (iter->second.second)
                iter->second.second->start();
            iter++;
        }
        return true;
    }

    /**
     * @brief 容器初始化完成
     *
     * @return true
     * @return false
     */
    bool finish() {
        for (auto iter = containerMap.begin(); iter != containerMap.end(); ) {
            if (iter->second.second)
                iter->second.second->finish();
            iter++;
        }
        return true;
    }

    /**
     * @brief 容器 shutdown
     *
     * @return true
     * @return false
     */
    bool shutdown() {
        for (auto iter = containerMap.begin(); iter != containerMap.end(); ) {
            if (iter->second.second)
                iter->second.second->stop();
            iter++;
        }
        //销毁服务
        containerMap.clear();
        return true;
    }
private:
    /**
     * @brief 容器列表
     *
     */
    ContainerMap containerMap;

    /**
     * @brief 空的对象
     *
     */
    functionMap emptyObject = {};
};
}
}
