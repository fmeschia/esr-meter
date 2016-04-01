ESR Meter
=========
An Arduino-based battery internal resistance meter.

Introduction
------------
This device measures the internal resistance of a battery by measuring the voltage difference at the battery terminals when a load is applied and then removed. By measuring the difference in voltage and the difference in current, the resistance is readily determined via Ohm's law.

Design goals
------------
* Measure internal resistance for batteries with voltages ranging from a 4-cell NiXX to a 4S LiPo pack
* Resistance range up to 1 Ohm
* Ability to do 4-wire measurements
* Target precision: 2% ± 1 LSB
* Ability to draw power either from the battery under test, or from a separate battery
* Powered by the Atmel ATMega328P microcontroller (so that it can be prototyped with Arduino)
* Low cost!

Design
------

This device measures internal resistance by using Ohm's law: R = ∆V/∆I. The battery is first connected to a load, then the load is removed. The voltage at the battery teminals is measured just before removing the load, and just after the load is removed. The current drawn from the battery is also measured, almost simultaneously with the two voltage measurements.

Voltage and current measurement is performed using a 4-wire (Kelvin) technique: current is measured by a sensing resistor along the "hot" high-current path which connects the battery to the load, and voltage is sensed through two "cold" leads, that carry no current. This also allows for internal resistance measurement of the individual cells in a multi-cell battery, as long as the individual cell voltages are available at a balance tap.

One of the problems that must be solved is how to measure voltages (like the two battery under test terminals) which may exceed the ADC reference voltage. The ATMega328P microcontroller can be powered with 5.5V maximum, so if we were to do a direct measurement (battery terminals direct to ADC inputs) we would be only be able to measure a 4-cell NiXX battery... if it's not fully charged.

One possibility to overcome this problem would be to use differential analog amplifiers to bring the voltages within the measurement range. The gain and the baseline of the amplifiers could be adjusted by the micro with digital potentiometers, to adapt to different battery voltages. The drawback is the number of additional components, which influences the cost of the device.

Instead of going this route, we decided to use "AC coupling". The voltages sensed at the battery terminals are filtered through one-pole (RC) low-pass filters, so that their DC component is removed. If the time constant of the fiter is sufficiently long compared to the load-switching-to-measurement delay, we would only measure the actual change in voltage, with negligible error (compare, in the scope plot below, the yellow DC-coupled positive terminal voltage with the green AC-coupled signal).

![DC vs. AC measure of ∆V](screenshots/measure1.png "DC (yellow) vs. AC (green) measure of ∆V")

In order to ensure reasonable precision with low cost, we also decided to reference the ADCs to the internal bandgap voltage reference of the micro. This means that, if we want to meet the 1 Ohm maximum range stated in the design goals, we need to draw less than about 1.1 A (we designed the prototype for 0.8 A).

Such an electronic load can be conveniently built with a LM317 regulator used as a current source. This will be the heart of the "hot" loop. Additionally, the hot loop will also include: 

*  a Schottky diode for protection against reverse polarity
*  a FQP30N06L n-channel MOSFET used as a switch
*  a 1 Ohm, 1% precision, current sensing resistance

![Hot loop schematic](screenshots/hotloop.png)

Mainly because of the LM317 dropout characteristics, the minimum voltage that needs to be applied to the hot loop if we want the design current draw is about 4 V. A lower voltage would lead to a lower current figure, as long as it is enough to correctly bias the BJTs inside the LM317 (after that, the device won't work any more). We tested the circuit with a single LiPo cell discharged to 3.5V, without any problem.

The hot loop is switched on and off by the microcontroller, which we decided to power at 3.3 V with an external clock runing at 8 MHz. The program running on the micro switches on the load for 500 ms, then switches it off for 1500 ms, before starting a new cycle. In this way, the 25% duty cycle helps in keeping the average power dissipation acceptable for a LM317 with a passive heat sink, even when sinking current from a 4S LiPo battery.

The microcontroller also drives a standard HD44780 2x16 LCD display module, to output the result of the internal resistance computation at the end of each 2-second measurement cycle. The measurement is automatically refreshed every 2 seconds, for as long as the unit is powered. 

Power for the circuit may come either directly from the battery under test (requires the use of an LDO regulator and a 3.3 V display module), or from a separate battery.

