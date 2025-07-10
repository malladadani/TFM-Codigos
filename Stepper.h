#ifndef Stepper_h
#define Stepper_h

#include "Arduino.h"

class Stepper {
public:
  Stepper(int pinStep, int pinDir, int MS1, int MS2, int MS3);
  void Spin(bool direction); // Cambiado a bool para claridad
  void Go(int pasos);
  void Change_Vel(int time);
  void Change_Step(int option);
  void Stop();
  void Resume();
  void Update();
  bool Check_Steps();

private:
  int _pinStep;
  int _pinDir;
  int _MS1;
  int _MS2;
  int _MS3;
  int _TDead;

  int _pasosPendientes;
  bool _stopFlag;
  bool _stepState;
  unsigned long _lastStepTime;

};

#endif