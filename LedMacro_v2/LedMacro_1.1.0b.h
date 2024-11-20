// LedMacro
//.h

/*
  v1.1.0 changes:
    - implement a method to change the speed/rate of a macro that is currently running
    - partial re-documentation

  ToDo:
    - Currently Beta, review code and run tests
    - Split LedMacro and LedMacroManager classes
*/

#ifndef LedMacro_h
#define LedMacro_h

#include "Arduino.h"

class LedMacro
{
  /*
    note:
      - reference is required by every macro because it is the output value AND can be used as a form of identification
  */
public:
  bool run();   // run led macro. return true if ready for another macro. false if still running a macro
  bool ready(); // returns if ready for another macro/equivalent of run, just without running the macro(faster)
  void stop();  // stop macros, (ready with return true)

  // transition functions:
  /*
    lineEase():
    quadEase():
    cubicEase():
    set():
      initiate transition. Given reference value will be modified as needed for set transition
        lineEase(): linear ease
        quadEase(): quadratic ease
        cubicEase(): cubic ease
        set(): set to specific value and delay

      inputs: ((uint8_t &) reference value, (uint8_t) transition target value, (uint32_t) transition time. ms)
  */
  void lineEase(uint8_t &, uint8_t, uint32_t);  // linear ease/fade, input(value(reference), target(0 - 255), frames(frames to run macro))
  void quadEase(uint8_t &, uint8_t, uint32_t);  // quadratic ease-in and ease-out, inputs(value(reference), target(0 - 255), frames)
  void cubicEase(uint8_t &, uint8_t, uint32_t); // cubic ease-in and ease-out, inputs(value(reference), target(0 - 255), frames)
  void set(uint8_t &, uint8_t, uint32_t);       // sets value to target and delays set amount(ms). inputs(value(reference), target(0 - 255), delay(ms))

  void delay(uint8_t &, uint16_t); // delay all macros for set time. inputs(value(reference), delay(ms))

  /*
    mod():
      modify transition time during a transition
      will not affect if no transition is running
      needs to be called with each new macro
          1.0 = normal speed
          2.0 = double speed
          0.5 = half speed
        inputs: ((float) transition modify multiplier)
  */
  void mod(float);

  /*
    mod_t():
      set new total transition time during a transition
      will not affect if no transition is running
      needs to be called with each new macro
        inputs: ((float) transition modify multiplier)
  */
  void mod_t(uint32_t);

  void fps(uint16_t); // set fps(frames(refreshes) per second). default is set on 60 fps. fps is only used for linear and easing

  uint8_t *refValue();

private:
  enum class _transitionTypeEnum
  {
    set, /* <- works same with delay*/
    lineEase,
    quadEase,
    cubicEase
  };
  float _quad_easeinout(float);  // calculate quadEase function. inputs(t(time)). outputs v(horizontal percentage)
  float _cubic_easeinout(float); // calculate cubicEase function. inputs(t(time)). outputs v(horizontal percentage)

  uint8_t _lerp(int16_t, int16_t, float); // Linear Interpolation function, inputs (point_a, point_b, percentage(0.0 - 1.0)). outputs value between point a and point b relative to percentage input

  bool _macroRun = false; // determines wether a macro is currently running

  _transitionTypeEnum _transitionType; // use to distinguish between "set"(0), "linear"(1), "quad"(2), and "cubic"(3). enum implemented
  uint8_t _startVal;                   // start value captured at the call of a transition. laster will be used with '_lerp'
  uint8_t _endVal;                     // end value captured at the call of a transition. also will be later used with '_lerp'
  uint16_t _rate_ms = 16;              // delay between each transition step in ms. defaults to aprox 60 fps(16.667ms)
  uint16_t _nextDelay;                 // value set when transition functions called. used during run
  uint32_t _timer;                     // timer for keeping track of transitions. used with "millis()"
  uint32_t _totalTime;                 // total amount of time transition requires to complete
  float _currentStep;                  // current step for transition. Travels 0.0 - 1.0
  float _step;                         // amount to step '_currentStep' by.

  float _transitionMod = 1.0; // value to keep track of transition speed

  uint8_t *_refValue; // pointer of referenced value giving in each transition function
};

class LedMacroManager
{
public:
  LedMacroManager(LedMacro *, uint8_t); // takes a pool of ledMacro's. inputs(macro pool(pointer), Macro pool count)

