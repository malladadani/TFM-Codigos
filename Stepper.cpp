#include "Stepper.h"

Stepper::Stepper(int pinStep, int pinDir, int MS1, int MS2, int MS3) {
  _pinStep = pinStep;
  _pinDir = pinDir;
  _MS1 = MS1;
  _MS2 = MS2;
  _MS3 = MS3;
  _TDead = 100e+2; // Valor inicial más razonable

  _stopFlag = false;

  pinMode(_pinStep, OUTPUT);
  pinMode(_pinDir, OUTPUT);
  pinMode(_MS1, OUTPUT);
  pinMode(_MS2, OUTPUT);
  pinMode(_MS3, OUTPUT);

  digitalWrite(_MS1, LOW);
  digitalWrite(_MS2, LOW);
  digitalWrite(_MS3, LOW);

  digitalWrite(_pinDir, LOW);

  Serial.begin(9600);
}

void Stepper::Spin(bool direction) {
  Serial.println("Sentido de giro cambiado");
  digitalWrite(_pinDir, direction);
}

/*
void Stepper::Go(int pasos) {
  for (int i = 0; i < pasos; i++) {
    
    if (_stopFlag)
    {
      Serial.println("STOP MOTOR");
      return;
    }
    else{    
    digitalWrite(_pinStep, HIGH);
    delayMicroseconds(_TDead);
    digitalWrite(_pinStep, LOW);
    delayMicroseconds(_TDead);
    Serial.print("-------> ");
    Serial.println(i);
    }
  }
}
*/


void Stepper::Change_Vel(int time) {
  Serial.println("Velocidad cambiada");
  _TDead = time;
}

void Stepper::Change_Step(int option)
{
  Serial.println("Las opciones son las siguientes: \n (1) - FULL STEP \n (2) - HALF STEP \n (3) - QUARTER STEP \n (4) - EIGTH STEP \n (5) - SIXTEENTH STEP");
  switch(option)
  {
    case(1):
      digitalWrite(_MS1, LOW);
      digitalWrite(_MS2, LOW);
      digitalWrite(_MS3, LOW);
    break;

    case(2):
      digitalWrite(_MS1, HIGH);
      digitalWrite(_MS2, LOW);
      digitalWrite(_MS3, LOW);
    break;
    
    case(3):
      digitalWrite(_MS1, LOW);
      digitalWrite(_MS2, HIGH);
      digitalWrite(_MS3, LOW);
    break;

    case(4):
      digitalWrite(_MS1, HIGH);
      digitalWrite(_MS2, HIGH);
      digitalWrite(_MS3, LOW);
    break;

    case(5):
      digitalWrite(_MS1, HIGH);
      digitalWrite(_MS2, HIGH);
      digitalWrite(_MS3, HIGH);
    break;
  }
}

void Stepper::Resume()
{
  _stopFlag = false;
}

void Stepper::Stop()
{
  _stopFlag = true;
}

void Stepper::Go(int pasos)
{
  _pasosPendientes = pasos;
  _stopFlag = false;
  _lastStepTime = 0;
  _stepState = false;
}
 
void Stepper::Update()
{
  if (_stopFlag || _pasosPendientes <= 0) return;

  unsigned long currentMicros = micros();
  if (currentMicros - _lastStepTime >= _TDead) {
    digitalWrite(_pinStep, _stepState ? LOW : HIGH);
    _stepState = !_stepState;

    if (!_stepState) {
      _pasosPendientes--;
    }

    Serial.println(_pasosPendientes);

    _lastStepTime = currentMicros;
  }
}

bool Stepper::Check_Steps()
{
  if (_pasosPendientes = 0){return true;}
  else{return false;}
}
