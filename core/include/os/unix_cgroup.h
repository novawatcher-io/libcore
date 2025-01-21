/**
******************************************************************************
* @file           : unix_cgroup.h
* @author         : zhanglei
* @brief          : None
* @attention      : None
* @date           : 2024/2/12
******************************************************************************
*/
//
// Created by zhanglei on 2024/2/12.
//

#pragma once

#include <libcgroup.h>  // for cgroup_delete_cgroup, cgroup_free
#include <sys/types.h>  // for pid_t
#include <string>       // for basic_string, string

namespace Core {
namespace OS {
class CGroup {
public:
    CGroup(const std::string& name);

    bool run();

    ~CGroup() {
        if (cg) {
            cgroup_delete_cgroup(cg, 0);
            cgroup_free(&cg);
        }
    }

    void setCpuRate(float cpu) {
        rate = cpu;
    }

    void setMemoryLimit(int limit_) {
        limit = limit_;
    }

    bool attach(pid_t pid);

private:
    bool doCpuRate();

    bool doMemoryLimit();

    std::string name_ = "watchermen";
    struct cgroup *cg = nullptr;
    struct cgroup_controller *cpuController = nullptr;
    struct cgroup_controller *memoryController = nullptr;
    int limit = 0;
    float rate = 0;
};
}
}
