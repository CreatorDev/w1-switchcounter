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

This guide assumes that you have flashed the latest Ci40 Creator image onto your Ci40 [**Instructions**](http://docs.creatordev.io) and that you have completed the steps in the [quick start guide](../../guides/quick-start-guide). On Ci40, you may need to install awalwm2m and letmecreate (opkg update && opkg install letmecreate awalwm2m) if you are not using a Creator image.

It is also assumed that you have an Ubuntu 16.04 PC/VM (other versions/distros may work, but this cannot be guaranteed).

## Step-by-Step Setup Instructions

First, you need to install the OpenWrt build dependencies on your machine:

<pre>
$ sudo apt-get install git libncurses5-dev libncursesw5-dev zlib1g-dev libssl-dev gawk subversion device-tree-compiler
</pre>

### Get the SDK and Source Code

Then you need to download and extract the OpenWrt SDK:

<pre>
$ wget https://downloads.creatordev.io/pistachio/marduk/OpenWrt-SDK-0.10.4-pistachio-marduk_gcc-5.3.0_musl-1.1.14.Linux-x86_64.tar.bz2
$ mkdir openwrt && tar -xvf OpenWrt-SDK-0.10.4-pistachio-marduk_gcc-5.3.0_musl-1.1.14.Linux-x86_64.tar.bz2 -C openwrt/ --strip-components 1
</pre>

Git clone the example project repository:

<pre>
$ mkdir custom && cd custom
$ git clone http://github.com/mattatkinson/w1-switchcounter
</pre>

### Securely Connecting to Device Server

Before attempting to build and run the example, you need to add a PSK (pre-shared key) to the source code to securely link the Ci40 to your account on the Device Server.

In a browser, go to [console.creatordev.io](http:/console.creatordev.io) and create and account (or log in). Once logged in, click on "Device Keys" in the left sidebar and then click the "Get PSK+" button. Copy the "Identity" and "Secret" values into the following piece of code within custom/w1-switchcounter/Switch/switch.c:

<pre>
... 
int main(void) 
{ 
    char *clientName = "Creator Digital Input"; 
    char *clientIdentity = ""; 
    char *clientSecretHex = ""; 
... 
</pre>

### Building your Project for OpenWrt

Navigate back to the openwrt SDK folder and add your custom folder as an OpenWrt feed (change the path in the echo command to be the **absolute path** to the 'custom' folder created by the earlier git clone):

<pre>
$ cd openwrt
$ echo src-link custom /home/username/ci40/custom >> feeds.conf.default
</pre>

Now update your OpenWrt feeds to add your new package and then build:

<pre>
$ ./scripts/feeds update -a && ./scripts/feeds install -a
$ make package/w1-switchcounter/compile
</pre>

This first build will take longer than normal, as the dependencies (such as awalwm2m) must be built first. Future builds of the same application will be much faster.

Once the build is complete, you can find the ipk file in the bin/pistachio/packages/custom folder.

### Running the Application on Ci40

You now need to get this .ipk file onto your Ci40. There are numerous ways to do this (usb/microsd/scp/fileserver etc.) but we will use scp in this example. Check your Ci40's IP address by running 'ifconfig' in its terminal, then run the following on your build PC (assuming it is on the same network):

<pre>
$ scp switch_1.0.0-1_pistachio.ipk root@ci40ipaddress:/
</pre>

Once you've copied the file, you can install your package and run the application on Ci40 by running the following in its terminal:

<pre>
/# opkg install switch_1.0.0-1_pistachio.ipk
/# LD_LIBRARY_PATH=/usr/lib bin/switch
</pre>

### Viewing the Data on the Developer Console

[The Developer Console](http://console.creatordev.io) provides and interface to view the devices connected to Device Server. With your application running on Ci40, navigate to the "Devices" page. Select the device, select Object ID: 3200 and you will see the number of times the button has been pressed. You can refresh this value using the on-screen refresh button.

## Adding a Temperature Sensor

A simple way to learn more about how the code works is to edit it to add more features. To do this we will use a Thermo3 Click board (connected to MirkoBUS port 1) to make temperature reading visibles on the Developer Console alongside the switch counter.

First, you will want to copy and rename your switch.c file to keep a backup.

There are 2 things you need to change to switch from counting switch presses to reading temperature. First we need to change the IPSO object definition, and secondly add the logic to read the sensor.

### Changing IPSO Object definition

IPSO is a standard to allow interoperability between LWM2M devices. If you look under "Devices" on the [Developer Console](http://console.creatordev.io) you will find a tab for Object Definitions. There you can see that we have listed all the different IPSO standard objects and resources. If you would like a primer on LWM2M, view our introductory guide [here](../../../deviceserver/guides/lwm2m-overview). If you look up Digital Input on this list you will see some key pieces of information:

* ObjectID - 3200
* Digital Input Counter - ResourceID - 5501
* Digital Input Counter - Data Type - Integer

If you then look up Temperature instead of Digital Input you will see:

* ObjectID - 3303
* Sensor Value - ResourceID - 5700
* Sensor Value - Data Type - Float

TBC...
