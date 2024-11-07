#define DHT_DDR PORTD.DIR
#define DHT_PORT PORTD.OUT
#define DHT_PIN PORTD.IN

void DHT22_init(void)
{
	DHT_DDR |= PIN2_bm;		// Set PIN as output
	DHT_PORT |= PIN2_bm;		// Set high
}

uint8_t DHT22_read(float *dht_temperatura, float *dht_humedad)
{
	uint8_t bits[5];
	uint8_t i, j = 0;
	uint8_t contador;

	DHT_PORT &= ~PIN2_bm;		// Set low
	delay_ms(18);				// Wait for 18 ms
	DHT_PORT |= PIN2_bm;		// Set high
	DHT_DDR &= ~PIN2_bm;		// Set PIN as input
	
	// Wait for the DHT22 to pull the line low
	contador = 0;
	while (DHT_PIN & PIN2_bm)
	{
		delay_us(1);
		contador++;
		if (contador > 1000) // Timeout, adjust as necessary
		{
			DHT_DDR |= PIN2_bm; // Set PIN as output
			DHT_PORT |= PIN2_bm; // Set high
			return 0; // Timeout error
		}
	}

	// Read the bits
	for (j = 0; j < 5; j++)
	{
		uint8_t result = 0;
		for (i = 0; i < 8; i++)
		{
			while (!(DHT_PIN & PIN2_bm)); // Wait for the pin to go high
			delay_us(30);                  // Measure the duration of the bit

			if (DHT_PIN & PIN2_bm)
			result |= (1<<(7-i));      // If high, record a 1

			while (DHT_PIN & PIN2_bm);    // Wait for the pin to go low
		}
		bits[j] = result;                 // Store the read byte
	}

	DHT_DDR |= PIN2_bm;	// Set PIN as output
	DHT_PORT |= PIN2_bm;	// Set high
	
	// Check checksum
	if ((uint8_t)(bits[0] + bits[1] + bits[2] + bits[3]) == bits[4])
	{
		uint16_t rawhumedad = bits[0] << 8 | bits[1];
		uint16_t rawtemperatura = bits[2] << 8 | bits[3];
		
		if (rawtemperatura & 0x8000)
		{
			*dht_temperatura = (float)((rawtemperatura & 0x7fff) / 10.0) * -1.0;
		}
		else
		{
			*dht_temperatura = (float)(rawtemperatura) / 10.0;
		}

		*dht_humedad = (float)(rawhumedad) / 10.0;

		return 1; // Successful read
	}
	return 0; // Checksum failed
}