  void run();            // handler. runs all LedMacros in background. Required method! Make sure this is called frequently
  bool run(uint8_t &);   // runs all LedMacros. returns ready-state of macro with given reference value
  bool ready(uint8_t &); // returns if macro ready using reference value. does not run/handle macro
  void stop(uint8_t &);  // stop macro for given reference value

  // transition functions:
  /*
    lineEase():
    quadEase():
    cubicEase():
    set():
      attaches macro to reference value and initiate transition.
        lineEase(): linear ease
        quadEase(): quadratic ease
        cubicEase(): cubic ease
        set(): set to specific value and delay

      inputs: ((uint8_t &) reference value, (uint8_t) transition target value, (uint32_t) transition time. ms)
      returns: (boolean) true if value successfully attached to macro, false otherwise
  */
  bool lineEase(uint8_t &, uint8_t, uint32_t);  // linear ease/fade
  bool quadEase(uint8_t &, uint8_t, uint32_t);  // quadratic ease-in and ease-out
  bool cubicEase(uint8_t &, uint8_t, uint32_t); // cubic ease-in and ease-out
  bool set(uint8_t &, uint8_t, uint32_t);       // sets value to target and delays set amount(ms)

  /*
    delay():
      delay any and all macros for set time

      input: ((uint8_t &) reference value, (uint32_t) transition time. ms)
      output: (boolean) true if value successfully attached to macro, false otherwise
      */
  bool delay(uint8_t &, uint32_t); // delay any and all macros for set time

  /*
    mod():
      modify transition time during a transition
      will not affect if no transition is running
      needs to be called with each new macro
          1.0 = normal speed
          2.0 = double speed
          0.5 = half speed
        inputs: ((uint8_t &) reference value, (float) transition modify multiplier)
  */
  void mod(uint8_t &, float);

  /*
    mod_t():
      set new total transition time during a transition
      will not affect if no transition is running
      needs to be called with each new macro
        inputs: ((uint8_t &) reference value, (float) transition modify multiplier)
  */
  void mod_t(uint8_t &, uint32_t);

  /*
    fps():
      sets global fps for all macros. default is set on 60 fps.
      fps is only used for linear and easing transitions.
      Higher fps requires more processing, lower fps results in visually 'choppy' transition
      Will interfere with current transitions

      input: ((uint16_t) fps value)
  */
  void fps(uint16_t);

private:
  LedMacro *_LedMacro_Pointer;
  uint8_t _numberOfInstances;
};

//.cpp

/////////////////////////////////////////////////
//////////////// LedMacro source ////////////////

bool LedMacro::run()
{
  if (_macroRun)
  {
    switch (_transitionType)
    {
    // set/delay macros require different timer management:
    case _transitionTypeEnum::set:
      if (millis() - _timer >= (uint32_t)((float)_nextDelay / _transitionMod))
      {
        _macroRun = false; // delay completed, end macro
      }
      break;
    default:
      if (millis() - _timer >= _nextDelay)
      {
        switch (_transitionType)
        {
        case _transitionTypeEnum::lineEase:
          _timer += _nextDelay; // absolute timing, make sure led runs exactly at specified frameRate
          // _timer = millis(); // relative timing, use if absolute timing becomes unstable
          _currentStep += (_step * _transitionMod);
          if (_currentStep >= 1.0) // check if transistion is complete
          {
            _currentStep = 1.0;
            _macroRun = false;
          }

          *_refValue = _lerp(_startVal, _endVal, (_currentStep)); // update value
          break;
        case _transitionTypeEnum::quadEase:
          _timer += _nextDelay; // absolute timing, make sure led runs exactly at specified frameRate
          // _timer = millis(); // relative timing, use if absolute timing becomes unstable
          _currentStep += (_step * _transitionMod);
          if (_currentStep >= 1.0) // check if transistion is complete
          {
            _currentStep = 1.0;
            _macroRun = false;
          }

          *_refValue = _lerp(_startVal, _endVal, _quad_easeinout(_currentStep)); // update value
          break;
        case _transitionTypeEnum::cubicEase:
          _timer += _nextDelay; // absolute timing, make sure led runs exactly at specified frameRate
          // _timer = millis(); // relative timing, use if absolute timing becomes unstable
          _currentStep += (_step * _transitionMod);
          if (_currentStep >= 1.0) // check if transistion is complete
          {
            _currentStep = 1.0;
            _macroRun = false;
          }

          *_refValue = _lerp(_startVal, _endVal, _cubic_easeinout(_currentStep)); // update value
          break;
        default:             // unknown transition
          _macroRun = false; // end macro so we dont get a stuck macro
          break;
        }
      }
      break;
    }
  }
  return !_macroRun; // return 'not' wether macros are currently running
}
bool LedMacro::ready()
{
  return !_macroRun; // return 'not' wether macros are currently running
}
void LedMacro::stop()
{
  _macroRun = false;
}

