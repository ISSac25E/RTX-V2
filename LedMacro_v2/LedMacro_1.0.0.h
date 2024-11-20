// LedMacro
//.h
#ifndef LedMacro_h
#define LedMacro_h

#include "Arduino.h"

class LedMacro
{
  /*
    note:
      - reference is required by every macro because it can be used as a form of identification
     */
public:
  bool run();   // run led macro. return true if ready for another macro. false if still running a macro
  bool ready(); // returns if ready for another macro/equivilent of run, just without running the macro(faster)
  void stop();  // stop macros, (ready with return true)

  // transition functions:
  void lineEase(uint8_t &, uint8_t, uint8_t);  // linear ease/fade, input(value(reference), target(0 - 255), frames(frames to run macro))
  void quadEase(uint8_t &, uint8_t, uint8_t);  // quadratic ease-in and ease-out, inputs(value(reference), target(0 - 255), frames)
  void cubicEase(uint8_t &, uint8_t, uint8_t); // cubic ease-in and ease-out, inputs(value(reference), target(0 - 255), frames)
  void set(uint8_t &, uint8_t, uint16_t);      // sets value to target and delays set amount(ms). inputs(value(reference), target(0 - 255), delay(ms))
  void delay(uint8_t &, uint16_t);             // delay all macros for set time. inputs(value(reference), delay(ms))

  void fps(uint16_t); // set fps(frames(refreshes) per second). default is set on 60 fps. fps is only used for linear and easing

  uint8_t *refValue();

private:
  float _quad_easeinout(float);  // calculate quadEase function. inputs(t(time)). outputs v(horizontal percentage)
  float _cubic_easeinout(float); // calculate cubicEase function. inputs(t(time)). outputs v(horizontal percentage)

  uint8_t _lerp(int16_t, int16_t, float); // Linear Interpolation function, inputs (point_a, point_b, percentage(0.0 - 1.0)). outputs value between point a and point b relative to percentage input

  bool _macroRun = false; // determines wether a macro is currently running

  uint8_t _transitionType; // use to distinguish between "set"(0), "linear"(1), "quad"(2), and "cubic"(3)
  uint8_t _startVal;       // start value captured at the call of a transition. laster will be used with "_lerp"
  uint8_t _endVal;         // end value captured at the call of a transition. also will be later used with "_lerp"
  uint8_t _frameCount;     // used to keep track how many frames completed during transition. always set to 0 when transition called
  uint8_t _totalFrames;    // set how many total frames transition needs to run for
  uint16_t _rate_ms = 16;  // delay between each transition step in ms. defaults to aprox 60 fps(16.667ms)
  uint16_t _nextDelay;     // value set when transition functions called. used during run
  uint32_t _timer;         // timer for keeping track of transitions. used with "millis()"

  uint8_t *_refValue; // pointer of referenced value giving in each transition function
};

class LedMacroManager
{
public:
  LedMacroManager(LedMacro *, uint8_t); // takes an array of LedMacro's and controls them. inputs(LedMacro(pointer), numberOfLedMacro(in the array))

  void run();            // runs all LedMacros in background
  bool run(uint8_t &);   // runs all LedMacros in background
  bool ready(uint8_t &); // returns if macro ready using reference value. inputs(refValue(used and value identification))
  void stop(uint8_t &);  // stop macro for given referenced value

  // transition functions:
  bool lineEase(uint8_t &, uint8_t, uint8_t);  // linear ease/fade, input(value(reference), target(0 - 255), frames(frames to run macro)). outputs true if macro successfully attached to val, false otherwise
  bool quadEase(uint8_t &, uint8_t, uint8_t);  // quadratic ease-in and ease-out, inputs(value(reference), target(0 - 255), frames). outputs true if macro successfully attached to val, false otherwise
  bool cubicEase(uint8_t &, uint8_t, uint8_t); // cubic ease-in and ease-out, inputs(value(reference), target(0 - 255), frames). outputs true if macro successfully attached to val, false otherwise
  bool set(uint8_t &, uint8_t, uint16_t);      // sets value to target and delays set amount(ms). inputs(value(reference), target(0 - 255), delay(ms)). outputs true if macro successfully attached to val, false otherwise
  bool delay(uint8_t &, uint16_t);             // delay all macros for set time. inputs(value(reference), delay(ms)). outputs true if macro successfully attached to val, false otherwise

