#include <Arduino.h>
#include <door_functions.hpp>

enum StateMachine_states{
    STATE_INIT,
    STATE_GET_CARD_ONLINE,
    STATE_GET_CARD_OFFLINE,
    STATE_CHECK_CONNECTION,
    STATE_TRY_TO_CONNECT
};

int StateMachine_state;

void setup() {
    StateMachine_state = STATE_INIT;
}

void loop() {
    delay(50);

    switch(StateMachine_state){
        case STATE_INIT:
            if(setup_connections()){
                StateMachine_state = STATE_GET_CARD_ONLINE;
            }else{
                StateMachine_state = STATE_GET_CARD_OFFLINE;
            }

            break;

        case STATE_GET_CARD_ONLINE:
            check_card();

            StateMachine_state = STATE_CHECK_CONNECTION;

            break;

        case STATE_GET_CARD_OFFLINE:
            if(check_card()){
                open_door();
            }

            StateMachine_state = STATE_TRY_TO_CONNECT;

            break; 
        
        case STATE_CHECK_CONNECTION:
            if(verify_connections()){
               StateMachine_state = STATE_GET_CARD_ONLINE; 
            }else{
               StateMachine_state = STATE_GET_CARD_OFFLINE; 
            }

            break;

        case STATE_TRY_TO_CONNECT:

            static int counter = 0;

            if(counter++ > 1350){        // Only try to connect once every five minutes -> 1350
                try_to_connect(10);
                counter = 0;
            }

            StateMachine_state = STATE_CHECK_CONNECTION;

            break;

    }
}