#define X_EN 38
#define X_STEP A0
#define X_DIR A1
#define Y_EN A8
#define Y_STEP 46
#define Y_DIR 48
#define EZEKTOR 10
#define SILOWNIK 9
#define X_LIMIT_PIN 3
#define Y_LIMIT_PIN 14

#include <AccelStepper.h>

class OUTPUT_DEVICE {
  int numer_pinu_; 
public:
  OUTPUT_DEVICE(int Numer_PIN) {
    numer_pinu_ = Numer_PIN;
    pinMode(numer_pinu_, OUTPUT);
    this->OFF();
  }
  void ON() {
    digitalWrite(numer_pinu_, HIGH);
  }
  void OFF() {
    digitalWrite(numer_pinu_, LOW);
  }
};

class LIMIT_SWITCH {
  int numer_pinu_;
public:
  LIMIT_SWITCH(int Numer_PIN) {
    numer_pinu_ = Numer_PIN;
    pinMode(numer_pinu_, INPUT_PULLUP);
  }
  bool Check() {
    bool stage = true;
    for (int i=0; i<5; i++) {
      stage = digitalRead(numer_pinu_) && stage;
      delayMicroseconds(100);
    }
    return stage;
  }
};

class Stepper_Axis : public AccelStepper, public LIMIT_SWITCH {
  public:
    Stepper_Axis(int pin_enable__, int pin_step__, int pin_dir__, float max_speed__, float max_acc__, int pin_limit__) : AccelStepper(1, pin_step__, pin_dir__), LIMIT_SWITCH(pin_limit__)
  {
      pinMode(pin_enable__, OUTPUT);
      digitalWrite(pin_enable__, LOW);
      this->setMaxSpeed(max_speed__);
      this->setAcceleration(max_acc__);
    }
    void Home(float vel, bool prec) { //prec=true - II faza zerowania, cofnięcie i precyzyjny dojazd wózka
      if (!prec){
        this->setSpeed(vel);
        while (!this->Check()) {
          this->runSpeed();
        }
      }
      else {
        //this->setMaxSpeed(-vel);
        this->setSpeed(-vel);
        //this->moveTo(30);
        unsigned long t1 = millis();
        unsigned long t2 = t1;
        while(t2-t1<=400) {
          this->runSpeed();
          t2 = millis();
        }
        delay(100);
        this->setSpeed(vel);
        this->Home(vel, false);
      }
    this->setCurrentPosition(0);
    delay(200);
    }
};

class Robot {
  Stepper_Axis OS_X;
  Stepper_Axis OS_Y;
  OUTPUT_DEVICE OS_Z;
  OUTPUT_DEVICE VACUUM;
  
public:
  Robot() : OS_X(X_EN, X_STEP, X_DIR, 800.0, 800.0, X_LIMIT_PIN), OS_Y(Y_EN, Y_STEP, Y_DIR, 800.0, 800.0, Y_LIMIT_PIN), OS_Z(SILOWNIK), VACUUM(EZEKTOR) {    
    OS_Y.Home(-800.0, false);
    OS_X.Home(-800.0, false);
    OS_Y.Home(-400.0, true);
    OS_X.Home(-400.0, true);   
  }
  void MoveTo(float x__, float y__) {//koordynaty w mm
    OS_X.moveTo(x__*5.0); //poniewaz 500 krokow = 100 mm przesuniecia;
    OS_Y.moveTo(y__*5.0); //to na 1 mm "przypada" 5 krokow

    while (OS_X.distanceToGo()!=0 || OS_Y.distanceToGo()!=0) {
      OS_X.run();
      OS_Y.run();
    }
    delay(500);
  }
  void Pick() {
    OS_Z.ON();
    VACUUM.ON();
    delay(500);
    OS_Z.OFF();    
  }
  
  void Place() {
    OS_Z.ON();
    VACUUM.OFF();
    delay(500);
    OS_Z.OFF();
  }
};

void setup() {
  Robot robot;
  delay(1000);

  while(1) {
    for (int k=0; k<360; k+=10) {
      float rad = (float)k/180.0*3.1415;
      float x = 200*sin(rad);
      float y = 200*cos(rad);
      robot.MoveTo(200+x, 200+y);
    }
    // inny przyklad - kwadrat
    //robot.MoveTo(0,0);
    //robot.MoveTo(0,400);
    //robot.MoveTo(400,400);
    //robot.MoveTo(400,0);
  }
}

void loop() { 
}
