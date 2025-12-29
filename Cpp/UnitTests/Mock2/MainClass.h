#pragma once

class MainClass {
public:
    MainClass();
    ~MainClass();
    
    void publish();

private:
    class MainClassImpl;
    MainClassImpl* pImpl;
};