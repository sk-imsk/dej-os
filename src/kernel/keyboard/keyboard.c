// keyboard.c
// ps2 keyboard driverrr
// For ps2 keyboard support
// built on assumptions
//
#include "../x86.h"
#include "stdbool.h"
#include "stddef.h"
#include <stdint.h>
#include "keyboard.h"

#define waittosend while (x86_inb(0x64) & 0x02) \
    __asm__ volatile ("nop");
#define waitfordata while (!(x86_inb(0x64) & 0x01)) \
    __asm__ volatile ("nop");


//0x60 = keyboard/controller DATA port
//0x64 = keyboard controller STATUS/COMMAND port



typedef enum{
    SCANCODE_SET_1 = 0x43,
    SCANCODE_SET_2 = 0x41,
    SCANCODE_SET_3 = 0x3F
}scancode_set;




static scancode_set scodeset;
static bool inited = false;
uint8_t keyd = 0;


// -1 = something super weird is up
// 0 = all good
// 1 = keyboard failed its own test
int keyboard_init(void){
    int attempts = 0;
    uint8_t response = 0;
    // send 0xFF for keyboard turn on
    waittosend

    x86_outb(0x60, 0xFF);

    waitfordata

    response = x86_inb(0x60);


    // response can = 0xfe (send again lil bro) 0xfa (all good)
    if (response == 0xFE){
        // resend cuz keyboard is a lil retarded
        attempts = 0;
        while (attempts < 3){
            response = 0;
            waittosend

            x86_outb(0x60, 0xFF);

            waitfordata

            response = x86_inb(0x60);

            if (response != 0xFE) break;
            attempts++;

        }
    }

    if (response != 0xFA){
        // something weird is up so lowk make it not work
        return -1;
    }


    waitfordata


    response = x86_inb(0x60);

    if (response != 0xAA){ // it didnt succeed
        return 1;
    }


    waittosend

    x86_outb(0x60, 0xF0);

    waitfordata

    response = x86_inb(0x60);


    if (response == 0xFE){
        attempts = 0;
        while (attempts < 3){
            attempts++;
            waittosend
            x86_outb(0x60, 0xF0);

            waitfordata
            response = x86_inb(0x60);

            if (response != 0xFE) break;
        }
    }
    if (response != 0xFA){
        return -1;          //should have a ack rn or else the keyboard is lowk broken
    }


    waittosend
    x86_outb(0x60, 0x00);

    waitfordata
    x86_inb(0x60);

    if (response == 0xFE){
        attempts = 0;
        while (attempts < 3){
            attempts++;
            waittosend
            x86_outb(0x60, 0x00);

            waitfordata
            response = x86_inb(0x60);

            if (response != 0xFE) break;
        }
    }


    waitfordata
    response = x86_inb(0x60);
    scodeset = x86_inb(0x60);
    if (!(scodeset == 0x43 ||scodeset == 0x41 ||scodeset == 0x3F)) return -1;


    inited = true;
    return 0;
}


// triggers when a key is released
//
// -1 = initilise the keyboard
key_t keyboard_poll_k(){
    if (!inited) return -1;
    uint8_t response;

start:

    waitfordata
    response = x86_inb(0x60);


    if (response == 0xE1){ // we gotta lowk trash the pause key cuz it kinda forgot to exist
        waitfordata // check if its press or release
        for (int i = 0; i < 7; i++){
            waitfordata
            x86_inb(0x60); // discard 7 bytes

        }
        goto start; // user doesnt care about pause key

    }

    if (response == 0xF0){
        if (keyd > 0) keyd--;
        waitfordata
        response = x86_inb(0x60);
        return set2_table[response];



    }

    if (response == 0xE0){
        waitfordata
        response = x86_inb(0x60);
        if (response == 0xF0){
            waitfordata
            response = x86_inb(0x60);
            return set2_e0_table[response];
        } else {
            keyd++;
            goto start;
        }


    }






    return -1;
}
