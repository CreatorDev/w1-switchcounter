# w1-switchcounter
----

In this project we will connect a Ci40 to Device Server and log the number of times a button is pressed. This number will be visible on the Developer Console web interface.

Areas covered in this project include:

* Cross-compiling a package for OpenWrt using the SDK
* LetMeCreate library
* Awalwm2m
* Device Server
* Developer Console

---

## Assumptions

This guide assumes that you have flashed the latest Ci40 Creator image onto your Ci40 [Instructions](https://docs.creatordev.io/ci40/guides/openwrt-platform/#system-upgrade) and that you have completed the steps in the [quick start guide](../../guides/quick-start-guide). On Ci40, you may need to install awalwm2m and letmecreate (opkg update && opkg install letmecreate awalwm2m) if you are not using a Creator image.

It is also assumed that you have an Ubuntu 16.04 PC/VM (other versions/distros may work, but this cannot be guaranteed).

## Build Environment Setup Instructions

We have prepared a script that will create the build environment for you. You can inspect the script on gist [here](https://gist.github.com/MattAtkinson/3ee477c92ff10c3a246c3ab27d864b58) and run it manually, or run it directly by pasting this command into your build machines terminal in the desired directory:

<pre>
$ curl https://gist.githubusercontent.com/MattAtkinson/3ee477c92ff10c3a246c3ab27d864b58/raw | bash
</pre>

Once the script has completed its install procedure, you will have an openwrt/ folder (containing the Creator OpenWrt SDK) and a custom/ folder (containing the workshop code and makefiles).

### Building your Application

Run the following commands:
<pre>
$ cd openwrt
$ make package/w1-switchcounter/compile
</pre>

There are normally a lot of warnings created at the start of the build process, but they are not a concern. This first build will take longer than normal, as the dependencies (such as awalwm2m) must be built first. Future builds of the same application will be much faster.

Once the build is complete, you can find the ipk file in the bin/pistachio/packages/custom folder.

### Securely Provisioning to Device Server and Running the Application

Before running the example, you need to create a certificate and then copy the certificate and application installer to Ci40. The certificate securely connects Ci40 to your user account on the Device Server.

![certificateimage](img/cert.jpg)

To get a certificate, go to [console.creatordev.io](http:/console.creatordev.io) and create an account (or log in). Once logged in, click on "Device Keys" in the left sidebar and then change to the "Certificates" tab an click "Get Certificate+" button. Take a copy of this certificate and save it to a file called "creatorworkshop.crt".

You now need to get both the certificate and ipk files onto your Ci40. There are numerous ways to do this (usb/microsd/scp/fileserver etc.) but we will use scp in this example. Check your Ci40's IP address by running 'ifconfig' in its terminal, then run the following on your build PC (using your Ci40 ipaddress and the paths to the files):

<pre>
$ scp path/to/switch_1.0.0-1_pistachio.ipk root@yourci40ipaddress:/
$ scp path/to/creatorworkshop.crt root@yourci40ipaddress:/etc/config
</pre>

Once you've copied the files, you can install your package and run the application on Ci40 by running the following in its terminal:

<pre>
/# opkg install switch_1.0.0-1_pistachio.ipk
/# /bin/switch
</pre>

### Viewing the Data on the Developer Console

![deviceimage](img/device.jpg)

[The Developer Console](http://console.creatordev.io) provides and interface to view the devices connected to Device Server. With your application running on Ci40, navigate to the "Devices" page. Select the device, select Object ID: 3200, and you will see the number of times the button (SW1 on Ci40) has been pressed. You can refresh this value using the on-screen refresh button.
