// SequenceBuild
//.h
/*
  changes from v1.0.1:
    - you can now set one of two priorities with sequences(normal-priority or high-priority)
      if any sequence with normal-priority is called and a high-priority sequence is currently running,
      the called sequence will be ignored until the running sequence is stopped or resets its priority
 */
#ifndef SequenceBuild_h
#define SequenceBuild_h

#include "Arduino.h"

class SequenceBuild
{
public:
  void run(); // runs in the background keeps up the sequence

  void setSequence(uint8_t (*)(uint8_t), uint8_t, bool);         // set a new sequence and start running it only if it isn't already running and a higher priority sequence is not currently running. input(sequenceFunct, indexToStartFrom, forceFirstSter(default false/wait for current sequence to finish))
  void setSequence(uint8_t (*)(uint8_t));                        // set sequence without running it, will stop whichever sequence is currently running(except priority sequences). Use "start" to begin sequence
  void setPrioritySequence(uint8_t (*)(uint8_t), uint8_t, bool); // set and start a priority sequence. can only be interrupted with "stop()" or another priority sequence
  void setPrioritySequence(uint8_t (*)(uint8_t));                // set a priority sequence. use "start" to begin
  void resetPriority();                                          // set current sequence priority to normal. this will allow any other sequences to take over
  void loop(uint8_t);                                            // loop a sequence back to a different index/use inside sequence funct. input(indexToJumpTo)
  void start(uint8_t, bool);                                     // same as "setSequence()" but runs current funct instead of setting new. input(indexToStart, forceStart(default false))
  void stop();                                                   // stop current sequence(priority also)/put on standby. use "start()" to resume again

  uint8_t index(); // returns current running index

private:
  uint8_t _index;
  uint8_t (*_funct)(uint8_t);
  bool _run = false;
  bool _forceFirstStep = false;

  bool _currentPriority = false; // false = normal, true = priority
};

//.cpp

void SequenceBuild::run()
{
  if ((_run && _funct(255) == 1) || _forceFirstStep)
  {
    _forceFirstStep = false;
    _index++;
    if (_funct(_index) == 2)
      stop();
  }
}

void SequenceBuild::setSequence(uint8_t (*funct)(uint8_t), uint8_t startIndex, bool forceFirstStep = false)
{
  if (funct != _funct && !_currentPriority)
  {
    _funct = funct;
    start(startIndex, forceFirstStep);
  }
}
void SequenceBuild::setSequence(uint8_t (*funct)(uint8_t))
{
  if (!_currentPriority)
  {
    _funct = funct;
    stop();
  }
}
void SequenceBuild::setPrioritySequence(uint8_t (*funct)(uint8_t), uint8_t startIndex, bool forceFirstStep = false)
{
  if (funct != _funct)
  {
    _currentPriority = true;
    _funct = funct;
    start(startIndex, forceFirstStep);
  }
}
void SequenceBuild::setPrioritySequence(uint8_t (*funct)(uint8_t))
{
  _funct = funct;
  stop();
  _currentPriority = true;
}
void SequenceBuild::resetPriority()
{
  _currentPriority = false;
}
void SequenceBuild::loop(uint8_t index)
{
  _index = index - 1;
}
void SequenceBuild::start(uint8_t startIndex, bool forceFirstStep = false)
{
  _index = startIndex - 1;
  _forceFirstStep = forceFirstStep;
  _run = true;
}
void SequenceBuild::stop()
{
  _run = false;
  _currentPriority = false;
}

uint8_t SequenceBuild::index()
{
  return _index + 1;
}

#define SB_FUNCT(funct, cond)    \
  uint8_t funct(uint8_t _Index_) \
  {                              \
    uint8_t _IndexTest_ = 0;     \
    if (_Index_ == 255)          \
      return (cond ? 1 : 0);

#define SB_STEP(step)           \
  if (_IndexTest_++ == _Index_) \
  {                             \
    step                        \
  }                             \
  else

#define SB_END \
  return 2;    \
  return 0;    \
  }

#endif
