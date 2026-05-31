#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

/* HZ */
#define F_CPU 16000000UL
#define TWI_SCL_HZ 100000UL

/* pin constants */
#define BUTTON_PIN PD4
#define INTERNAL_LED PB5
#define BUZZER_PIN PD5

/* constant vals */
#define DOT_TIME 100
#define DASH_TIME 300
#define SEND_TIME 2000
#define CLOSE_TIME 10000
#define TWBR_VAL ((uint8_t)((F_CPU / TWI_SCL_HZ - 16UL) / 2UL))

/* I2C address */
#define PCF_ADDR 0x27
#define LCD_BL 0x08

/* Binary Morse Tree */
static char letters[] = {
'_', 'E', 'T', 'I', 'A', 'N', 'M', 'S', 'U', 'R', 'W', 'D', 'K', 'G', 'O', 'H', 'V', 'F', '_', 'L', '_', 'P', 'J', 'B', 'X', 'C', 
'Y', 'Z', 'Q', '_', '_', '5', '4', '_', '3', '_', '_', '_', '2', '_', '_', '+', '_', '_', '_', '_', '1', '6', '=', '/', '_', '_', 
'_', '_', '_', '7', '_', '_', '_', '8', '_', '9', '0'
};

/* Global flags */
volatile uint8_t lcd_flag = 0;
volatile uint8_t clear_flag = 0;
volatile uint8_t buzzer_flag = 0;
volatile uint8_t message_start_flag = 0;

volatile char letter = 0;

/* TWI Protocol */
static void TWI_START()
{
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while (!(TWCR & (1<<TWINT)));
}
static void TWI_STOP()
{
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
    while (TWCR & (1<<TWSTO));
}

static void twi_write_byte(uint8_t b)
{
    TWDR = b;
    TWCR = (1<<TWINT)|(1<<TWEN);
    while (!(TWCR & (1<<TWINT)));
}

static void twi_init(void)
{
    TWBR = TWBR_VAL;
    TWSR = 0x00;
}
static void pcf_write(uint8_t data)
{
    TWI_START();
    twi_write_byte((uint8_t)(PCF_ADDR << 1 | 0));
    twi_write_byte(data);
    TWI_STOP();
}

// this was coppied from internet
static void lcd_init(void)
{
    twi_init();
    _delay_ms(50);

    /* send three 0x30 to reset */
    lcd_write_nibble(0x30, 0); _delay_ms(5);
    lcd_write_nibble(0x30, 0); _delay_us(150);
    lcd_write_nibble(0x30, 0); _delay_us(150);
    lcd_write_nibble(0x20, 0); _delay_us(150);
    lcd_command(0x28); /* Function Set — 4-bit, 2 lines, 5×8 */
    lcd_command(0x0C); /* Display ON, cursor OFF, blink OFF */
    lcd_command(0x06); /* Entry mode — cursor right, no shift */
    lcd_command(0x01); /* Clear display (needs ≥ 1.52 ms wait) */
    _delay_ms(2);
}

static void lcd_write_nibble(uint8_t nibble, uint8_t rs)
{
    uint8_t base = (nibble & 0xF0) | LCD_BL | (rs ? 0x01 : 0x00);
    pcf_write((uint8_t)(base | 0x04));
    _delay_us(1);
    pcf_write((uint8_t)(base & ~0x04));
    _delay_us(50);
}

static void lcd_send(uint8_t byte, uint8_t rs)
{
    lcd_write_nibble(byte & 0xF0, rs);
    lcd_write_nibble((uint8_t)(byte << 4), rs);
}

static void lcd_command(uint8_t cmd)
{
    lcd_send(cmd, 0);
    if (cmd <= 0x03)
    _delay_ms(2);
}

static void lcd_char(char c)
{
    lcd_send((uint8_t)c, 1);
}


static void init_timer()
{
    /* init hardware timer to 1 ms interrupts */
    TCCR0A |= (1 << WGM01);
    TCCR0B |= (1 << CS01) | (1 << CS00);
    OCR0A = 249;
    TIMSK0 |= (1 << OCIE0A);
}

ISR(TIMER0_COMPA_vect)
{
    static uint32_t push_ms = 0;
    static uint32_t close_ms = 0;
    static uint32_t send_ms = 0;
    static uint8_t prev_button_state = 1;
    static uint8_t i = 0;

    /* read button */
    volatile uint8_t button_state = (PIND & (1 << BUTTON_PIN)) ? 1 : 0;

    /* if pressed */
    if(button_state == 0)
    {
        /* alert user of a incoming message */
        if(message_start_flag == 0)
        {
            buzzer_flag = 1;
            message_start_flag = 1;
        }
        push_ms++;
        close_ms = 0;
    }

    /* if stopped pressing traverse binary according to pressed time*/
    else if(button_state == 1 && prev_button_state == 0)
    {
        send_ms = 0;
        close_ms = 0;

        if(push_ms >= DASH_TIME)
        {
            i = 2 * i+2;
            if (letters[i] != '_' && i <= 63)
            letter = letters[i];
            else
            i = 0;
        }

        else if(push_ms >= DOT_TIME)
        {
            i = 2 * i+1;
            if (letters[i] != '_' && i <= 63)
            letter = letters[i];
            else
            i = 0;
        }
        push_ms = 0;
    }

    /* if not pressed */
    else if(button_state == 1 && prev_button_state == 1 && !lcd_flag)
    {
        send_ms++;
        close_ms++;

        /* look if we have a letter to send*/
        if(send_ms >= SEND_TIME && letter != 0)
        {
            lcd_flag = 1;
            send_ms = 0;
            i = 0;
        }

        /* clear the screen*/
        if(close_ms >= CLOSE_TIME && letter == 0)
        {
            clear_flag = 1;
            message_start_flag = 0;
            close_ms = 0;
        }
        if(lcd_flag && letter == 0)
            lcd_flag = 0;
    }
    prev_button_state = button_state;
}

int main(void)
{
    /* init ports */
    DDRD &= ~(1 << BUTTON_PIN);
    PORTD |= (1 << BUTTON_PIN);
    DDRD &= ~(1 << BUZZER_PIN);
    PORTD |= (1 << BUZZER_PIN);
    DDRB |= (1 << INTERNAL_LED);

    init_timer();
    lcd_init();
    
    sei();
    
    while(1)
    {
        if(buzzer_flag)
        {
            /* buzz the buzeer for 60ms */
            PORTB |= (1<<BUZZER_PIN);
            _delay_ms(60);
            PORTB &= ~(1<<BUZZER_PIN);
        }

        /* turn on the internal led for 30ms and send the letter*/
        if (lcd_flag)
        {
            PORTB |= (1<<INTERNAL_LED);
            _delay_ms(30);
            PORTB &= ~(1<<INTERNAL_LED);

            lcd_char(letter);
            
            letter = 0;
            lcd_flag = 0;
        }

        /* clear the screen*/
        if(clear_flag)
        {
            lcd_command(0x01);
            clear_flag = 0;
        }
    }
}