void LedMacro::lineEase(uint8_t &ref, uint8_t target, uint32_t transitionTime)
{
  _macroRun = false; // safety precaution, in-case handler is run by an interrupt
  _refValue = &ref;  // save reference value

  if (transitionTime >= _rate_ms) // run only if 'transitionTime' is greater or equal to one frame(ms)
  {
    _transitionType = _transitionTypeEnum::lineEase; // set transition type, delay(0), linearEase(1), quadraticEase(2), cubicEase(3)
    _startVal = *_refValue;                          // start value is the current refValue
    _endVal = target;                                // end value is target value

    _transitionMod = 1.0; // reset transition mod value

    _currentStep = 0.0;                                // reset step val
    _step = ((float)_rate_ms / (float)transitionTime); // calc float step amount

    // double check:
    if (!(_step > 0.0)) // '_step' MUST be greater than 0.0
    {
      // ToDo: set '_step' to minimume possible value?
      // bailout method
      _macroRun = false;
      *_refValue = target;
      return;
    }

    _nextDelay = _rate_ms;
    _timer = millis(); // reset timer for new transition
    _macroRun = true;
  }
  else
  {
    _macroRun = false;   // redundant
    *_refValue = target; // simply set value to target and exit
  }
}
void LedMacro::quadEase(uint8_t &ref, uint8_t target, uint32_t transitionTime)
{
  _macroRun = false; // safety precaution, in-case handler is run by an interrupt
  _refValue = &ref;  // save reference value

  if (transitionTime >= _rate_ms) // run only if 'transitionTime' is greater or equal to one frame(ms)
  {
    _transitionType = _transitionTypeEnum::quadEase; // set transition type, delay(0), linearEase(1), quadraticEase(2), cubicEase(3)
    _startVal = *_refValue;                          // start value is the current refValue
    _endVal = target;                                // end value is target value

    _transitionMod = 1.0; // reset transition mod value

    _currentStep = 0.0;                                // reset step val
    _step = ((float)_rate_ms / (float)transitionTime); // calc float step amount

    // double check:
    if (!(_step > 0.0)) // '_step' MUST be greater than 0.0
    {
      // ToDo: set '_step' to minimume possible value?
      // bailout method
      _macroRun = false;
      *_refValue = target;
      return;
    }

    _nextDelay = _rate_ms;
    _timer = millis(); // reset timer for new transition
    _macroRun = true;
  }
  else
  {
    _macroRun = false;   // redundant
    *_refValue = target; // simply set value to target and exit
  }
}
void LedMacro::cubicEase(uint8_t &ref, uint8_t target, uint32_t transitionTime)
{
  _macroRun = false; // safety precaution, in-case handler is run by an interrupt
  _refValue = &ref;  // save reference value

  if (transitionTime >= _rate_ms) // run only if 'transitionTime' is greater or equal to one frame(ms)
  {
    _transitionType = _transitionTypeEnum::cubicEase; // set transition type, delay(0), linearEase(1), quadraticEase(2), cubicEase(3)
    _startVal = *_refValue;                           // start value is the current refValue
    _endVal = target;                                 // end value is target value

    _transitionMod = 1.0; // reset transition mod value

    _currentStep = 0.0;                                // reset step val
    _step = ((float)_rate_ms / (float)transitionTime); // calc float step amount

    // double check:
    if (!(_step > 0.0)) // '_step' MUST be greater than 0.0
    {
      // ToDo: set '_step' to minimume possible value?
      // bailout method
      _macroRun = false;
      *_refValue = target;
      return;
    }

    _nextDelay = _rate_ms;
    _timer = millis(); // reset timer for new transition
    _macroRun = true;
  }
  else
  {
    _macroRun = false;   // redundant
    *_refValue = target; // simply set value to target and exit
  }
}
void LedMacro::set(uint8_t &ref, uint8_t target, uint32_t delay_ms)
{
  _macroRun = false;   // safety precaution, in-case handler is run by an interrupt
  _refValue = &ref;    // save reference value
  *_refValue = target; // set to target val

  if (delay_ms) // pointless without a delay
  {
    _transitionType = _transitionTypeEnum::set;
    _transitionMod = 1.0; // reset transition mod value
    _nextDelay = delay_ms;
    _timer = millis(); // reset timer for new transition
    _macroRun = true;
  }
  else
  {
    _macroRun = false; // redundant
  }
}
void LedMacro::delay(uint8_t &ref, uint16_t delay_ms)
{
  _macroRun = false; // safety precaution, in-case handler is run by an interrupt
  _refValue = &ref;  // save reference value

  if (delay_ms) // pointless without a delay
  {
    _transitionType = _transitionTypeEnum::set;
    _transitionMod = 1.0; // reset transition mod value
    _nextDelay = delay_ms;
    _timer = millis(); // reset timer for new transition
    _macroRun = true;
  }
  else
  {
    _macroRun = false; // redundant
  }
}

