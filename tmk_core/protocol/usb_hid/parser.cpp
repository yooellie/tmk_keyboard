#include "parser.h"
#include "usb_hid.h"

#include "print.h"


void KBDReportParser::Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf)
{
    xprintf("input %d:", hid->GetAddress());
    for (uint8_t i = 0; i < len; i++) {
        xprintf(" %02X", buf[i]);
    }
    xprintf("\r\n");

    // Apple USB Magic keyboard (Model A1644)
    // USB Descriptor: // https://gist.github.com/tmk/0626b78f73575d5c1efa86470c4cdb18
    //
    //  01 00 00 00 00 00 00 00 00 00
    //
    // buf[0]     Report ID:1
    // buf[1]     Modifiers
    // buf[2]     Reserve
    // buf[3-8]   Keys
    // buf[9]     eject and fn
    //            0x01: eject key (top right next key to F12) pressed
    //            0x02: fn key (bottom left key) pressed
    //            0x03: fn and eject key pressed at the same time
    if (buf[0] == 0x01 && len == 10) {
        uint8_t t = buf[0];
        buf[0] = buf[1];
        buf[1] = buf[2];
        buf[2] = buf[3];
        buf[3] = buf[4];
        buf[4] = buf[5];
        buf[5] = buf[6];
        buf[6] = buf[7];
        buf[7] = buf[8];
        // adhoc: Map eject and fn to F23 and F24
        if (buf[9] & 0x01) buf[6] = KC_F23; // eject -> F23
        if (buf[9] & 0x02) buf[7] = KC_F24; // Fn -> F24
    }

    // Rollover error
    // Cherry: 0101010101010101
    // https://geekhack.org/index.php?topic=69169.msg2638223#msg2638223
    // Apple:  0000010101010101
    // https://geekhack.org/index.php?topic=69169.msg2760969#msg2760969
    if (buf[2] == 0x01) {
       xprintf("Rollover error: ignored\r\n");
       return;
    }

    ::memcpy(&report, buf, sizeof(report_keyboard_t));
    time_stamp = millis();
}
