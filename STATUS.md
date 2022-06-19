# SimAVR Status

## Current Position

This development has exceeded my estimates for effort required and overall code size.

Some of this is a result of feature creep with out doubt, but primarily this has been
caused by simply under estimating the complexities of actually simulating a device
even as simple as an ATMega328P MCU.  The requirement for accurate simultations of the
devices embedded in the MCU on top of the simulation of the CPU itself is a continued
source of additional requirements which (left unsatisfied) cause the execution of known
good executables to diverge from the same executable running on the real hardware.

This is solveable but requires continued and focused work on devices
