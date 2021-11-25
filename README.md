-------------
I94100 BSP - V3.05.004 2020/02/17
-------------
[Fixed]
1.	SPI0/2 status register issue. Busy flag might work improperly due to harware glitch.
	Fixed by checking RX&TX FIFO status.
2.	VAD_Wakeup sample has incorrect BIQ coefficient.

[Revised]
1. Revise "CLK_EnablePLL" API for  more accurate parameters calculation.

[Add]
1.	"PWM_Timer" Demonstrate how to use PWM0 channel 0 to work as timer.