void LedMacro::mod(float transitionModifier)
{
  if (_macroRun)
  {
    // ToDo: Constraints?
    if (transitionModifier > 0.0)
      _transitionMod = transitionModifier;
  }
}

void LedMacro::mod_t(uint32_t transitionTime)
{
  if (_macroRun)
  {
    switch (_transitionType)
    {
    case _transitionTypeEnum::set:
      _macroRun = false; // safety precaution, in-case handler is run by an interrupt
      if (transitionTime)
      {
        _transitionMod = 1.0; // reset transition mod value
        _nextDelay = transitionTime;
        _macroRun = true;
      }
      else
      {
        _macroRun = false; // redundant
      }
      break;
    case _transitionTypeEnum::lineEase:
    case _transitionTypeEnum::quadEase:
    case _transitionTypeEnum::cubicEase:
      _macroRun = false;              // safety precaution, in-case handler is run by an interrupt
      if (transitionTime >= _rate_ms) // run only if 'transitionTime' is greater or equal to one frame(ms)
      {
        _transitionMod = 1.0;                              // reset transition mod value
        _step = ((float)_rate_ms / (float)transitionTime); // calc float step amount

        // double check:
        if (!(_step > 0.0)) // '_step' MUST be greater than 0.0
        {
          // ToDo: set '_step' to minimume possible value?
          // bailout method
          _macroRun = false;
          *_refValue = _endVal;
          return;
        }

        _macroRun = true;
      }
      else
      {
        _macroRun = false;    // redundant
        *_refValue = _endVal; // simply set value to target and exit
      }
      break;
      // ignore unknown macros?
    }
  }
}

void LedMacro::fps(uint16_t set_fps)
{
  if (set_fps)
    _rate_ms = (1000 / set_fps);
  else
    _rate_ms = 1000; // min fps: 1 fps(1000ms)
}

uint8_t *LedMacro::refValue()
{
  return _refValue;
}

float LedMacro::_quad_easeinout(float time)
{
  time *= 2.0;
  if (time < 1.0)                      // ease in(first half)
    return (float)(time * time) / 2.0; // devide by 2 because we multiplied by 2
  time -= 1.0;
  return (float)((-1.0 * time * (time - 2.0) + 1.0) / 2.0);
}
float LedMacro::_cubic_easeinout(float time)
{
  time *= 2.0;
  if (time < 1.0) // ease in(first half)
    return (float)(time * time * time) / 2.0;
  time -= 2.0;
  return (float)(((time * time * time) + 2.0) / 2.0);
}

uint8_t LedMacro::_lerp(int16_t point_a, int16_t point_b, float time)
{
  return ((((int16_t)point_b - (int16_t)point_a) * (float)time) + (int16_t)point_a);
}

////////////////////////////////////////////////////////
//////////////// LedMacroManager source ////////////////

LedMacroManager::LedMacroManager(LedMacro *LedMacro_Pointer, uint8_t numberOfInstances)
{
  _LedMacro_Pointer = LedMacro_Pointer;
  _numberOfInstances = numberOfInstances;
}

void LedMacroManager::run()
{
  for (uint8_t x = 0; x < _numberOfInstances; x++)
    _LedMacro_Pointer[x].run();
}
bool LedMacroManager::run(uint8_t &ref)
{
  bool returnValue = true;
  for (uint8_t x = 0; x < _numberOfInstances; x++)
  {
    _LedMacro_Pointer[x].run();
    if (_LedMacro_Pointer[x].refValue() == &ref)
      returnValue = _LedMacro_Pointer[x].ready();
  }
  return returnValue;
}
bool LedMacroManager::ready(uint8_t &ref)
{
  for (uint8_t x = 0; x < _numberOfInstances; x++)
  {
    if (_LedMacro_Pointer[x].refValue() == &ref)
      return _LedMacro_Pointer[x].ready();
  }
  return true;
}
void LedMacroManager::stop(uint8_t &ref)
{
  for (uint8_t x = 0; x < _numberOfInstances; x++)
  {
    if (_LedMacro_Pointer[x].refValue() == &ref)
      _LedMacro_Pointer[x].stop();
  }
}

