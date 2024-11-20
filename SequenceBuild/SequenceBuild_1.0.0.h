// SequenceBuild
//.h
#ifndef SequenceBuild_h
#define SequenceBuild_h

#include "Arduino.h"

class SequenceBuild
{
public:
  void run(); // runs in the background keeps up the sequence

  void setSequence(uint8_t (*)(uint8_t), uint8_t, bool); // set a new sequence and start running it. input(sequenceFunct, indexToStartFrom, forceFirstSter(default false/wait for current sequence to finish))
  void loop(uint8_t);                                    // loop a sequence back to a different index/use inside sequence funct. input(indexToJumpTo)
  void start(uint8_t, bool);                             // same as "setSequence()" but runs currect funct instead of setting new. input(indexToStart, forceStart(default false))
  void stop();                                           // stop current sequence/bput on standby. use "start()" to resume again

  uint8_t index(); // returns current running index

private:
  uint8_t _index;
  uint8_t (*_funct)(uint8_t);
  bool _run = false;
  bool _forceFirstStep = false;
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
  _funct = funct;
  start(startIndex, forceFirstStep);
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
