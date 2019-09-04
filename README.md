# WiFi Selector

This Arduino library makes it easy for your esp8266 to connect to a WiFi access point.

## Usage (PlatformIO)

#### Upload SPIFFS Image

The WiFi Selector page is served by a small web server with a file system.<br>
The files that make up the landing page are in [`basic-front-end`](https://github.com/oatssss/WiFiSelector/tree/master/basic-front-end).<br>
In order for the web server to serve those files, they must be [uploaded to the SPIFFS image](http://docs.platformio.org/en/latest/platforms/espressif8266.html#uploading-files-to-file-system-spiffs) on the ESP board.

For example, I upload the file-system with:

>###### platformio.ini
>``` ini
>[platformio]
>data_dir = .pio/libdeps/esp01/WifiSelector/basic-front-end
>
>[env:esp01]
>build_flags = -Wl,-Teagle.flash.1m512.ld
>```
>[Other possible options in `build_flags`](https://github.com/esp8266/Arduino/tree/master/tools/sdk/ld)

>###### run
>```
>platformio.exe run --target uploadfs
>```

#### Code Usage

``` c++
#include <WifiSelector.h>

void setup()
{
    WifiSelector.reconnect("esp8266-ssid", "esp8266-pass");

    // WifiSelector::reconnect blocks here and attempts to connect to a saved access point
    // If unsuccessful, the board enables AP mode and creates the network "esp8266-ssid"
    // HTTP clients connected to the ESP will be served the WiFi Selector landing page
    // The program unblocks here only when the client provides correct auth details and the board connects to an AP
}
```
