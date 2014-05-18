NOTE: This fork is a WIP, so some of the old stuff is still here.

opensnap-quicktile
==========

What's that?
------------
Opensnap brings the Aero Snap feature to Openbox using quicktile.


Does this work with other window managers?
------------------------------------------
Probably.


Dependencies?
-------------
Just quicktile and gtk3 as far as I know. I only use Arch Linux and have not tested this on other distros yet.


Future:
------
I plan to refine the code to make it only for quicktile. I don't plan to dedicate myself to development of this.

Old stuff:

Installing
----------
If you want to install opensnap from source first make sure you have git installed. (Or fetch a zip file: https://github.com/lawl/opensnap/archive/master.zip)

Fetch via git:

    git clone https://github.com/lawl/opensnap.git

Fetch via zip:

    wget https://github.com/lawl/opensnap/archive/master.zip
    unzip master.zip

Make sure you have all prerequisits:

    sudo apt-get install build-essential libx11-dev libgtk-3-dev wmctrl

And finally build and install it:

    cd opensnap*
    make
    mkdir -p ~/.config/opensnap
    cp sample_configs/* ~/.config/opensnap/

And now start opensnap by

    bin/opensnap --daemon
    

How can I use it?
-----------------
This should tell you all you need to know:

    opensnap --help

Copy the sample configs to ~/.config/opensnap/.


Unsnap workaround
-----------------

As of now opensnap does not support unsnapping (see #4).
You can find a workaround here: https://github.com/lawl/opensnap/issues/4#issuecomment-23666097

Do note however that this does not perfect unsnapping. I.e. your cursor possition and the window you are dragging might get displaced a bit on unsnapping. But it does work well enough for daily use.
