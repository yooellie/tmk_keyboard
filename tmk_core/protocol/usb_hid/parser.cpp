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

    // Apple USB Magic keyboard (Model A1644) (buf[1] == 0x01) has mods byte in buf[1] while buf[0] is always 0x01:
    // input 1: 01 00 00 00 00 00 00 00 00 01
    // It has 8 key input bytes instead of the usual 6
    // buf[9] on magic keyboard:
    // 0x01: eject key (top right next key to F12) pressed
    // 0x02: fn key (bottom left key) pressed
    // 0x03: fn and eject key pressed at the same time
    // we store buf[9] in buf[1] (reserved byte)
    // and map eject and fn to F23 and F24
    if (buf[0] == 0x01 && len == 10) {
        uint8_t t = buf[0];
        buf[0] = buf[1];
        buf[1] = buf[9];
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
