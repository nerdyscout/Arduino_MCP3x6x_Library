# PCB Fabrication

KiCad projects and KiBot configs for reference boards (`Adafruit/`, `Arduino/`, `Sparkfun/`).

Generate renders and fabrication outputs:

```sh
./pcb/kibot.sh
```

- `pcb/render.kibot.yaml` — board renders
- `pcb/fab.kibot.yaml` — fabrication outputs

Outputs are written next to each `.kicad_pcb`.

## Board renders

| Board | Top | Bottom |
|---|---|---|
| Adafruit Feather | [Feather_top.svg](Adafruit/Feather_top.svg) | [Feather_bottom.svg](Adafruit/Feather_bottom.svg) |
| Arduino Uno | [Uno_top.svg](Arduino/Uno_top.svg) | [Uno_bottom.svg](Arduino/Uno_bottom.svg) |
| Sparkfun ProMicro | [ProMicro_top.svg](Sparkfun/ProMicro_top.svg) | [ProMicro_bottom.svg](Sparkfun/ProMicro_bottom.svg) |
