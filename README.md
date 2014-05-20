NOTE: This fork is a WIP, so some of the old stuff is still here.

opensnap-quicktile
==================

What's that?
------------
Opensnap brings the Aero Snap feature to Openbox using quicktile.


Does this work with other window managers?
------------------------------------------
Probably. This is most likely going to be dependant on quicktile.

Dependencies?
-------------
Just quicktile and gtk3 as far as I know. I only use Arch Linux and have not tested this on other distros yet.

Future:
------
I plan to refine the code along the way. There is most likely some useless code that can be rid of. I'd be lying if I said I was an amzing programmer. I don't plan to dedicate myself to development of this.

Installing
----------
If you want to install opensnap-quicktile. from source first make sure you have git installed. (Or fetch a zip file: https://github.com/lawl/opensnap/archive/master.zip)

Install from AUR:
    yaourt -S opensnap-quicktile-git

Fetch via git:

    git clone https://github.com/Pr0Wolf29/opensnap-quicktile.git

Fetch via zip:

    wget https://github.com/Pr0Wolf29/opensnap-quicktile/archive/master.zip
    unzip master.zip

Make sure you have all prerequisits:

    sudo apt-get install build-essential libx11-dev libgtk-3-dev
    I have no idea how to install quicktile on apt systems.

And finally build and install it:

    cd opensnap*
    make

And now start opensnap by

    bin/opensnap --daemon