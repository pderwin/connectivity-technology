#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/trace.h>

#define GET_PORT_PIN(__val) \
   device = __val < 32 ? gpio0_dev : gpio1_dev; \
   pin    = __val % 32

static const struct device *gpio0_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
static const struct device *gpio1_dev = DEVICE_DT_GET(DT_NODELABEL(gpio1));

#if 0
static void port_pin(char *name, uint32_t address)
{
   uint32_t
      pin,
      port,
      val = sys_read32(address);

   port = (val >> 5) & 1;
   pin  = val & 0x1f;

   printk("   %-8.8s: ", name);

   if (val & (1 <<31)) {
      printk("    N/C    |");
   }
   else {
      printk("%d %2d  (%2d) |", port, pin, val & 0xff);
   }

   printk("     (%x)\n", address);
}

static void port_pin_set(uint32_t address, uint32_t val)
{
   sys_write32(val, address);
}

static void check_i2s(uint32_t regs)
{
   printk("I2S\n");
   port_pin("MCK",   regs + 0x560);
   port_pin("SCK",   regs + 0x564);
   port_pin("LRCK",  regs + 0x568);
   port_pin("SDIN",  regs + 0x56c);
   port_pin("SDOUT", regs + 0x570);
}

static void check_pdm(uint32_t regs)
{
   printk("PDM\n");
   port_pin("CLK",   regs + 0x540);
   port_pin("DIN",   regs + 0x544);
}

static void check_pwm(uint32_t regs)
{
   printk("PDM\n");
   port_pin("OUT[0]",   regs + 0x560);
   port_pin("OUT[1]",   regs + 0x564);
   port_pin("OUT[2]",   regs + 0x568);
   port_pin("OUT[3]",   regs + 0x56c);
}

static void check_quad_decoder(uint32_t regs)
{
   printk("Quad Decoder\n");
   port_pin("LED",  regs + 0x51c);
   port_pin("A",    regs + 0x520);
   port_pin("B",    regs + 0x524);
}

static void check_qspi(uint32_t regs)
{
   printk("QSPI\n");
   port_pin("SCK",  regs + 0x524);
   port_pin("CSN",  regs + 0x528);
   port_pin("IO0",  regs + 0x530);
   port_pin("IO1",  regs + 0x534);
   port_pin("IO2",  regs + 0x538);
   port_pin("IO3",  regs + 0x53c);
}


void check_spi(uint32_t regs)
{
   printk("SPI\n");
   port_pin("SCK",  regs + 0x508);
   port_pin("MOSI", regs + 0x50c);
   port_pin("MISO", regs + 0x510);
   port_pin("CS",   regs + 0x514);
   port_pin("DCX",  regs + 0x56c);
}

void set_spi(uint32_t regs, uint32_t val)
{
   printk("SET SPI val: %d \n", val);
   port_pin_set(regs + 0x510, val);
}


static void check_twi(uint32_t regs)
{
   printk("TWI\n");
   port_pin("SCL", regs + 0x508);
   port_pin("SDA", regs + 0x50c);
}

static void check_uart(uint32_t regs)
{
   printk("UART\n");
   port_pin("RTS", regs + 0x508);
   port_pin("TXD", regs + 0x50c);
   port_pin("CTS", regs + 0x510);
   port_pin("RXD", regs + 0x514);
}

static void check_user(uint32_t regs)
{
   uint32_t
      nfc_pins;

   nfc_pins = sys_read32(regs + 0x20c);

   printk("USER\n");
   printk("   NFC pins: %s\n", nfc_pins & 1 ? "Antenna":"GPIO");
}


