FreeOSEK_LPC1769
================

FreeOSEK OS implementation for LPC1769 microcontrollers.

This is a LPCXpresso workspace. In order to use it: <br />
1) Open LPCXpresso and select this repository as your current workspace. <br />
2) Go to the Quickstart Panel and use the "Import Project(s)" command. <br />
3) Use the option "Project Directory (unpacked)" and select this folder again. <br />
4) Uncheck "Copy projects into workspace", and select the project(s) you want to import. By default you will need: <br />
- app_osek_blinky
- lpc_board_nxp_lpcxpresso_1769
- lpc_chip_175x_6x

Also you need to have PHP installed in order to run the generation steps. <br />
If you're using Ubuntu you can install PHP by typing:

> $ sudo apt-get install php5

If you're using Windows, you can download PHP from http://windows.php.net/
Unzip the file and add the destination path to your PATH environment variable.
You can check if PHP is correctly installed by opening a command line and type:

> php -v

You should see something like this:

PHP 5.5.15 (cli) (built: Jul 23 2014 15:05:09)
Copyright (c) 1997-2014 The PHP Group
Zend Engine v2.5.0, Copyright (c) 1998-2014 Zend Technologies

As you can see, this repository has been tested with package php-5.5.15-Win32-VC11-x86.

Also the LPCXpresso version used was LPCXpresso v7.3.0 [Build 186] [2014-07-09]. 
You can get it at http://www.lpcware.com/lpcxpresso

If you have any questions, please contact us at: 
contacto(at)proyecto-ciaa(dot)com(dot)ar

Also you can subscribe to our mailing list:
http://groups.google.com/group/embebidos32

Happy coding! :)

The CIAA-Team.
