//
// Created by kiva on 2019-01-01.
//
#include <spark/SparkGC_All.h>
#include <cstdint>
#include <vector>
#include <stack>
#include <thread>

using namespace spark;

using Int = int64_t;

struct Vec2 : public CollectedObject {
    Int x;
    Int y;

    virtual Size getOnStackSize() override {
        return sizeof(*this);
    }

    virtual void markChildren(ColorMarker &marker) override {
    }
};

enum Opcode : int {
    NEW_VEC2,
    ADD2,
    SHOW2,
    INC2_X,
    INC2_Y,
    COLLECT,
    HALT,
};

class SimpleVM : public SparkMutator {
private:
    SparkGC *gc;
    std::vector<Opcode> opcodes;
    std::deque<CollectedObject *> stack;
    bool wait;

protected:
    virtual void markGlobalRoot() override {
        for (auto obj : stack) {
            markGray(obj);
        }
    }

public:
    explicit SimpleVM(SparkGC *gc) {
        gc->registerMutator(this);
        this->gc = gc;
        this->wait = false;
    }

    ~SimpleVM() override {
        gc->unregisterMutator(this);
    }

    SimpleVM *addCode(std::vector<Opcode> &&code) {
        for (auto c : code) {
            opcodes.push_back(c);
        }
        return this;
    }

    template<typename T>
    T *gcNew() {
        Addr addr = this->allocate(sizeof(T));
        if (addr == nullptr) {
            doGCWait();
            addr = this->allocate(sizeof(T));
        }
        if (addr == nullptr) {
            throw std::bad_alloc();
        }
        T *t = new(addr) T();
        return t;
    }

    template<typename T>
    T *pop() {
        auto object = stack.front();
        stack.pop_front();
        return (T *) object;
    }

    void push(CollectedObject *o) {
        stack.push_front(o);
    }

    void run() {
        for (auto code : opcodes) {
            switch (code) {
                case NEW_VEC2:
                    printf("new-vec2\n");
                    push(gcNew<Vec2>());
                    break;
                case HALT:
                    printf("halt\n");
                    doGCWait();
                    return;
                case COLLECT:
                    printf("collect\n");
                    doGCWait();
                    break;
                case SHOW2: {
                    printf("show2\n");
                    auto vec2 = pop<Vec2>();
                    printf("\tVec2 %p { x = %lld, y = %lld }\n",
                        vec2, vec2->x, vec2->y);
                    break;
                }
                case ADD2: {
                    printf("add2\n");
                    auto lhs = pop<Vec2>();
                    auto rhs = pop<Vec2>();
                    auto r = gcNew<Vec2>();
                    printf("\tlhs %p\n", lhs);
                    printf("\trhs %p\n", rhs);
                    printf("\tres %p\n", r);
                    r->x = lhs->x + rhs->x;
                    r->y = lhs->y + rhs->y;
                    push(r);
                    break;
                }
                case INC2_X: {
                    printf("inc2-x\n");
                    auto v = pop<Vec2>();
                    auto r = gcNew<Vec2>();
                    printf("\tv %p\n", v);
                    printf("\tr %p\n", r);
                    r->x = v->x + 1;
                    r->y = v->y;
                    push(r);
                    break;
                }
                case INC2_Y: {
                    printf("inc2-y\n");
                    auto v = pop<Vec2>();
                    auto r = gcNew<Vec2>();
                    printf("\tv %p\n", v);
                    printf("\tr %p\n", r);
                    r->x = v->x;
                    r->y = v->y + 1;
                    push(r);
                    break;
                }
            }
        }
    }

    void doGCWait() {
        printf("doGCWait()\n");
        wait = true;
        std::thread([this]() {
            gc->collect();
            wait = false;
        }).detach();
        while (wait) {
            handshakeCollector();
        }
    }
};

void entrance(SparkGC *gc) {
    SimpleVM vm(gc);

    // Vec2(1, 2)
    vm.addCode({NEW_VEC2,
                INC2_X,
                INC2_Y, INC2_Y});

    // Vec2(3, 4)
    vm.addCode({NEW_VEC2,
                INC2_X, INC2_X, INC2_X,
                INC2_Y, INC2_Y, INC2_Y, INC2_Y});

    vm.addCode({ADD2, COLLECT, SHOW2, HALT});

    vm.run();
}

int main() {
    // 128K
    constexpr Size memSize = 128 * 1024;
    SparkGC *gc = SparkGC::newGC(memSize);
    if (gc == nullptr) {
        throw std::bad_alloc();
    }

    std::thread mainThread(entrance, gc);
    mainThread.join();
    SparkGC::deleteGC(gc);
    return 0;
}
