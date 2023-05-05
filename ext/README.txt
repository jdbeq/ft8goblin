ext/
----

So here we contain (if you pulled submodules) a few external pieces:

	ft8_lib		ft8/ft4 decoder/encoder library
	rmsmap		A map for displaying RMS (Winlink) nodes, we use a
			forked version to map what we're seeing
	termbox2	Terminal support library
	yajl		yajl is a json parsing library, used to read config file

If these don't exist:
	git submodule init
	git submodule update


