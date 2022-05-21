int aht10_read()
{
       unsigned char ath10[16];
       ath10[0] = 0x33;
       ath10[1] = 0x00;
       i2c_write_block(0x38, 0xac, ath10, 2);
       vTaskDelay(20);
       i2c_read(0x38, 0x33, ath10, 7);
       //for(int a =0; a<7; a++) printf(" %02x", ath10[a]);
       //printf("\n");

       humidity = (int)(1000*(float)(ath10[1]<<12) + (ath10[2]<<4) + ((ath10[3] & 0xf0)>>4))/(1<<20);
       temperature = (int)(10*(-50 + 200 * ((float)((ath10[3]%16)<<16) + (ath10[4]<<8) + ath10[5])/(1<<20)));

       //printf(" hum %d%%   temp %dC   %dF\n", humidity, temperature, 320+(int)(1.8*temperature));

       return(0);
}
