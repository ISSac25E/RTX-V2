SequenceBuild change log:
  SequenceBuild allows you to create a simple non-blocking sequence that is managed in the background
  Very useful if you need to create a simple or complex sequence with an led
  It is not intended or designed for high speed patterns such as in data communications,
  rather it was intended for simple gui's with leds or possibly other forms of output such as a buzzer

v1.0.1 changes:
  - "setSequence()" will now only start the sequence if it isn't already running
    this allows for you to continuously call the same sequence without worrying about it restarting over and over
    it will only run from the start on the first call
    this is useful af a sequence is dependent on a variable that might change from time to time

v1.0.2 changes:
  - you can now set one of two priorities with sequences(normal-priority or high-priority)
    if any sequence with normal-priority is called and a high-priority sequence is currently running,
    the called sequence will be ignored until the running sequence is stopped or resets its priority