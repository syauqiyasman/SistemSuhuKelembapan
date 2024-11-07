#include <asf.h>
#include <stdio.h>
#include <ioport.h>
#include <board.h>
#include "DHT22.h"
#define TEMP_THRESHOLD 25
#define TEMP_THRESHOLD2 26
#define HUM_THRESHOLD 42.25

uint8_t counter = 0;

void PWM_Init(void)
{
	// Fungsi inisialisasi PWM
	PORTC.DIR |= PIN0_bm;
	TCC0.CTRLA = PIN1_bm;
	TCC0.CTRLB = (PIN4_bm) | (PIN2_bm) | (PIN1_bm);
	TCC0.PER = 1000;
	TCC0.CCA = 0;
}

void init_external_interrupt(void) //Fungsi untuk deklarasi interrupt
{
	PORTF.DIR &= ~(1 << 1);
	PORTF.PIN1CTRL &= (3<<3);
	PORTF.PIN1CTRL |= (0x03 << 0);
	PORTF.INT0MASK = (1 << 1);
	PORTF.INTCTRL = (0x01 << 0);
	PMIC.CTRL |= 0x01;
	SREG |= (1<<7);
}

ISR(PORTF_INT0_vect) //Fungsi untuk penentuan port dan pin interrupt
{
	PORTB.OUTTGL = 0x01;
	delay_ms(100);
}

void fan_control_init(void) //Fungsi untuk penentuan port dan pin kipas
{
	PORTB.DIR |= PIN0_bm;
	PORTB.OUT |= PIN0_bm;
}

void update_speed(uint8_t counter_value) //Fungsi untuk mengatur kecepatan kipas

{
	if (counter_value == 0) {
		PORTB.OUT &= ~PIN0_bm; //Menyalakan PORT B PIN 0

		//Mengubah PWM menjadi 0
		PORTC.OUTCLR = PIN0_bm;
		TCC0.CCA = 0;
		} else {
		PORTB.OUT &= ~PIN0_bm; //Menyalakan PORT B PIN 0

		//Mengubah PWM sesuai counter
		PORTC.OUTSET = PIN0_bm;
		TCC0.CCA = (counter_value * 1000) / 2;
	}
}

void pompa_control_init(void) //Fungsi untuk penentuan port dan pin pompa
{
	PORTA.DIR |= PIN0_bm;
	PORTA.OUT |= PIN0_bm;
}

int main(void)
{
	//Definisi variabel yang diperlukan
	char printbuff[10];
	float temp;
	float hum;
	float last_suhu = 26.0;
	float last_hum = 40.0;
	uint8_t count = 200;

	//Pemanggilan fungsi - fungsi inisialisasi
	init_external_interrupt();
	board_init();
	sysclk_init();
	gfx_mono_init();
	gpio_set_pin_high(LCD_BACKLIGHT_ENABLE_PIN);
	DHT22_init();
	fan_control_init();
	pompa_control_init();
	PWM_Init();

	//Loop Utama
	while (1)
	{
		//Pengecekan interrupt
		bool btn = PORTF.IN & (1 << 1);
		if(!btn){
			PORTB.OUTTGL = 0x02;
		}

		//Delay menunggu semua stabil
		count++;
		if(count >= 10)
		{
			//Pembacaan data dan kalibrasi
			count = 0;
			uint8_t status = DHT22_read(&temp, &hum);
			temp = temp*1.7;
			hum = hum/80;

			if (status) //Kondisi saat pembacaan data berhasil
			{

				//Definisi dan format string variabel kerja
				last_suhu = temp;
				last_hum = hum;
				printf("temp = %2.2f\n\r", temp);
				printf("hum = %2.2f\n\r", hum);

				//Pengecekan suhu terhadap batas yang ditentukan
				if (temp > TEMP_THRESHOLD && temp <= TEMP_THRESHOLD2) { //Kondisi saat suhu melewati batas pertama
					//Menyala dengan external power saja tanpa bantuan PWM
					PORTB.OUT &= ~PIN0_bm;
					counter = 0;
					update_speed(counter);
					} else if (temp > TEMP_THRESHOLD2) {  //Kondisi saat suhu melewati batas kedua
					//Menyala dengan bantuan PWM
					counter = 2;
					update_speed(counter);
					} else {
					PORTB.OUT |= PIN0_bm;
				}
				
				//Pengecekan kelembaban terhadap batas yang ditentukan
				if (hum < HUM_THRESHOLD) {
					PORTA.OUT &= ~PIN0_bm;
					} else {
					PORTA.OUT |= PIN0_bm;
				}

				//Pencetakan data ke LCD
				gfx_mono_draw_filled_rect(0, 0, 128, 32, GFX_PIXEL_CLR);
				gfx_mono_draw_string("Temp. ", 0, 0, &sysfont);
				dtostrf(temp, 2, 2, printbuff);
				gfx_mono_draw_string(printbuff, 50, 0, &sysfont);
				gfx_mono_draw_string("C", 100, 0, &sysfont);
				gfx_mono_draw_string("Hum. ", 0, 16, &sysfont);
				dtostrf(hum, 2, 2, printbuff);
				gfx_mono_draw_string(printbuff, 50, 16, &sysfont);
				gfx_mono_draw_string("%", 100, 16, &sysfont);
			}
			else
			{   //Kondisi ketika pembacaan data gagal, pencetakan data berhasil terakhir
				gfx_mono_draw_filled_rect(0, 0, 128, 32, GFX_PIXEL_CLR);
				gfx_mono_draw_string("L Temp. ", 0, 0, &sysfont);
				dtostrf(last_suhu, 2, 2, printbuff);
				gfx_mono_draw_string(printbuff, 50, 0, &sysfont);
				gfx_mono_draw_string("C", 100, 0, &sysfont);
				gfx_mono_draw_string("L Hum. ", 0, 16, &sysfont);
				dtostrf(last_hum, 2, 2, printbuff);
				gfx_mono_draw_string(printbuff, 50, 16, &sysfont);
				gfx_mono_draw_string("%", 100, 16, &sysfont);
			}
		}
		else
		{
			//Menunggu delay
			gfx_mono_draw_string("Starting...", 0, 0, &sysfont);
		}
	}
}
