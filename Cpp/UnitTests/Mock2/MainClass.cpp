#include "MainClass.h"
#include "Publisher.h"

class MainClass::MainClassImpl {
public:
    void publish() {
        Publisher p;
        p.publish(42, 100);
    }
};

MainClass::MainClass() : pImpl(new MainClassImpl()) {
}

MainClass::~MainClass() {
    delete pImpl;
}

void MainClass::publish() {
    pImpl->publish();
}