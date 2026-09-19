#pragma once

class MachineState
{
  public:
    virtual ~MachineState() = default;

    virtual void reset() = 0;

    virtual void enter() = 0;
    virtual bool isEnterComplete() const = 0;

    virtual void execute() = 0;

    virtual void exit() = 0;
    virtual bool isExitComplete() const = 0;
};
