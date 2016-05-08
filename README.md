# luther

An arduino sketch to monitor and tweet temperature using a Feather Huzzah and the Losant tools.

# [Luther B Dragon](https://twitter.com/lutherbdragon)

Is the bearded dragon that lives in my office. He has a twitter page where he graciously allows me to post on his behalf.

I am using a [Feather Huzzah](https://www.adafruit.com/product/2821) and the [Losant](https://www.losant.com/iot-platform) tools to:
* Read the temperature using a TMP36 sensor form the [Builder Kit](https://store.losant.com/products/losant-builder-kit)
* Report the temperature every 12 seconds
* Tweet the temperature to Luther's twitter when either:
  * 12 hours has elapsed without a tweet
  * The current temperature has changed by 5 Degrees compared to the most recent tweet
  * The on-board button is pressed
* Provide a display of the [temperature history](https://app.losant.com/#/dashboards/572d0ee088a6f20100df2900)

Thanks to [Losant](https://www.losant.com) for the giving out free [Builder Kits](https://store.losant.com/products/losant-builder-kit) and providing free use of their online tools (not sure how long that will last, but trying it out while it does).

# Notes:

This sketch is based on the temperature monitor sketch from the [Builder Kit Source](https://github.com/Losant/losant-kit-builder).

I use 12 seconds for the delivery rate because the underlying MQTT library has a fixed (#defined) connection timeout of 15 seconds. Using the 15 seconds time as the Builder Kit sample did results in frequent timeout/disconnect errors.

The Arduino IDE makes updating the code on the device simple. It also gets in the way of writing code in some ways. Fortunately, most of the issues I ran into can be dodged by putting methods, constants, and global (yuck) variables inside an unnamed namespace. Apparently, this avoids the 'helpful' automatic prototype generation and (maybe) some other 'helpful' things the IDE does. I understand that these things almost certainly make it easier for folks new to programming to get started with these kinds of devices. Bravo to Arduino for making it easier for folks (I mean that sincerely). It is a nuisance to me that this helpfulness cannot be turned off.

