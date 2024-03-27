

## add printer test to base Makefile rules.
  +LIBS-$(CONFIG_PRINTER_TEST_X2000) += test/printer_x2000/libprintertest.o

## add CONFIG_PRINTER_TEST_X2000 to board config.

```
include/configs/halley5.h
+#define CONFIG_PRINTER_TEST_X2000
```

## uboot commands: printer

