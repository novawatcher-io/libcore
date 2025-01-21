#pragma once

namespace Core {

//不允许移动类
class Nonmoveable {
public:

    Nonmoveable(Nonmoveable&&) = delete;
    Nonmoveable& operator=(Nonmoveable&&) = delete;
protected:
    Nonmoveable() = default;
    ~Nonmoveable() = default;
};
}
