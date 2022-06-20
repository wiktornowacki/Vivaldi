#include "Robot.h"

Robot::Robot(char com_port[], DWORD COM_BAUD_RATE) : Serial(com_port, COM_BAUD_RATE) {
    std::cout << "Stan COM (0/1) = " << Serial.connected_ << std::endl;
    std::this_thread::sleep_for(2000ms);
    std::cout << "OK, Home!" << std::endl;
    this->Home();
    std::cout << "Inicjalizacja robota = OK" << std::endl;
}

void Robot::Pick() {
    string read_in = "a";
    char* to_send = &read_in[0];
    bool is_sent = Serial.WriteSerialPort(to_send);
    std::this_thread::sleep_for(100ms);
    this->WaitForResponse();
}
void Robot::Place() {
    string read_in = "b";
    char* to_send = &read_in[0];
    bool is_sent = Serial.WriteSerialPort(to_send);
    std::this_thread::sleep_for(100ms);
    this->WaitForResponse();
}
void Robot::Home() {
    string read_in = "h";
    char* to_send = &read_in[0];
    bool is_sent = Serial.WriteSerialPort(to_send);
    std::this_thread::sleep_for(100ms);
    this->WaitForResponse();
}
void Robot::Move(int x, int y) {
    if (x < 0 || x>400 || y < 0 || y>400) throw "BAD_RANGE_ON_AXIS";
    long do_wyslania = 1000000 + 1000 * x + y;
    std::string read_in = std::to_string(do_wyslania);
    read_in[0] = 'x';
    std::cout << "Move = " << read_in << std::endl;
    char* to_send = &read_in[0];
    bool is_sent = Serial.WriteSerialPort(to_send);
    std::cout << is_sent << std::endl;
    std::this_thread::sleep_for(100ms);
    this->WaitForResponse();
}
void Robot::Move(std::pair<int, int> point) {
    this->Move(point.first, point.second);
}
void Robot::WaitForResponse() {
    while (1) {
        string incoming = Serial.ReadSerialPort(1, "greater_less_than");
        if (incoming != "") {
            break;
        }
    }
}