  void fps(uint16_t); // sets GLOBAL fps(frames(refreshes) per second) for all LedMacros. default is set on 60 fps. fps is only used for linear and easing transitions
private:
  LedMacro *_LedMacro_Pointer;
  uint8_t _numberOfInstances;
};

//.cpp

bool LedMacro::run()
{
  if (_macroRun)
  {
    if (millis() - _timer >= _nextDelay)
    {
      if (_transitionType == 0) // setDelay transition
      {
        _macroRun = false; // delay already done, value set at the beginning of transition call. disable macros
      }
      else if (_transitionType == 1) // linearFade transition
      {
        _timer += _nextDelay; // absolute timing, make sure led runs exactly at specified frameRate
        // _timer = millis(); // relative timing, use if absolute timing becomes unstable
        _frameCount++;                                                                      // increment framecount before setting value
        *_refValue = _lerp(_startVal, _endVal, ((float)_frameCount / (float)_totalFrames)); // calculate linear fade according to framecount/frames

        if (_totalFrames <= _frameCount) // check if transition is done. if so, set "macroRun" to false
          _macroRun = false;
      }
      else if (_transitionType == 2) // quadraticEase transition
      {
        _timer += _nextDelay; // absolute timing, make sure led runs exactly at specified frameRate
        // _timer = millis(); // relative timing, use if absolute timing becomes unstable
        _frameCount++;                                                                                     // increment framecount before setting value
        *_refValue = _lerp(_startVal, _endVal, _quad_easeinout((float)_frameCount / (float)_totalFrames)); // calculate linear fade according to framecount/frames

        if (_totalFrames <= _frameCount) // check if transition is done. if so, set "macroRun" to false
          _macroRun = false;
      }
      else if (_transitionType == 3) // cubicEase transition
      {
        _timer += _nextDelay; // absolute timing, make sure led runs exactly at specified frameRate
        // _timer = millis(); // relative timing, use if absolute timing becomes unstable
        _frameCount++;                                                                                      // increment framecount before setting value
        *_refValue = _lerp(_startVal, _endVal, _cubic_easeinout((float)_frameCount / (float)_totalFrames)); // calculate linear fade according to framecount/frames

        if (_totalFrames <= _frameCount) // check if transition is done. if so, set "macroRun" to false
          _macroRun = false;
      }
      else                 // unknown transition
        _macroRun = false; // set "macroRun" to false so we dont end up with a stuck macro
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

void LedMacro::lineEase(uint8_t &ref, uint8_t target, uint8_t frames)
{
  _refValue = &ref; // save reference value
  if (frames)       // only start macro if there are frames. if no frames and macro will run, divide by 0 error will occur
  {
    _macroRun = true;
    _transitionType = 1;    // set transition type, delay(0), linearEase(1), quadraticEase(2), cubicEase(3)
    _startVal = *_refValue; // start value is the current refValue
    _endVal = target;       // end value is target value
    _totalFrames = frames;
    _frameCount = 0; // reset frame count
    _nextDelay = _rate_ms;
    _timer = millis(); // reset timer for transition
  }
  else
  {
    _macroRun = false;
    *_refValue = target; // simply set value to target and exit
  }
}
void LedMacro::quadEase(uint8_t &ref, uint8_t target, uint8_t frames)
{
  _refValue = &ref; // save reference value
  if (frames)       // only start macro if there are frames. if no frames and macro will run, divide by 0 error will occur
  {
    _timer = millis(); // reset timer for transition
    _macroRun = true;
    _transitionType = 2;    // set transition type, delay(0), linearEase(1), quadraticEase(2), cubicEase(3)
    _startVal = *_refValue; // start value is the current refValue
    _endVal = target;       // end value is target value
    _totalFrames = frames;
    _frameCount = 0; // reset frame count
    _nextDelay = _rate_ms;
  }
  else
  {
    _macroRun = false;
    *_refValue = target; // simply set value to target and exit
  }
}
void LedMacro::cubicEase(uint8_t &ref, uint8_t target, uint8_t frames)
{
  _refValue = &ref; // save reference value
  if (frames)       // only start macro if there are frames. if no frames and macro will run, divide by 0 error will occur
  {
    _timer = millis(); // reset timer for transition
    _macroRun = true;
    _transitionType = 3;    // set transition type, delay(0), linearEase(1), quadraticEase(2), cubicEase(3)
    _startVal = *_refValue; // start value is the current refValue
    _endVal = target;       // end value is target value
    _totalFrames = frames;
    _frameCount = 0; // reset frame count
    _nextDelay = _rate_ms;
  }
  else
  {
    _macroRun = false;
    *_refValue = target; // simply set value to target and exit
  }
}
void LedMacro::set(uint8_t &ref, uint8_t target, uint16_t delay_ms)
{
  _refValue = &ref; // save reference value
  *_refValue = target;
  if (delay_ms) // pointless without a delay
  {
    _timer = millis();
    _macroRun = true;
    _transitionType = 0;
    _nextDelay = delay_ms;
  }
  else
  {
    _macroRun = false;
  }
}
void LedMacro::delay(uint8_t &ref, uint16_t delay_ms)
{
  _refValue = &ref; // save reference value
  if (delay_ms)     // pointless without a delay
  {
    _timer = millis();
    _macroRun = true;
    _transitionType = 0;
    _nextDelay = delay_ms;
  }
  else
  {
    _macroRun = false;
  }
}

void LedMacro::fps(uint16_t set_fps)
{
  if (set_fps)
    _rate_ms = (1000 / set_fps);
  else
    _rate_ms = 1;
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
bool LedMacroManager::lineEase(uint8_t &ref, uint8_t target, uint8_t frames)
{
  uint8_t minMacro = 255;
  for (uint8_t x = 0; x < _numberOfInstances; x++)
  {
    if (minMacro == 255 && _LedMacro_Pointer[x].ready())
      minMacro = x;
    if (_LedMacro_Pointer[x].refValue() == &ref)
    {
      _LedMacro_Pointer[x].lineEase(ref, target, frames);
      return true;
    }
  }
  if (minMacro != 255)
  {
    _LedMacro_Pointer[minMacro].lineEase(ref, target, frames);
    return true;
  }
  return false;
}
bool LedMacroManager::quadEase(uint8_t &ref, uint8_t target, uint8_t frames)
{
  uint8_t minMacro = 255;
  for (uint8_t x = 0; x < _numberOfInstances; x++)
  {
    if (minMacro == 255 && _LedMacro_Pointer[x].ready())
      minMacro = x;
    if (_LedMacro_Pointer[x].refValue() == &ref)
    {
      _LedMacro_Pointer[x].quadEase(ref, target, frames);
      return true;
    }
  }
  if (minMacro != 255)
  {
    _LedMacro_Pointer[minMacro].quadEase(ref, target, frames);
    return true;
  }
  return false;
}
bool LedMacroManager::cubicEase(uint8_t &ref, uint8_t target, uint8_t frames)
{
  uint8_t minMacro = 255;
  for (uint8_t x = 0; x < _numberOfInstances; x++)
  {
    if (minMacro == 255 && _LedMacro_Pointer[x].ready())
      minMacro = x;
    if (_LedMacro_Pointer[x].refValue() == &ref)
    {
      _LedMacro_Pointer[x].cubicEase(ref, target, frames);
      return true;
    }
  }
  if (minMacro != 255)
  {
    _LedMacro_Pointer[minMacro].cubicEase(ref, target, frames);
    return true;
  }
  return false;
}
bool LedMacroManager::set(uint8_t &ref, uint8_t target, uint16_t delay_ms)
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
bool LedMacroManager::delay(uint8_t &ref, uint16_t delay_ms)
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

void LedMacroManager::fps(uint16_t set_fps)
{
  for (uint8_t x = 0; x < _numberOfInstances; x++)
    _LedMacro_Pointer[x].fps(set_fps);
}
#endif
