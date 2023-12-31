#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>
#include <tuple>



using namespace std;

#define MAX_LINE_LENGTH 256


vector<string> split_string(const string& str, char delimiter) {
    vector<string> tokens;
    string token; 
    istringstream tokenStream(str);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


class TB6612FNG_MD {
    public:
        TB6612FNG_MD(uint PWMA_pin=0, uint AI2_pin=1, uint AI1_pin=2, uint STBY_pin=6, uint BI1_pin =5, uint BI2_pin=4, uint PWMB_pin=3) 
        : pwma_pin(PWMA_pin), ai2_pin(AI2_pin), ai1_pin(AI1_pin), stby_pin(STBY_pin), bi1_pin(BI1_pin), bi2_pin(BI2_pin), pwmb_pin(PWMB_pin){

            // initialize pins
            gpio_init(ai1_pin);
            gpio_init(ai2_pin);
            gpio_init(bi1_pin);
            gpio_init(bi2_pin);
            gpio_init(stby_pin);

            gpio_set_dir(ai1_pin, GPIO_OUT);
            gpio_set_dir(ai2_pin, GPIO_OUT);
            gpio_set_dir(bi1_pin, GPIO_OUT);
            gpio_set_dir(bi2_pin, GPIO_OUT);
            gpio_set_dir(stby_pin, GPIO_OUT);
            
            // gpio_put(stby_pin, true);

            set_standby_mode(false);

            // Setup PWM
            gpio_set_function(pwma_pin, GPIO_FUNC_PWM);
            gpio_set_function(pwmb_pin, GPIO_FUNC_PWM);
            
            pwm_set_wrap( pwm_gpio_to_slice_num(pwma_pin), top);
            pwm_set_wrap( pwm_gpio_to_slice_num(pwmb_pin), top);

            pwm_set_enabled(pwm_gpio_to_slice_num(pwma_pin), true);
            pwm_set_enabled(pwm_gpio_to_slice_num(pwmb_pin), true);
        }

        //set speed [0-100%]
        void set_speed( uint percentage, bool motor=true) {
            // motor==true Motor A, motor==false for Motor B
            uint &pwm_pin = (motor==true) ? pwma_pin : pwmb_pin; 

            pwm_set_chan_level(pwm_gpio_to_slice_num(pwm_pin), 
            pwm_gpio_to_channel(pwm_pin),
            percentage*top/100);
        }

        void set_direction(string operation, bool motor=true){

            uint &dir1_pin = (motor==true) ? ai1_pin : bi1_pin; 
            uint &dir2_pin = (motor==true) ? ai2_pin : bi2_pin;

            if (operation=="ccw"){
                gpio_put(dir1_pin, false);
                gpio_put(dir2_pin, true);
            } else if (operation=="cw"){
                gpio_put(dir1_pin, true);
                gpio_put(dir2_pin, false);
            } else if (operation=="short_brake"){
                gpio_put(dir1_pin, true);
                gpio_put(dir2_pin, true);
            } else if (operation=="stop"){
                gpio_put(dir1_pin, false);
                gpio_put(dir2_pin, false);
            } else {}
        }

        void set_standby_mode(bool active=false){
            stby_state = active;
            gpio_put(stby_pin, !stby_state);
        }

        bool get_standby_state(){
            return stby_state;
        }

    private:
        uint pwma_pin, ai2_pin, ai1_pin, stby_pin, bi1_pin, bi2_pin, pwmb_pin;
        uint top=255; // 8-bit resolution
        bool stby_state;

};


struct motors{
    TB6612FNG_MD motor_driver;
    string forward;
    string backward;
    bool front;
    bool back;
    };

bool DEFAULT_MOTOR_CONFIG[4] = {true,false,false,true};

// in the normal congifuration  Left motor front is a0[true] (b0[false] is back), right motor front is b0[false] (a0[true] is back)
class Rover{
    public:
        Rover(TB6612FNG_MD left_MD, TB6612FNG_MD right_MD, string name="Rover1", bool motor_config[4] = DEFAULT_MOTOR_CONFIG) : 
        name(name){

                
                left_motors.motor_driver = left_MD;
                left_motors.front = motor_config[0];
                left_motors.back = motor_config[1];
                left_motors.forward = "ccw";
                left_motors.backward = "cw";
                                
                right_motors.motor_driver = right_MD;
                right_motors.front = motor_config[2];
                right_motors.back = motor_config[3];
                right_motors.forward = "cw";
                right_motors.backward = "ccw";

            set_motion_direction("forward");
        
        }

        void set_motion_direction(string direction){
            
            if (direction =="forward"){

                left_motors.motor_driver.set_direction(left_motors.forward, left_motors.front);
                left_motors.motor_driver.set_direction(left_motors.forward, left_motors.back);
                right_motors.motor_driver.set_direction(right_motors.forward, right_motors.front);
                right_motors.motor_driver.set_direction(right_motors.forward, right_motors.back);
             

            } else if(direction == "backward"){

                left_motors.motor_driver.set_direction(left_motors.backward, left_motors.front);
                left_motors.motor_driver.set_direction(left_motors.backward, left_motors.back);
                right_motors.motor_driver.set_direction(right_motors.backward, right_motors.front);
                right_motors.motor_driver.set_direction(right_motors.backward, right_motors.back);
            

            } else if(direction == "left"){

                left_motors.motor_driver.set_direction(left_motors.backward, left_motors.front);
                left_motors.motor_driver.set_direction(left_motors.forward, left_motors.back);
                right_motors.motor_driver.set_direction(right_motors.forward, right_motors.front);
                right_motors.motor_driver.set_direction(right_motors.backward, right_motors.back);

            } else if(direction == "right"){
                
                left_motors.motor_driver.set_direction(left_motors.forward, left_motors.front);
                left_motors.motor_driver.set_direction(left_motors.backward, left_motors.back);
                right_motors.motor_driver.set_direction(right_motors.backward, right_motors.front);
                right_motors.motor_driver.set_direction(right_motors.forward, right_motors.back);

            } else if(direction == "rotate_right"){
                
                left_motors.motor_driver.set_direction(left_motors.forward, left_motors.front);
                left_motors.motor_driver.set_direction(left_motors.forward, left_motors.back);
                right_motors.motor_driver.set_direction(right_motors.backward, right_motors.front);
                right_motors.motor_driver.set_direction(right_motors.backward, right_motors.back);

            } else if(direction == "rotate_left"){
                
                left_motors.motor_driver.set_direction(left_motors.backward, left_motors.front);
                left_motors.motor_driver.set_direction(left_motors.backward, left_motors.back);
                right_motors.motor_driver.set_direction(right_motors.forward, right_motors.front);
                right_motors.motor_driver.set_direction(right_motors.forward, right_motors.back);

            } else if(direction == "stop"){
                
                left_motors.motor_driver.set_direction("stop", left_motors.front);
                left_motors.motor_driver.set_direction("stop", left_motors.back);
                right_motors.motor_driver.set_direction("stop", right_motors.front);
                right_motors.motor_driver.set_direction("stop", right_motors.back);

            }

        }
            
        void set_speed_all(uint speed_percentage = 50){

            left_motors.motor_driver.set_speed(speed_percentage, left_motors.front);
            left_motors.motor_driver.set_speed(speed_percentage, left_motors.back);
            right_motors.motor_driver.set_speed(speed_percentage, right_motors.front);
            right_motors.motor_driver.set_speed(speed_percentage, right_motors.back);

        }

        // format of string -->"direction:string;speed:int"
        tuple<string,int> parse_string_command(string command_string){
            
            vector<string> control_commands = split_string(command_string, ';');
            string direction = split_string(control_commands[0], ':')[1];
            uint speed = stoi(split_string(control_commands[1], ':')[1]);
            return make_tuple(direction, speed);
        }        

        // }

        // string ping_status(){

        // }
    
    private:
    motors left_motors, right_motors;
    string name, status;

};

int main(){
    /* Setup motors
    Motors: 
    Front Left(motors_left==true) 
    Back Left(motors_left==false) 
    Front Right(motores_right==true) 
    Back Right(motores_right==false)
    */
   stdio_init_all();
   char line[MAX_LINE_LENGTH];
   TB6612FNG_MD right_md(0,1,2,6,5,4,3);
   TB6612FNG_MD left_md(20,21,22,19,26,27,28);

   Rover robot(left_md, right_md, "test_robot");

   //main loop 
   while (true){
    
    if (fgets(line, MAX_LINE_LENGTH, stdin)) {
        
        printf("Received: %s", line);
        string line_str(line);

        auto [direction,speed]= robot.parse_string_command(line_str);

        printf("direction set to:  %s, speed set to: %u\n", direction.c_str(), speed);

        robot.set_motion_direction(direction);
        robot.set_speed_all(speed);
        }
    
    }
};