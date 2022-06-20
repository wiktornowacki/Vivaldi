#pragma once
#include <chrono>
#include <thread>
#include "SimpleSerial.h"
#include <iostream>

class Robot
{
private:
    SimpleSerial Serial;
public:
    Robot(char com_port[],DWORD COM_BAUD_RATE);
    void Pick();
    void Place();
    void Home();
    void Move(int x, int y);
    void Move(std::pair<int, int> point);
    void WaitForResponse();
};

