CPPFLAGS_common += -I$(src)/mainboard/$(MAINBOARDDIR)/include

bootblock-y += bootblock.c
bootblock-y += gpio_early.c

ramstage-y += ramstage.c
ramstage-y += gpio.c
ramstage-y += hda_verb.c

SPD_SOURCES = samsung-M471A1G44AB0-CWE