// transition functions:
bool LedMacroManager::lineEase(uint8_t &ref, uint8_t target, uint32_t transitionTime)
{
  uint8_t minMacro = 255;
  for (uint8_t x = 0; x < _numberOfInstances; x++)
  {
    if (minMacro == 255 && _LedMacro_Pointer[x].ready())
      minMacro = x;
    if (_LedMacro_Pointer[x].refValue() == &ref)
    {
      _LedMacro_Pointer[x].lineEase(ref, target, transitionTime);
      return true;
    }
  }
  if (minMacro != 255)
  {
    _LedMacro_Pointer[minMacro].lineEase(ref, target, transitionTime);
    return true;
  }
  return false;
}
bool LedMacroManager::quadEase(uint8_t &ref, uint8_t target, uint32_t transitionTime)
{
  uint8_t minMacro = 255;
  for (uint8_t x = 0; x < _numberOfInstances; x++)
  {
    if (minMacro == 255 && _LedMacro_Pointer[x].ready())
      minMacro = x;
    if (_LedMacro_Pointer[x].refValue() == &ref)
    {
      _LedMacro_Pointer[x].quadEase(ref, target, transitionTime);
      return true;
    }
  }
  if (minMacro != 255)
  {
    _LedMacro_Pointer[minMacro].quadEase(ref, target, transitionTime);
    return true;
  }
  return false;
}
bool LedMacroManager::cubicEase(uint8_t &ref, uint8_t target, uint32_t transitionTime)
{
  uint8_t minMacro = 255;
  for (uint8_t x = 0; x < _numberOfInstances; x++)
  {
    if (minMacro == 255 && _LedMacro_Pointer[x].ready())
      minMacro = x;
    if (_LedMacro_Pointer[x].refValue() == &ref)
    {
      _LedMacro_Pointer[x].cubicEase(ref, target, transitionTime);
      return true;
    }
  }
  if (minMacro != 255)
  {
    _LedMacro_Pointer[minMacro].cubicEase(ref, target, transitionTime);
    return true;
  }
  return false;
}
bool LedMacroManager::set(uint8_t &ref, uint8_t target, uint32_t delay_ms)
{
  uint8_t minMacro = 255;
  for (uint8_t x = 0; x < _numberOfInstances; x++)
  {
    if (minMacro == 255 && _LedMacro_Pointer[x].ready())
      minMacro = x;
    if (_LedMacro_Pointer[x].refValue() == &ref)
    {
      _LedMacro_Pointer[x].set(ref, target, delay_ms);
      return true;
    }
  }
  if (minMacro != 255)
  {
    _LedMacro_Pointer[minMacro].set(ref, target, delay_ms);
    return true;
  }
  return false;
}
bool LedMacroManager::delay(uint8_t &ref, uint32_t delay_ms)
{
  uint8_t minMacro = 255;
  for (uint8_t x = 0; x < _numberOfInstances; x++)
  {
    if (minMacro == 255 && _LedMacro_Pointer[x].ready())
      minMacro = x;
    if (_LedMacro_Pointer[x].refValue() == &ref)
    {
      _LedMacro_Pointer[x].delay(ref, delay_ms);
      return true;
    }
  }
  if (minMacro != 255)
  {
    _LedMacro_Pointer[minMacro].delay(ref, delay_ms);
    return true;
  }
  return false;
}

void LedMacroManager::mod(uint8_t &ref, float transitionModifier)
{
  for (uint8_t x = 0; x < _numberOfInstances; x++)
  {
    if (_LedMacro_Pointer[x].refValue() == &ref)
      _LedMacro_Pointer[x].mod(transitionModifier);
  }
}

void LedMacroManager::mod_t(uint8_t &ref, uint32_t transitionTime)
{
  for (uint8_t x = 0; x < _numberOfInstances; x++)
  {
    if (_LedMacro_Pointer[x].refValue() == &ref)
      _LedMacro_Pointer[x].mod_t(transitionTime);
  }
}

void LedMacroManager::fps(uint16_t set_fps)
{
  for (uint8_t x = 0; x < _numberOfInstances; x++)
    _LedMacro_Pointer[x].fps(set_fps);
}
#endif
