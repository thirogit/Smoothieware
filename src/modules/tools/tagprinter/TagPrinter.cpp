#include "TagPrinter.h"
#include "Module.h"
#include "Kernel.h"
#include "nuts_bolts.h"
#include "Config.h"
#include "StreamOutputPool.h"
#include "SerialMessage.h"
#include "checksumm.h"
#include "ConfigValue.h"
#include "StepTicker.h"
#include "Block.h"
#include "SlowTicker.h"
#include "Robot.h"
#include "utils.h"
#include "Pin.h"
#include "Gcode.h"
#include "PwmOut.h" // mbed.h lib
#include "PublicDataRequest.h"
#include "modules/robot/Conveyor.h"
#include "wait_api.h"

#include <algorithm>

#define tagprinter_checksum                          CHECKSUM("tagprinter")
#define tagprinter_module_pin_checksum               CHECKSUM("tagprinter_module_pin")
#define tagprinter_module_ttl_pin_checksum           CHECKSUM("tagprinter_module_ttl_pin")
#define tagprinter_module_resolution_checksum        CHECKSUM("tagprinter_module_resolution")
#define tagprinter_module_rate_mm_s_checksum         CHECKSUM("tagprinter_module_rate_mm_s")
#define tagprinter_module_dot_burn_us_checksum       CHECKSUM("tagprinter_module_dot_burn_us")


#define MAX_NCOLS 75
#define MAX_NROWS 300

uint8_t _print_buffer[MAX_NROWS][MAX_NCOLS];

TagPrinter::TagPrinter()
{
    laser_on = false;
    scale = 1;
    manual_fire = false;
    fire_duration = 0;
}

void TagPrinter::on_module_loaded()
{
    /*if( !THEKERNEL->config->value( laser_module_enable_checksum )->by_default(false)->as_bool() ) {
        // as not needed free up resource
        delete this;
        return;
    }*/

    // Get smoothie-style pin from config
    /*Pin* dummy_pin = new Pin();
    dummy_pin->from_string(THEKERNEL->config->value(laser_module_pin_checksum)->by_default("nc")->as_string())->as_output();

    // Alternative less ambiguous name for pwm_pin
    if (!dummy_pin->connected())
        dummy_pin->from_string(THEKERNEL->config->value(laser_module_pwm_pin_checksum)->by_default("nc")->as_string())->as_output();

    pwm_pin = dummy_pin->hardware_pwm();

    if (pwm_pin == NULL) {
        THEKERNEL->streams->printf("Error: Laser cannot use P%d.%d (P2.0 - P2.5, P1.18, P1.20, P1.21, P1.23, P1.24, P1.26, P3.25, P3.26 only). Laser module disabled.\n", dummy_pin->port_number, dummy_pin->pin);
        delete dummy_pin;
        delete this;
        return;
    }


    this->pwm_inverting = dummy_pin->is_inverting();

    delete dummy_pin;
    dummy_pin = NULL;
*/
    // TTL settings
    this->ttl_pin = new Pin();
    ttl_pin->from_string( THEKERNEL->config->value(tagprinter_module_ttl_pin_checksum)->by_default("nc" )->as_string())->as_output();
    this->ttl_used = ttl_pin->connected();
    this->ttl_inverting = ttl_pin->is_inverting();
    if (ttl_used) {
        ttl_pin->set(0);
    } else {
        delete ttl_pin;
        ttl_pin = NULL;
    }


    uint32_t period = THEKERNEL->config->value(tagprinter_module_pwm_period_checksum)->by_default(20)->as_number();
   /* this->pwm_pin->period_us(period);
    this->pwm_pin->write(this->pwm_inverting ? 1 : 0);
    this->laser_maximum_power = THEKERNEL->config->value(laser_module_maximum_power_checksum)->by_default(1.0f)->as_number() ;

    // These config variables are deprecated, they have been replaced with laser_module_maximum_power and laser_module_minimum_power
    this->laser_minimum_power = THEKERNEL->config->value(laser_module_tickle_power_checksum)->by_default(0)->as_number() ;

    // Load in our preferred config variables
    this->laser_minimum_power = THEKERNEL->config->value(laser_module_minimum_power_checksum)->by_default(this->laser_minimum_power)->as_number() ;

    // S value that represents maximum (default 1)
    this->laser_maximum_s_value = THEKERNEL->config->value(laser_module_maximum_s_value_checksum)->by_default(1.0f)->as_number() ;
*/
    set_laser_power(0);

    //register for events
    this->register_for_event(ON_HALT);
    this->register_for_event(ON_GCODE_RECEIVED);
    this->register_for_event(ON_CONSOLE_LINE_RECEIVED);
    this->register_for_event(ON_GET_PUBLIC_DATA);

    // no point in updating the power more than the PWM frequency, but not faster than 1KHz
    ms_per_tick = 1000 / std::min(1000UL, 1000000 / period);
    //THEKERNEL->slow_ticker->attach(std::min(1000UL, 1000000 / period), this, &TagPrinter::set_proportional_power);
}

