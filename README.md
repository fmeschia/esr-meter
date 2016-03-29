ESR Meter
=========
An Arduino-based battery internal resistance meter.

Introduction
------------
This circuit measures the internal resistance of a battery by measuring the voltage difference at the battery terminals when a load is applied and then removed. By measuring the difference in voltage and the difference in current, the resistance is readily determined via Ohm's law.

Design
------

Voltage and current measurement is performed using a 4-wire (Kelvin) technique: current is measured by a sensing resistance along the "hot" high-current path which connects the battery to the load, and voltage is sensed through two "cold" leads, that carry no current. This also allows for internal resistance measurement of the individual cells in a multi-cell battery, as long as the individual cell voltages are available at a balance tap.

The hot loop is made up by:

*  a Schottky diode for protection against reverse polarity
*  a LM317 regulator used as a current source
*  a FQP30N06L n-channel MOSFET used as a switch
*  a 1 Ohm, 1% precision, current sensing resistance

Mainly because of the LM317 dropout characteristics, the minimum voltage that can be applied to the "
hot loop is about 3.5 V. This means that a single-cell LiPo battery can be measured, as long as it's not too low on voltage, but not, for instance, a single NiMH cell.

The hot loop is switched on and off by an ATMega 328P microcontroller, clocked at 8 MHz. The program running on the micro switches on the load for 500 ms, then switches it off for 1500 ms, before starting a new cycle. In this way, the 25% duty cycle helps in keeping the average power dissipation acceptable for a LM317 with a passive heat sink, even when sinking current from a 4S LiPo battery.

The microcontroller's analog inputs are used to measure the voltage across the current sensing resistor in the hot loop, and to sense the voltage difference. Since the voltage level at the positive terminal of the battery could well be above the ADC reference, the measurement is AC-coupled: the sensed voltages are passed through two one-pole low-pass filters, with a 0.1 s time constant, and the filter outputs are then sampled by the ADC.

The microcontroller also drives a standard HD44780 2x16 LCD display module, to output the result of the internal resistance computation at the end of each 2-second measurement cycle. The measurement is automatically refreshed every 2 seconds, for as long as the unit is powered. 

Power for the circuit may come either directly from the battery under test (requires the use a LDO regulator and a 3.3 V display module), or from a separate battery.

