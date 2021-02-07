### Hardware

- [Prusa i3 Hephestos](https://reprap.org/wiki/Prusa_i3_Hephestos/de)
- [E3Dv6 Hotend & Bondtech Extruder (clone)](https://www.aliexpress.com/item/32998931899.html)
- [Einsy Rambo 1.1b Board (clone)](https://www.aliexpress.com/item/33050288555.html)
- [24v Power Supply](https://www.aliexpress.com/item/32981105025.html)
- [Z-axis motor and leadscrew](https://www.aliexpress.com/item/32969385500.html)
- [Heated Bed Spring Steel Plate](https://www.aliexpress.com/item/32967508650.html)
- [Y Carriage](https://www.aliexpress.com/item/32974984693.html)
- Small pieces
  - [longer Cable](https://www.aliexpress.com/item/33004224274.html)
  - [New Belts](https://www.aliexpress.com/item/32918948939.html)
  - [Textile Sleeve Cable Wire Wrapping 50cm](https://www.aliexpress.com/item/33025264662.html)
  - [Textile Sleeve Cable Wire Wrapping 30cm](https://www.aliexpress.com/item/32906748700.html)
  - [Square nuts](https://www.aliexpress.com/item/33041131000.html)
  - [Aluminum Spacer for Y Carriage](https://www.aliexpress.com/item/4000333522667.html)
  - [Screws](https://www.aliexpress.com/item/33022099174.html)
  - 50 cm Nylon Filament 3mm (Or 3mm PFTE Tube)

### Firmware

    git clone git@github.com:jbrunner/Prusa-Firmware-JBR.git
    git remote add bondtech git@github.com:BondtechAB/Bondtech-Prusa-Firmware.git
    git fetch bondtech
    git merge bondtech/Bondtech-MK3-FWnnn

Only initial:

    cp Firmware/variants/Bondtech-MK3S-16-EINSy10a-E3Dv6full.h Configuration_prusa.h

Modifications are made to: Configuration_prusa.h


### Build

    docker run --rm -v `pwd`:/app jb5r/prusa-firmware-builder