/*-------------------------------------------------------------------------
 *
 * name:        gpio_mapper
 *
 * description: try to figure out what GPIOs are connected to
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
int gpio_mapper (const struct device *device)
{
   (void) device;

   uint32_t
      i,
      j,
//      new,
      pin;

   printk("%s %d (from %p) \n", __func__, __LINE__, __builtin_return_address(0) );

   /*
    * Show pins when assigned to functional blocks.
    */
   check_i2s(0x40025000);
   check_pdm(0x4001d000);

   check_pwm(0x4001C000); //  PWM PWM0 Pulse width modulation unit 0
   check_pwm(0x40021000); //  PWM PWM1 Pulse width modulation unit 1
   check_pwm(0x40022000); //  PWM PWM2 Pulse width modulation unit 2
   check_pwm(0x4002D000); //  PWM PWM3 Pulse width modulation unit 3

   check_quad_decoder(0x40012000); //  QDEC QDEC Quadrature decoder

   check_qspi(0x40029000); //  QSPI QSPI External memory interface

   check_spi(0x40003000); // SPI SPI0 SPI master 0
   check_spi(0x40004000); // SPI SPI1 SPI master 1
   check_spi(0x40023000); //  SPI SPI2 SPI master 2
   check_spi(0x4002F000); // SPIM 3

   check_twi(0x40003000); // TWI TWI0 Two-wire interface master 0
   check_twi(0x40004000); // TWI TWI1 Two-wire interface master 1

   check_uart(0x40002000); //  UART UART0 Universal asynchronous receiver/transmitter
   check_uart(0x40028000); //  UARTE UARTE1 Universal asynchronous receiver/transmitter with EasyDMA,

   check_user(0x10001000); //  User information


   /*
    * Twiddle the pins themselves.
    */
   gpio0_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
   gpio1_dev = DEVICE_DT_GET(DT_NODELABEL(gpio1));

   /*
    * Make all pins input.
    */
   for (i=0; i<32; i++) {
      gpio_pin_configure(gpio0_dev, i, GPIO_INPUT);
   }
   for (i=0; i<16; i++) {
      gpio_pin_configure(gpio1_dev, i, GPIO_INPUT);
   }

#define GPIO_NUM (15)
#define DEVICE   gpio1_dev

#define RESET_GPIO (2)

   /*
    * Make reset pin output and drive high (inactive).
    */
   GET_PORT_PIN( RESET_GPIO );
   gpio_pin_configure(device, pin, GPIO_OUTPUT_ACTIVE);
   gpio_pin_set(device, pin, 1);

   /*
    * read all pins.
    */
   for (i=0; i<48; i++) {
      GET_PORT_PIN(i);

      before[i] = gpio_pin_get(device, pin);

   }

   /*
    * Take the reset pin low
    */
   GET_PORT_PIN( RESET_GPIO );
   gpio_pin_set(device, pin, 0);

   /*
    * Verify all pins.
    */
   for (i=0; i<48; i++) {
      if (i != RESET_GPIO) {
         GET_PORT_PIN(i);

         new = gpio_pin_get(device, pin);

         if (new != before[i]) {
            printk("GPIO %d changed state\n", i);
         }

      }
   }



uint32_t
    before[48];

void gpio_mapper_read_all(void)
{
   uint32_t
      i,
      pin;
   const struct device
      *device;

   printk("%s %d READING ALL PINS\n", __func__, __LINE__);

   /*
    * read all pins.
    */
   for (i=0; i<48; i++) {
      GET_PORT_PIN(i);

      before[i] = gpio_pin_get(device, pin);
   }
}

void gpio_mapper_compare_all(void)
{
   uint32_t
      i,
      new,
      pin;
   const struct device
      *device;

/*
    * Verify all pins.
    */
   for (i=0; i<48; i++) {
      GET_PORT_PIN(i);

      new = gpio_pin_get(device, pin);

      if (new != before[i]) {
         printk("GPIO %d changed state\n", i);
      }

   }
}
#endif

#define DELAY_MS (10)

int gpio_mapper (const struct device *device)
{
   (void) device;

   uint32_t
      i,
      j,
      pin;

   printk("TWIDDLE OUTPUTS\n");

   for (i=0; i<48; i++) {

      GET_PORT_PIN(i);

      printk("%s %d twiddle port: %p  pin: %d \n", __func__, __LINE__, device, pin);

      gpio_pin_configure(device, pin, GPIO_OUTPUT_ACTIVE);

      for (j=0; j<10; j++) {
         k_sleep(K_MSEC(DELAY_MS));

         gpio_pin_set(device, pin, 0);

         k_sleep(K_MSEC(DELAY_MS));

         gpio_pin_set(device, pin, 1);
      } // 10 iterations

      gpio_pin_configure(device, pin, GPIO_INPUT);

   } // each GPIO


   return 0;
}

// SYS_INIT(gpio_mapper, APPLICATION,  CONFIG_GPIO_INIT_PRIORITY );
