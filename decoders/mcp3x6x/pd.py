import sigrokdecode as srd

FAST_COMMANDS = {
    0x68: ('CONVERSION', 'Start conversion'),
    0x6C: ('STANDBY', 'Standby mode'),
    0x70: ('SHUTDOWN', 'Shutdown'),
    0x74: ('FULL_SHUTDOWN', 'Full shutdown'),
    0x78: ('RESET', 'Reset'),
}


class Decoder(srd.Decoder):
    api_version = 3
    id = 'mcp3x6x'
    name = 'MCP3x6x'
    longname = 'Microchip MCP3x6x'
    desc = 'Decode Microchip MCP3x6x SPI fast commands.'
    license = 'gplv2+'
    inputs = ['spi']
    outputs = []
    tags = ['IC', 'ADC']
    annotations = (
        ('fast-cmd', 'Fast command'),
        ('warning', 'Warning'),
    )
    annotation_rows = (
        ('cmd', 'Commands', (0, 1)),
    )
    options = (
        {'id': 'addr_mask', 'desc': 'Device address mask (informational)', 'default': 0x40},
    )

    def __init__(self):
        self.reset()

    def reset(self):
        pass

    def start(self):
        self.out_ann = self.register(srd.OUTPUT_ANN)

    def decode(self, ss, es, data):
        if not data:
            return
        ptype = data[0]
        if ptype == 'DATA':
            if len(data) < 3:
                return
            mosi = data[1]
            if mosi is None:
                return
            try:
                v = int(mosi) & 0xFF
            except Exception:
                return
            if v in FAST_COMMANDS:
                name, desc = FAST_COMMANDS[v]
                self.put(ss, es, self.out_ann, [0, ['%s (0x%02X) — %s' % (name, v, desc)]])
            else:
                self.put(ss, es, self.out_ann, [1, ['Unknown MOSI byte: 0x%02X' % v]])
        elif ptype == 'TRANSFER':
            if len(data) < 2:
                return
            mosi_bytes = data[1]
            for item in mosi_bytes or []:
                val = None
                iss, ies = ss, es
                try:
                    val = item.val
                    iss = item.ss
                    ies = item.es
                except AttributeError:
                    if isinstance(item, (list, tuple)):
                        if len(item) >= 1:
                            val = item[0]
                        if len(item) >= 3:
                            iss, ies = item[1], item[2]
                    else:
                        continue
                if val is None:
                    continue
                try:
                    v = int(val) & 0xFF
                except Exception:
                    continue
                if v in FAST_COMMANDS:
                    name, desc = FAST_COMMANDS[v]
                    self.put(iss, ies, self.out_ann, [0, ['%s (0x%02X) — %s' % (name, v, desc)]])
                else:
                    self.put(iss, ies, self.out_ann, [1, ['Unknown MOSI byte: 0x%02X' % v]])
