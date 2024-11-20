// InputMacro
//.h

#ifndef InputMacro_h
#define InputMacro_h

class InputMacro
{
public:
  InputMacro(bool); // declare starting state of Macro

  bool run(bool);          // run input macro. inputs(input(bool)). returns true is state/input changes
  bool operator()(bool);   // same as "run()"
  operator bool();         // return state of macro, does not update state, only returns last recorded state
  bool state();            // same as "operator bool". returns last state of macro
  bool stateChange();      // returns true if StateChange from "last" run call
  uint32_t interval();     // returns current interval(ms) of input from last stateChange
  uint32_t prevInterval(); // returns previous recorded interval(ms)
  void reset();            // reset current interval to 0ms

private:
  bool _state;               // global state of macro
  bool _stateChange = false; // stateChange status from last "run" call

  uint32_t _timerInterval;    // this hold the millis() value at the time of last stateChange
  uint32_t _prevInterval = 0; // this hold actual value(in ms) of last interval
};

//.cpp

InputMacro::InputMacro(bool startState)
{
  _state = startState;
  reset();
}

bool InputMacro::run(bool stateInput)
{
  if (stateInput != _state)
  {
    _state = stateInput;
    _stateChange = true;

    _prevInterval = (millis() - _timerInterval);
    reset();
  }
  else
    _stateChange = false;

  return _stateChange;
}
bool InputMacro::operator()(bool stateInput)
{
  return run(stateInput);
}
InputMacro::operator bool()
{
  return _state;
}
bool InputMacro::state()
{
  return _state;
}
bool InputMacro::stateChange()
{
  return _stateChange;
}
uint32_t InputMacro::interval()
{
  return (millis() - _timerInterval);
}
uint32_t InputMacro::prevInterval()
{
  return _prevInterval;
}
void InputMacro::reset()
{
  _timerInterval = millis();
}

#endif