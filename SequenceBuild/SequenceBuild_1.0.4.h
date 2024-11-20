// SequenceBuild
//.h
/*
  changes from v1.0.2:
    - Added functionality to reset/clear current running sequence
    - improved stability for use with interrupts
 */
#ifndef SequenceBuild_h
#define SequenceBuild_h

#include "Arduino.h"

class SequenceBuild
{
public:
  void run(); // runs in the background keeps up the sequence

  void setSequence(uint8_t (*)(uint8_t, SequenceBuild *), uint8_t, bool);         // set a new sequence and start running it only if it isn't already running and a higher priority sequence is not currently running. input(sequenceFunct, indexToStartFrom, forceFirstSter(default false/wait for current sequence to finish))
  void setSequence(uint8_t (*)(uint8_t, SequenceBuild *));                        // set sequence without running it, will stop whichever sequence is currently running(except priority sequences). Use "start" to begin sequence
  void setPrioritySequence(uint8_t (*)(uint8_t, SequenceBuild *), uint8_t, bool); // set and start a priority sequence. can only be interrupted with "stop()"
  void setPrioritySequence(uint8_t (*)(uint8_t, SequenceBuild *));                // set a priority sequence. use "start" to begin
  void resetPriority();                                                           // set current sequence priority to normal. this will allow any other sequences to take over
  void reset();                                                                   // reset sequence function and stop all. This is used so the next sequence will run
  void loop(uint8_t);                                                             // loop a sequence back to a different index/use inside sequence funct. input(indexToJumpTo)
  void start(uint8_t, bool);                                                      // same as "setSequence()" but runs current funct instead of setting new. input(indexToStart, forceStart(default false))
  void stop();                                                                    // stop current sequence/put on standby. use "resume()" to continue sequence. resets priority
  void resume();                                                                  // resumes current sequence

  uint8_t index(); // returns current running index

private:
  uint8_t _index;
  uint8_t (*_funct)(uint8_t, SequenceBuild *);
  bool _run = false;
  bool _forceFirstStep = false;

  bool _currentPriority = false; // false = normal, true = priority
};

//.cpp

void SequenceBuild::run()
{
  if (_funct != nullptr)
    if ((_run && _funct(255, this) == 1) || _forceFirstStep)
    {
      _forceFirstStep = false;
      _index++;
      if (_funct(_index, this) == 2)
        stop();
    }
}

void SequenceBuild::setSequence(uint8_t (*funct)(uint8_t, SequenceBuild *), uint8_t startIndex, bool forceFirstStep = true)
{
  if (funct != _funct && !_currentPriority)
  {
    _funct = funct;
    start(startIndex, forceFirstStep);
  }
}
void SequenceBuild::setSequence(uint8_t (*funct)(uint8_t, SequenceBuild *))
{
  stop();
  _funct = funct;
  _currentPriority = false;
}
void SequenceBuild::setPrioritySequence(uint8_t (*funct)(uint8_t, SequenceBuild *), uint8_t startIndex, bool forceFirstStep = true)
{
  if (funct != _funct && !_currentPriority) // priority sequence cannot interrupt another priority sequence
  {
    _currentPriority = true;
    _funct = funct;
    start(startIndex, forceFirstStep);
  }
}
void SequenceBuild::setPrioritySequence(uint8_t (*funct)(uint8_t, SequenceBuild *))
{
  stop();
  _funct = funct;
  _currentPriority = true;
}
void SequenceBuild::resetPriority()
{
  _currentPriority = false;
}
void SequenceBuild::reset()
{
  _funct = nullptr;
  _currentPriority = false;
  _run = false;
}
void SequenceBuild::loop(uint8_t index)
{
  _index = index - 1;
}
void SequenceBuild::start(uint8_t startIndex, bool forceFirstStep = true)
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
void SequenceBuild::resume()
{
  _run = true;
}

uint8_t SequenceBuild::index()
{
  return _index + 1;
}

#define SB_FUNCT(funct, cond)                          \
  uint8_t funct(uint8_t _Index_, SequenceBuild *_this) \
  {                                                    \
    uint8_t _IndexTest_ = 0;                           \
    if (_Index_ == 255)                                \
      return ((cond) ? 1 : 0);

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
