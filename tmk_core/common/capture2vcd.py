#!/usr/bin/python3
# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
#
# This converts pin capture data into VCD format
#
# Capture record format: <time> ':' <stat>
#     time   6 hex chars; timestamp of event, which counts 1/16 us per tick
#           and is comprised of three bytes:
#               timer xtra(overflow count)
#               timer high
#               timer low
#     stat   2 hex chars;  Pin state
#

import sys
import re

# https://github.com/westerndigitalcorporation/pyvcd
from vcd import VCDWriter

if len(sys.argv) > 2:
    print(f"Usage: {sys.argv[0]} [infile]")
    sys.exit(1)

if len(sys.argv) == 2:
    file = open(sys.argv[1])
else:
    file = sys.stdin

# write VCD
from datetime import datetime
with VCDWriter(sys.stdout, timescale='1 us', date=datetime.utcnow().ctime()) as writer:

    # prams: scope, name, var_type, size
    port = writer.register_var('tmk_capture', 'port', 'wire', size=8, init=0xFF)

    # Pin change:
    #     TTTP
    # Timer overflow:
    #     CC0P
    # P:Pin
    # T:Timer(us)
    # C:Overflow count
    p = re.compile(r'([0-9A-Fa-f]{4})')

    ext_time = 0
    prv_time = 0
    prv_stat = 0
    for line in file:
        for record in line.strip().split():
            m = p.match(record)
            if m is not None:
                time = int(record[0:3], 16)
                stat = int(record[3:4], 16)

                # time overflow
                if prv_stat == stat:
                    if time == 0:
                        # rollover
                        ext_time += 0x100
                    else:
                        ext_time += time >> 4
                    prv_time = 0    # no time rollover
                    #print('time overflow: ', time >> 4, hex(time), stat, file=sys.stderr);
                    continue

                # time flipped - this indicates error
                if prv_time >= time:
                    ext_time += 1
                    print('time flipped: ', ext_time, ': ', hex(prv_time), '>', hex(time), file=sys.stderr);

                prv_time = time
                prv_stat = stat

                # time: 1us per tick
                #print(ext_time, time, hex(time), stat, file=sys.stderr)
                writer.change(port, ((ext_time * 0x1000) + time), stat)
            else:
                print('Invalid record: ', record, file=sys.stderr);
                #sys.exit(1)