void TagPrinter::on_console_line_received( void *argument )
{
    SerialMessage *msgp = static_cast<SerialMessage *>(argument);
	//msgp->stream->printf("laser module console line received");
	
    if(THEKERNEL->is_halted()) return; // if in halted state ignore any commands

    string possible_command = msgp->message;

    // ignore anything that is not lowercase or a letter
    if(possible_command.empty() || !islower(possible_command[0]) || !isalpha(possible_command[0])) {
        return;
    }
	
    string cmd = shift_parameter(possible_command);

    // Act depending on command
    if (cmd == "startimgload") {
        string xstr = shift_parameter(possible_command);
		string ystr = shift_parameter(possible_command);
        if(power.empty()) {
            msgp->stream->printf("Usage: fire power%% [durationms]|off|status\n");
            return;
        }  
            
    } else if (cmd == "stopimgload") {
		
		
		
	} else if (cmd == "loadimgline") {
		
		
		
	} else if (cmd == "burnimg") {
		
		
		
	} else if (cmd == "abortburn") {
		
		
		
	} else if (cmd == "burnprogress") {
		
		
		
	}       
	
	
}


/*
THEROBOT->reset_axis_position(0.0,0.0,0.0);
			
			int NROWS = MAX_NROWS;
			int NCOLS = MAX_NCOLS*8;
			
			for (int row = 0; row < NROWS; row++)
			{
				
				new_message.stream->printf("printing row forward - %d\n", row);
				
				int col = 0;

				//while(col < NCOLS && img2print[col] != 0) col += 8;
				while (col < NCOLS && !is_on(row, col)) col++;
				if (col >= NCOLS) //empty line
					continue;

				do
				{
					while (col < NCOLS && !is_on(row, col)) col++;

					if (col >= NCOLS) continue;

					move_to(new_message.stream,col, row);
					burn_dot(new_message.stream);
					col++;
					while (col < NCOLS && is_on(row, col))
					{	
						move_to(new_message.stream,col, row);
						//delta_move(new_message.stream,1, 0);						
						burn_dot(new_message.stream);
						col++;
					}
				} while (col < NCOLS);

				print_pos(new_message.stream);
				
				row++;
				if (row < NROWS)
				{
					new_message.stream->printf("printing row backwards - %d\n", row);
					//print_pos(new_message.stream);
					col = NCOLS - 1;

					while (col >= 0 && !is_on(row, col)) col--;
					if (col < 0) //empty line
						continue;

					do
					{
						while (col >= 0 && !is_on(row, col)) col--;
						move_to(new_message.stream,col, row);				
						burn_dot(new_message.stream);	
						col--;
						
						while (col >= 0 && is_on(row, col))
						{
							//delta_move(new_message.stream,-1, 0);
							move_to(new_message.stream,col, row);				
							burn_dot(new_message.stream);									
							col--;
						}
					} while (col > 0);
					
					print_pos(new_message.stream);

				}
*/

// returns instance
void TagPrinter::on_get_public_data(void* argument)
{
    PublicDataRequest* pdr = static_cast<PublicDataRequest*>(argument);

    if(!pdr->starts_with(tagprinter_checksum)) return;
    pdr->set_data_ptr(this);
    pdr->set_taken();
}


void TagPrinter::on_gcode_received(void *argument)
{
}

void TagPrinter::on_halt(void *argument)
{
    if(argument == nullptr) {
    }
}

void TagPrinter::move_to(StreamOutput* stream,float x,float y)
{
	float position[3] {0.0, 0.0, 0.0}; 
			
	position[0] = RESOLUTION*x;
	position[1] = RESOLUTION*y;
	THEROBOT->append_milestone(position, 5000.0);
	THECONVEYOR->wait_for_idle();

}

void TagPrinter::burn_dot(StreamOutput* stream)
{
	this->ttl_pin->set(true);
    wait_ms(8);
	this->ttl_pin->set(false);
}

bool TagPrinter::is_on(int row,int col)
{
	uint8_t const * const imgrow = img2print[row];
	int byte_col = col/8;
	int bit_col = col % 8;
	
	return (imgrow[byte_col] & 1 << bit_col) != 0;
	
}
