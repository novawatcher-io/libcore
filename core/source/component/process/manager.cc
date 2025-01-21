#include "component/process/manager.h"
#include <sys/wait.h>
#include "component/process/process.h"

namespace Core::Component::Process {
void Manager::onExit(int /* sig */, short /* events */, void *param) {
    int stat = 0;
    pid_t const pid = waitpid(-1, &stat, WNOHANG);
    if (pid <= 0) {
        return;
    }

    auto *manager = static_cast<Manager*>(param);
    if (manager == nullptr) {
        return;
    }

    auto iter = manager->maps.find(pid);
    if (iter == manager->maps.end()) {
        return;
    }
    if (iter->second->getStatus() == STOPPING) {
        iter->second->setStatus(STOPPED);
        iter->second->onStop();
        manager->maps.erase(iter);
        return;
    }
    if (iter->second->getStatus() == RELOADING) {
        iter->second->setStatus(RELOAD);
        iter->second->onReload();
        manager->maps.erase(iter);
        return;
    }
    if (iter->second->getStatus() == DELETING) {
        iter->second->setStatus(DELETED);
        iter->second->onDestroy();
        manager->maps.erase(iter);
        return;
    }
    iter->second->setStatus(EXITED);
    iter->second->onExit(manager);
}
}
