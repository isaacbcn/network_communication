// hello.bus.45.c
// 9600 baud serial bus hello-world
// Neil Gershenfeld
// 11/24/10
// (c) Massachusetts Institute of Technology 2010
// Permission granted for experimental and personal use;
// license for commercial sale available from MIT.
//
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <string.h>
// This macro tells the identity of the node
#define node_id '0'

#define output(directions,pin) (directions |= pin) // set port direction for output
#define input(directions,pin) (directions &= (~pin)) // set port direction for input
// Todo: set and clear macros seemed to be reversed
#define set(port,pin) (port |= pin) // set port pin
#define clear(port,pin) (port &= (~pin)) // clear port pin
#define pin_test(pins,pin) (pins & pin) // test for port pin
#define bit_test(byte,bit) (byte & (1 << bit)) // test for bit set
#define bit_delay_time 100 // bit delay for 9600 with overhead
#define bit_delay() _delay_us(bit_delay_time) // RS232 bit delay
#define half_bit_delay() _delay_us(bit_delay_time/2) // RS232 half bit delay

#define led_port PORTB
#define led_direction DDRB
#define led_pin (1 << PB0)

#define serial_port PORTB
#define serial_direction DDRB
#define serial_pins PINB
#define serial_pin_in (1 << PB3)
#define serial_pin_out (1 << PB4)

void get_char(volatile unsigned char *pins, unsigned char pin, char *rxbyte) {
   // read character into rxbyte on pins pin
   // assumes line driver (inverts bits)
   *rxbyte = 0;
   // wait for start bit
   while (pin_test(*pins,pin));
   // delay to middle of first data bit
   half_bit_delay();
   bit_delay();
   // unrolled loop to read data bits
   if pin_test(*pins,pin)
      *rxbyte |= (1 << 0);
   else
      *rxbyte |= (0 << 0);
   bit_delay();
   if pin_test(*pins,pin)
      *rxbyte |= (1 << 1);
   else
      *rxbyte |= (0 << 1);
   bit_delay();
   if pin_test(*pins,pin)
      *rxbyte |= (1 << 2);
   else
      *rxbyte |= (0 << 2);
   bit_delay();
   if pin_test(*pins,pin)
      *rxbyte |= (1 << 3);
   else
      *rxbyte |= (0 << 3);
   bit_delay();
   if pin_test(*pins,pin)
      *rxbyte |= (1 << 4);
   else
      *rxbyte |= (0 << 4);
   bit_delay();
   if pin_test(*pins,pin)
      *rxbyte |= (1 << 5);
   else
      *rxbyte |= (0 << 5);
   bit_delay();
   if pin_test(*pins,pin)
      *rxbyte |= (1 << 6);
   else
      *rxbyte |= (0 << 6);
   bit_delay();
   if pin_test(*pins,pin)
      *rxbyte |= (1 << 7);
   else
      *rxbyte |= (0 << 7);
   // wait for stop bit
   bit_delay();
   half_bit_delay();
}
void put_char(volatile unsigned char *port, unsigned char pin, char txchar) {
   // send character in txchar on port pin
   //    assumes line driver (inverts bits)
   // start bit
   clear(*port,pin);
   bit_delay();
   //
   // unrolled loop to write data bits
   //
   if bit_test(txchar,0)
      set(*port,pin);
   else
      clear(*port,pin);
   bit_delay();
   if bit_test(txchar,1)
      set(*port,pin);
   else
      clear(*port,pin);
   bit_delay();
   if bit_test(txchar,2)
      set(*port,pin);
   else
      clear(*port,pin);
   bit_delay();
   if bit_test(txchar,3)
      set(*port,pin);
   else
      clear(*port,pin);
   bit_delay();
   if bit_test(txchar,4)
      set(*port,pin);
   else
      clear(*port,pin);
   bit_delay();
   if bit_test(txchar,5)
      set(*port,pin);
   else
      clear(*port,pin);
   bit_delay();
   if bit_test(txchar,6)
      set(*port,pin);
   else
      clear(*port,pin);
   bit_delay();
   if bit_test(txchar,7)
      set(*port,pin);
   else
      clear(*port,pin);
   bit_delay();
   // stop bit
   set(*port,pin);
   bit_delay();
   // char delay
   bit_delay();
}

void put_string(volatile unsigned char *port, unsigned char pin, PGM_P str) {
   // send character in txchar on port pin
   //    assumes line driver (inverts bits)
   static char chr;
   static int index;
   index = 0;
   do {
      chr = pgm_read_byte(&(str[index]));
      put_char(&serial_port, serial_pin_out, chr);
      ++index;
      } while (chr != 0);
}
//LED flashes with a long delay
// Note: _delay_ms requires compile time constant otherwise compliation failes in avr/include/util/delay.h
void flash_long() {
   clear(led_port, led_pin);
   _delay_ms(500);
   set(led_port, led_pin);
   _delay_ms(500);
}
//LED flashes with a short delay
// Note: _delay_ms requires compile time constant otherwise compliation failes in that avr/include/util/delay.h
void flash_short() {
   clear(led_port, led_pin);
   _delay_ms(100);
   set(led_port, led_pin);
   _delay_ms(100);
}

void print_version(){
  // write a message when we start
  output(serial_direction, serial_pin_out);
  static const char message[] PROGMEM = "hello from version 6";
  put_string(&serial_port, serial_pin_out, (PGM_P) message);
  put_char(&serial_port, serial_pin_out, 10); // new line

}

int main() {
   // main
   static char chr;
   //
   // set clock divider to /1
   //
   CLKPR = (1 << CLKPCE);
   CLKPR = (0 << CLKPS3) | (0 << CLKPS2) | (0 << CLKPS1) | (0 << CLKPS0);
   //
   // initialize output pins
   //
   set(serial_port, serial_pin_out);
   input(serial_direction, serial_pin_out);
   set(led_port, led_pin);
   output(led_direction, led_pin);

   print_version();

   //
   // main loop
   // For ever
   while (1) {
      // note: the get_char() function call blocks until it can read on serial_pin_in.
      get_char(&serial_pins, serial_pin_in, &chr);
      flash_long();
      if (chr == node_id) {
         //if this message is for us reply back with a message and flash led
         //Todo: Create a function for print name
         output(serial_direction, serial_pin_out);
         static const char message[] PROGMEM = "i am node ";
         put_string(&serial_port, serial_pin_out, (PGM_P) message);
         put_char(&serial_port, serial_pin_out, chr);
         put_char(&serial_port, serial_pin_out, 10); // new line
         //Todo: we might want to flash manytimes here by adding a loop
         flash_short();
         flash_short();
       }
   }
}
